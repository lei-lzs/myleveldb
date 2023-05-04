#include "DBImpl.h"
#include "write_batch.h"
#include "options.h"
#include "version_set.h"
#include "db_format.h"
#include "memtable.h"
#include "logging.h"

#include <condition_variable>

//Writer���̰߳�ȫ��WriteBatch
struct DBImpl::Writer {
	Status status;
	WriteBatch* batch;
	bool sync;
	bool done;

	port::CondVar cv;

	explicit Writer(port::Mutex* mu) : cv(mu) { }
};

Status DBImpl::Put(const WriteOptions& options, const Slice& key, const Slice& value)
{
	return DB::Put(options, key, value);
}

Status DBImpl::Write(const WriteOptions& options, WriteBatch* my_batch)
{
	Writer w(&mutex_);
	w.batch = my_batch;
	w.sync = options.sync;
	w.done = false;

	mutex_.Lock();
	writers_.push_back(&w);

	if (!w.done && &w != writers_.front()) {
		w.cv.Wait();
	}

	if (w.done) {
		return w.status;
	}

	Status status = MakeRoomForWrite(my_batch == nullptr);
	uint64_t last_sequence = versions_->LastSequence();
	Writer* last_writer = &w;

	if (status.ok() && my_batch != nullptr) {  // nullptr batch is for compactions
	//updates�洢�ϲ��������WriteBatch
		WriteBatch* updates = BuildBatchGroup(&last_writer);
		WriteBatchInternal::SetSequence(updates, last_sequence + 1);
		last_sequence += WriteBatchInternal::Count(updates);

		// Add to log and apply to memtable.  We can release the lock
		// during this phase since &w is currently responsible for logging
		// and protects against concurrent loggers and concurrent writes
		// into mem_.
		{
			mutex_.Unlock();
			//WriterBatchд��log�ļ�������:sequence,����count,ÿ�β���������(Put/Delete)��key/value���䳤��
			status = log_->AddRecord(WriteBatchInternal::Contents(updates));
			bool sync_error = false;
			if (status.ok() && options.sync) {
				//log_�ײ�ʹ��logfile_���ļ�ϵͳ����������Sync���д��
				status = logfile_->Sync();
				if (!status.ok()) {
					sync_error = true;
				}
			}
			//д���ļ�ϵͳ���õ������ݶ�ʧ����������MemTable
			if (status.ok()) {
				status = WriteBatchInternal::InsertInto(updates, mem_);
			}
			mutex_.Lock();
			if (sync_error) {
				// The state of the log file is indeterminate: the log record we
				// just added may or may not show up when the DB is re-opened.
				// So we force the DB into a mode where all future writes fail.
				RecordBackgroundError(status);
			}
		}
		if (updates == tmp_batch_) tmp_batch_->Clear();

		versions_->SetLastSequence(last_sequence);
	}

	while (true) {
		Writer* ready = writers_.front();
		writers_.pop_front();
		if (ready != &w) {
			ready->status = status;
			ready->done = true;
			ready->cv.notify_one();
		}
		if (ready = last_writer) break;
	}

	if (!writers_.empty()) {
		writers_.front()->cv.notify_one();
	}

	return status;
}

//���memtable ��level-0�����
//MakeRoomForWrite��Ҫ�������ʹ�����̨�̵߳ȹ�����
//�����ж�Level 0���ļ��Ƿ� >= 8���ǵĻ���sleep 1ms�����������������ã�Level 0���ļ�̫�࣬˵��д��̫�죬Compaction������д����ٶȣ����ڶ�ȡ��ʱ��Level 0���ļ�֮��������ص�������̫��Ļ���Ӱ���ȡ��Ч�ʣ������ǱȽ���΢�����������sleepһ�Σ�
//�������ж�MemTable���Ƿ��пռ䣬�пռ�Ļ��Ϳ��Է����ˣ�д��Ϳ��Լ�����
//���MemTableû�пռ䣬�ж�Immutable MemTable�Ƿ���ڣ����ڵĻ���˵����һ��д����MemTable��û�����д�뵽SSTable�У�˵��д��̫���ˣ���Ҫ�ȴ�Immutable MemTableд����ɣ�
//���ж�Level 0���ļ����Ƿ� >= 12�����̫��˵��д��̫���ˣ���Ҫ�ȴ�Compaction����ɣ�
//����һ��˵������д�룬����MemTable�Ѿ�д���ˣ���Ҫ��MemTable���Immutable MemTable������һ���µ�MemTable��������̨�߳�д�뵽SSTable�С�
Status DBImpl::MakeRoomForWrite(bool force)
{
	//allow_delay �Ƿ����д���ٶ�
	bool allow_delay = !force;
	Status s;
	while (true) {
		if (!bg_error_.ok()) {

		}
		else if (allow_delay && versions_->NumLevelFiles(0) >= config::kL0_SlowdownWritesTrigger) {
			//���level 0���ļ��������࣬�ȴ� �ӳ�һ��
			mutex_.Unlock();
			env_->SleepForMicroseconds(1000);
			allow_delay = false;
			mutex_.Lock();
		}
		else if (!force && (mem_->ApproximateMemoryUsage() <= options_.write_buffer_size)) {
			break;
			// memtable ��δд��������д��
		}
		else if (imm_ != nullptr) {
			// We have filled up the current memtable, but the previous
			// one is still being compacted, so we wait.
			Log(options_.info_log, "Current memtable full; waiting...\n");
			background_work_finished_signal_.Wait();
		}
		else if (versions_->NumLevelFiles(0) >= config::kL0_StopWritesTrigger) {
			// There are too many level-0 files.
			// level-0�ļ�������Ҫ���ƣ�����Ӱ������ٶ�
			// ���>=12������ֹͣд��
			Log(options_.info_log, "Too many L0 files; waiting...\n");
			background_work_finished_signal_.Wait();
		}
		else {
			// Attempt to switch to a new memtable and trigger compaction of old
			assert(versions_->PrevLogNumber() == 0);
			uint64_t new_log_number = versions_->NewFileNumber();
			WritableFile* lfile = nullptr;
			s = env_->NewWritableFile(LogFileName(dbname_, new_log_number), &lfile);
			if (!s.ok()) {
				// Avoid chewing through file number space in a tight loop.
				versions_->ReuseFileNumber(new_log_number);
				break;
			}
			delete log_;
			delete logfile_;
			logfile_ = lfile;
			logfile_number_ = new_log_number;
			log_ = new log::Writer(lfile);
			imm_ = mem_;//mem_��С����4M�����ת��Ϊimm_
			has_imm_.Release_Store(imm_);
			mem_ = new MemTable(internal_comparator_);//����newһ���µ�mem_������
			mem_->Ref();
			force = false;   // Do not force another compaction if have room
			MaybeScheduleCompaction();
		}	
	}
	return s;
}








Status DB::Put(const WriteOptions& options, const Slice& key, const Slice& value) {
	WriteBatch batch;
	batch.Put(key, value);
	return Write(options, &batch);
}