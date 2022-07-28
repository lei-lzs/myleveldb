#include "SkipList.h"
#include <iostream>
#include <map>
#include "elapsed_time_reporter.h"
#include "coding.h"
using namespace std;

#define TEST_COUNT 500000

void test_insert(SkipList& mylist);
void test_random_insert(SkipList& mylist);
void test_random_insert(std::map<int,int>& mymap);
void test_remove(SkipList& mylist);
void test_random_remove(SkipList& mylist);
void test_find(SkipList& mylist);
void test_random_find(SkipList& mylist);
void test_random_find(std::map<int, int>& mymap);


int TestSkipList()
{
	SkipList mylist;
	test_random_insert(mylist);
	//test_random_find(mylist);

	//int re = mylist.find(7556);
	//std::cout << "find key = " << " value= : " << re << endl;

	//test_random_remove(mylist);

	std::map<int, int> mymap;
	test_random_insert(mymap);
	//test_random_find(mymap);
	//mylist.display();
	//mylist.dumpFile("lzs.list");
	std::cout << "重复: " << mylist.mdup << endl;

	//随机数的种子确定后，随机数就一定了
	//srand(time(nullptr));
	//insert
	//test_insert(mylist);

	//mylist.display();

	//test_remove(mylist);

	//mylist.display();

	//test_find(mylist);

	//mylist.display();

	//mylist.dumpFile("lzs.list");

	//SkipList mylist1;
	//mylist1.loadFile("lzs.list");
	//mylist1.display();
	
	return 0;
}

void test_insert(SkipList& mylist) {
	cout << "******test insert******" << endl;
	mylist.insert(5, 5);
	mylist.insert(2, 2);
	mylist.insert(3, 3);
	mylist.insert(4, 4);
	mylist.insert(5, 5);
	mylist.insert(3, 3);
	mylist.insert(7, 7);
	mylist.insert(4, 4);
	mylist.insert(9, 9);
	mylist.insert(10, 10);
	mylist.insert(1, 101);
}

void test_remove(SkipList& mylist) {
	cout << "******test remove******" << endl;
	mylist.remove(2);
	mylist.remove(8);
	mylist.remove(50);
	mylist.remove(10);
}

void test_find(SkipList& mylist) {
	cout << "******test find******" << endl;
	cout << "find 1: " << mylist.find(1) << endl;
	cout << "find 3: " << mylist.find(3) << endl;
	cout << "find 5: " << mylist.find(5) << endl;
	cout << "find 7: " << mylist.find(7) << endl;
	cout << "find 9: " << mylist.find(9) << endl;
	cout << "find -100: " << mylist.find(-100) << endl;
}


void test_random_insert(SkipList& mylist)
{
	elpased_time_report re("test_random_insert_skiplist",true);
	srand(time(nullptr));
	int count = TEST_COUNT;
	while (count--)
	{
		//[0- RAND_MAX] -[0-TEST_COUNT}
		int num = (double)rand()/RAND_MAX* TEST_COUNT;
		//cout << "insert : " << num << endl;
		mylist.insert(num, num);
	}
}

void test_random_find(SkipList& mylist)
{
	elpased_time_report re("test_random_find_skiplist");
	srand(time(nullptr));
	int count = TEST_COUNT;
	while (count--)
	{
		//[0- RAND_MAX] -[0-TEST_COUNT}
		int num = (double)rand() / RAND_MAX * TEST_COUNT;
		int re = mylist.find(num);
		//cout << "Key : " << num << "value: "<<re<<endl;

	}
}

void test_random_insert(std::map<int, int>& mymap)
{
	elpased_time_report re("test_random_insert_map", false);
	srand(time(nullptr));
	int count = TEST_COUNT;
	while (count--)
	{
		int num = (double)rand() / RAND_MAX * TEST_COUNT;
		//cout << "insert : " << num << endl;
		mymap.insert({num,num});
	}
}

void test_random_find(std::map<int, int>& mymap)
{
	elpased_time_report re("test_random_find_map");
	srand(time(nullptr));
	int count = TEST_COUNT;
	while (count--)
	{
		int num = (double)rand() / RAND_MAX * TEST_COUNT;
		//cout << "insert : " << num << endl;
		auto re = mymap.find(num);
	}
}

void test_random_remove(SkipList& mylist)
{
	elpased_time_report re("test_random_remove_skiplist");
	srand(time(nullptr));

	cout << "before remove size= : " << mylist.size() << endl;

	int count = TEST_COUNT;
	while (count--)
	{
		int num = (double)rand() / RAND_MAX * TEST_COUNT;
		//cout << "insert : " << num << endl;
		mylist.remove(num);
	}

	mylist.display();

	cout << "after remove size= : " << mylist.size() << endl;
}

extern void test_status();

int main()
{
	
	/*int num = 0xffad45;

	uint8_t a = num;

	string str;
	PutVarint32(&str, num);
	cout << "num: " << num << endl;
	cout << "coding: " << str << "size after coding: " << str.size() << endl;

	uint32_t re = 0;
	GetVarint32(&str, re);
	cout << "encoding: " << re << endl;*/

	test_status();

	return 0;
}