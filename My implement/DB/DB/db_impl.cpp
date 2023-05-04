#include "DBImpl.h"
#include "write_batch.h"
#include "options.h"
#include "version_set.h"
#include "db_format.h"
#include "memtable.h"
#include "logging.h"

#include <condition_variable>

//Writer是线程安全的WriteBatch
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
	//updates存储合并后的所有WriteBatch
		WriteBatch* updates = BuildBatchGroup(&last_writer);
		WriteBatchInternal::SetSequence(updates, last_sequence + 1);
		last_sequence += WriteBatchInternal::Count(updates);

		// Add to log and apply to memtable.  We can release the lock
		// during this phase since &w is currently responsible for logging
		// and protects against concurrent loggers and concurrent writes
		// into mem_.
		{
			mutex_.Unlock();
			//WriterBatch写入log文件，包括:sequence,操作count,每次操作的类型(Put/Delete)，key/value及其长度
			status = log_->AddRecord(WriteBatchInternal::Contents(updates));
			bool sync_error = false;
			if (status.ok() && options.sync) {
				//log_底层使用logfile_与文件系统交互，调用Sync完成写入
				status = logfile_->Sync();
				if (!status.ok()) {
					sync_error = true;
				}
			}
			//写入文件系统后不用担心数据丢失，继续插入MemTable
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

//检查memtable 和level-0的情况
//MakeRoomForWrite主要是限流和触发后台线程等工作：
//首先判断Level 0的文件是否 >= 8，是的话就sleep 1ms，这里是限流的作用，Level 0的文件太多，说明写入太快，Compaction跟不上写入的速度，而在读取的时候Level 0的文件之间可能有重叠，所以太多的话，影响读取的效率，这算是比较轻微的限流，最多sleep一次；
//接下来判断MemTable里是否有空间，有空间的话就可以返回了，写入就可以继续；
//如果MemTable没有空间，判断Immutable MemTable是否存在，存在的话，说明上一次写满的MemTable还没有完成写入到SSTable中，说明写入太快了，需要等待Immutable MemTable写入完成；
//再判断Level 0的文件数是否 >= 12，如果太大，说明写入太快了，需要等待Compaction的完成；
//到这一步说明可以写入，但是MemTable已经写满了，需要将MemTable变成Immutable MemTable，生成一个新的MemTable，触发后台线程写入到SSTable中。
Status DBImpl::MakeRoomForWrite(bool force)
{
	//allow_delay 是否减慢写入速度
	bool allow_delay = !force;
	Status s;
	while (true) {
		if (!bg_error_.ok()) {

		}
		else if (allow_delay && versions_->NumLevelFiles(0) >= config::kL0_SlowdownWritesTrigger) {
			//如果level 0的文件数量过多，等待 延迟一会
			mutex_.Unlock();
			env_->SleepForMicroseconds(1000);
			allow_delay = false;
			mutex_.Lock();
		}
		else if (!force && (mem_->ApproximateMemoryUsage() <= options_.write_buffer_size)) {
			break;
			// memtable 还未写满，继续写入
		}
		else if (imm_ != nullptr) {
			// We have filled up the current memtable, but the previous
			// one is still being compacted, so we wait.
			Log(options_.info_log, "Current memtable full; waiting...\n");
			background_work_finished_signal_.Wait();
		}
		else if (versions_->NumLevelFiles(0) >= config::kL0_StopWritesTrigger) {
			// There are too many level-0 files.
			// level-0文件个数需要控制，避免影响查找速度
			// 因此>=12个，则停止写入
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
			imm_ = mem_;//mem_大小超过4M，因此转化为imm_
			has_imm_.Release_Store(imm_);
			mem_ = new MemTable(internal_comparator_);//重新new一个新的mem_供更新
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