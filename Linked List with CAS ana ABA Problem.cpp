#include<iostream>
#include<thread>
using namespace std;

class Node //����data����������带����Ű�������ͷα����ȴ�.
{
public:
	int data;
	Node* nextNode;
	Node(int i)
	{
		data = i;
		nextNode = nullptr;
	}
};

long long int Node_to_addrCounter(Node* head) //��� �ּ� 56��Ʈ�� 8��Ʈ �Űܼ� 64��Ʈ�� �ٲ���
{
	if (head == nullptr)return 0;
	long long int p = (long long int)head;
	return (p << 8);
}

Node* addrCounter_to_Node(long long int addrCounter) //64��Ʈ�� ������ �� 56��Ʈ �ּҰ��� ��ȯ
{
	if (addrCounter >> 8 == 0)return nullptr;
	return reinterpret_cast<Node*>(addrCounter >> 8);
}

class LinkedList
{
public:
	volatile long long int headCounter;  //root ����ּ� 56bit + counter ���� 8bit
	LinkedList()
	{
		headCounter = 0;
	}
	void push(Node* newNode)
	{
		if (newNode == nullptr)return;
		while (true)
		{
			uint8_t counter = (uint8_t)headCounter; //8��Ʈ ī���� �� ����
			long long int currentCounter = headCounter;
			Node* current = addrCounter_to_Node(currentCounter); //current �� 64��Ʈ���� ����ּҸ� ������ �����Ѵ�
			newNode->nextNode = current;
			long long int newCounter = Node_to_addrCounter(newNode) + counter; //newNode �� �ּҿ� counter�� 64��Ʈ�� ��ȯ�Ѵ�.
			if (_InterlockedCompareExchange64(&headCounter, newCounter, currentCounter)
				== currentCounter)break;
		}
	}
	Node* pop()
	{
		while (true)
		{
			uint8_t counter = (uint8_t)headCounter + 1; //pop �� ���� ī���� �� 1����
			long long int currentCounter = headCounter;
			Node* current = addrCounter_to_Node(currentCounter); //current �� 64��Ʈ���� 56��Ʈ ����ּҸ� ������ �����Ѵ�
			if (current == nullptr)return nullptr; //�ּҰ� 0�̶�� ����
			long long int nextCounter;
			if (current->nextNode == nullptr) //���� ��尡 null�̶�� next �ּҴ� 0�̴�.
				nextCounter = 0 + counter;
			else
				nextCounter = Node_to_addrCounter(current->nextNode) + counter;  //next �� �ּҿ� counter�� 64��Ʈ�� ��ȯ�Ѵ�.
			if (_InterlockedCompareExchange64(&headCounter, nextCounter, currentCounter)
				== currentCounter)return addrCounter_to_Node(currentCounter);
		}
		return nullptr;
	}
};
LinkedList* FreeList = new LinkedList();
LinkedList* HeadList = new LinkedList();
static void ThreadBody()// ������ ���� ��������ϴ� ��ü
{
	for (int i = 0; i < 20000; i++)
	{
		for (int j = 0; j < 5; j++)
			HeadList->push(FreeList->pop());  //FreeList���� ���� node�� HeadList�� push
		for (int j = 0; j < 3; j++)
			FreeList->push(HeadList->pop());  //HeadList���� ���� node�� FreeList�� push
	}
}
int main()
{
	for (int i = 0; i < 1000000; i++) //FreeList�� �̻�� ��� ����
	{
		Node* n = new Node(i);
		FreeList->push(n);
	}
	//3���� �����带 ����
	thread t1(ThreadBody);
	thread t2(ThreadBody);
	thread t3(ThreadBody);
	t1.join();
	t2.join();
	t3.join();
	//3���� ������ ������ ������ HeadList�� headCounter ���� head �ּҸ� �����´�
	//HeadList�� ��� ������ ����ϴ� ����
	int count = 0;
	long long int n = HeadList->headCounter;
	Node* p = addrCounter_to_Node(n);
	if (p == nullptr)
	{
		cout << "HeadList Node ����: 0" << endl;
		return 0;
	}
	while (p != nullptr)
	{
		count++;
		//cout << count << "��° data: " << p->data << endl;
		p = p->nextNode;
	}

	cout << "HeadList Node ����: " << count << endl;
	return 0;
}