#include "SkipList.h"
#include <cstdlib>
#include <iostream>
#include <fstream>


SkipList::SkipList()
{
	mpHead = new Node(0, 0);
	mSize = 0;
	mcuurent_height = 1;
	mdup = 0;
}

SkipList::~SkipList()
{
	destroy();
}

void SkipList::insert(int key, int val)
{
	//ÿһ��ѭ�����ҵ�������ǰ��
	Node* prev[kHeight] ={};

	Node* current = mpHead;

	for (int level = mcuurent_height - 1; level >= 0; level--)
	{
		
		while (current->next[level] != nullptr && current->next[level]->mkey < key) { //������ܲ����ظ�key���ظ�key��ǰ��Ϊ ��keyС��һ��

			current = current->next[level];
		
		}

		prev[level] = current;

		if (current->next[level] != nullptr && current->next[level]->mkey == key) {

			//std::cout << "The key " << key << " is allready in the list, just updata the value" << std::endl;
			current->next[level]->mval = val;

			mdup++;

			return;
		}
	}

	//�����½ڵ�
	Node* toBeInsert = createNode(key, val); 

	//��ȡ����߶�
	int randomHeight = getRandomHeight(); // 1 to kHeight

	if (randomHeight > mcuurent_height) {
		
		for (int level = mcuurent_height; level < randomHeight; level++) {
			prev[level] = mpHead;
		}

		mcuurent_height = randomHeight;
	}

	//��0��randdonHeight�����μ�����

	for (int i = 0; i < randomHeight; i++)
	{
		toBeInsert->next[i] = prev[i]->next[i];
		prev[i]->next[i] = toBeInsert;
	}

	mSize++;

	// 
	for (int i = randomHeight; i < kHeight; i++)
	{

	}

}

int SkipList::size()
{
	return mSize;
}

int SkipList::find(const int& key)
{
	//��remove�߼�����
	//ÿһ�������ÿ�ҵ�����ǰ����Ȼ�������һ�㣬����Ѱ��
	Node* current = mpHead;
	for (int i = mcuurent_height - 1; i >= 0; i--)
	{
		while (current->next[i] != nullptr && current->next[i]->mkey < key)
		{
			current = current->next[i];
		}

		if (current->next[i] != nullptr && current->next[i]->mkey == key)
		{
			return current->next[i]->mval;
		}

	}
	return -1;
}

void SkipList::remove(const int& key)
{
	//ÿһ��������ҵ�ǰ��, ��һ���ǰ��һ������һ���ǰ������
	Node* current = mpHead;
	Node* toBeRemove = nullptr;

	for (int i = mcuurent_height -1; i >= 0; i--)
	{		
		while (current->next[i] != nullptr && current->next[i]->mkey < key)
		{
			current = current->next[i];
		}

		//remove��ǰ���link
		if (current->next[i] != nullptr && current->next[i]->mkey == key)
		{
			toBeRemove = current->next[i];
			current->next[i] = current->next[i]->next[i];
			
		}			
	}

	if (toBeRemove != nullptr)
	{
		mSize--;
		delete toBeRemove;
	}
		
}

void SkipList::display()
{
	auto disp = [](Node* head, int level) {
		std::cout << "level-" << level << ": ";

		while (head != nullptr) {
			std::cout << head->mkey << "-";
			head = head->next[level];
		}

		std::cout << std::endl;
	};

	for (int i = 0; i < kHeight; i++) {
		disp(mpHead->next[i], i);
	}
}

void SkipList::draw()
{
}

Node* SkipList::createNode(int key, int val)
{
	return new Node(key,val);
}

int SkipList::getRandomHeight()
{
	int k = 1;

	while (rand() % 2) {
		k++;
	}
		
	//k= rand() % kHeight;
	k = k<kHeight ? k : kHeight;

	//std::cout << "random height = " << k+1 << std::endl;

	return k;
}

void SkipList::destroy()
{
}

void SkipList::dumpFile(const std::string& path)
{
	std::cout << "Save data to file: " << path << std::endl;

	std::ofstream ofile(path);

	Node* head = mpHead->next[0];

	while (head != nullptr) {
		ofile << head->mkey <<" - " << head->mval<<std::endl;
		head = head->next[0];
	}
}

void SkipList::loadFile(const std::string& path)
{
	std::cout << "Load the data from " << path << std::endl;
	std::ifstream ifile(path);

	std::string line;

	int key;
	std::string place_holder;
	int val;

	while (!ifile.eof()) {
		//getline(ifile, line);
		ifile >> key>>place_holder>>val;

		insert(key, val);
	}
}