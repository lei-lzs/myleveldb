#pragma once
#ifndef _SKIPLIST_HH
#define _SKIPLIST_HH

#include <string>

//SkipList for int element
/*
1.insert
2.delete
3.find

Node: ÿ���ڵ��¼�˶༶�����������Ƕ��˺ܶ�ڵ�

strip size = 2
1                       
1           5             
1     3     5     7     
1  2  3  4  5  6  7  8  

Insert::
���Բ�ͨ���̶�����������ͨ��������������� һ���ڵ��м���������ÿ��ֻ��[0,random_height]��֮��������������ϲ�ڵ��٣��²�ڵ�ࡣ

������ߵģ�

������Ƕ�ÿһ���������в�������������ʱ��Ҫ�ҵ�ǰ��

Find::
�Ӷ��㿪ʼ��

remove:
ÿһ��Ķ�Ҫɾ����find��ʱ���¼ÿһ���ǰ��
*/

enum MaxHeight
{
	kHeight = 15
};

struct Node
{
	Node(int key, int val)
		:mkey(key)
		,mval(val)
	{
		/*for (int i = 0; i < kHeight; i++)
		{
			next[i] = nullptr;
		}*/
		memset(next, 0, sizeof(Node*) * kHeight);
	}
	int mkey;
	int mval;
	Node* next[kHeight]; //��¼�ýڵ���ÿһ�����һ���ڵ�
};

class SkipList
{
public:
	SkipList();
	~SkipList();
public:
	int mdup;

	int size();

	void insert(int key, int val);
	int find(const int& key);
	void remove(const int& key);

	void dumpFile(const std::string& path);
	void loadFile(const std::string& path);

	void display();
	void draw();
private:
	Node* createNode(int key ,int val);
	int getRandomHeight();

	void destroy();

private:
	Node* mpHead; // the dumpy head
	int mSize;
	int mcuurent_height;
	
};

#endif