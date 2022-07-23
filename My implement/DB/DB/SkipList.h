#pragma once
#ifndef _SKIPLIST_HH
#define _SKIPLIST_HH

#include <string>

//SkipList for int element
/*
1.insert
2.delete
3.find

Node: 每个节点记录了多级索引，并不是多了很多节点

strip size = 2
1                       
1           5             
1     3     5     7     
1  2  3  4  5  6  7  8  

Insert::
可以不通过固定步长，而是通过随机层数来决定 一个节点有几层索引。每次只在[0,random_height]层之间加索引，会让上层节点少，下层节点多。

超过层高的，

插入就是对每一层的链表进行插入操作，插入的时候要找到前驱

Find::
从顶层开始找

remove:
每一层的都要删掉，find的时候记录每一层的前驱
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
	Node* next[kHeight]; //记录该节点在每一层的下一个节点
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