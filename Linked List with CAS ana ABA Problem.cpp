#include<iostream>
#include<thread>
using namespace std;

class Node //노드는data값과다음노드를가리키는포인터로구성된다.
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

long long int Node_to_addrCounter(Node* head) //노드 주소 56비트를 8비트 옮겨서 64비트로 바꿔줌
{
	if (head == nullptr)return 0;
	long long int p = (long long int)head;
	return (p << 8);
}

Node* addrCounter_to_Node(long long int addrCounter) //64비트의 값에서 앞 56비트 주소값을 반환
{
	if (addrCounter >> 8 == 0)return nullptr;
	return reinterpret_cast<Node*>(addrCounter >> 8);
}

class LinkedList
{
public:
	volatile long long int headCounter;  //root 노드주소 56bit + counter 변수 8bit
	LinkedList()
	{
		headCounter = 0;
	}
	void push(Node* newNode)
	{
		if (newNode == nullptr)return;
		while (true)
		{
			uint8_t counter = (uint8_t)headCounter; //8비트 카운터 값 추출
			long long int currentCounter = headCounter;
			Node* current = addrCounter_to_Node(currentCounter); //current 의 64비트에서 노드주소만 가져와 저장한다
			newNode->nextNode = current;
			long long int newCounter = Node_to_addrCounter(newNode) + counter; //newNode 의 주소와 counter를 64비트로 변환한다.
			if (_InterlockedCompareExchange64(&headCounter, newCounter, currentCounter)
				== currentCounter)break;
		}
	}
	Node* pop()
	{
		while (true)
		{
			uint8_t counter = (uint8_t)headCounter + 1; //pop 할 때만 카운터 값 1증가
			long long int currentCounter = headCounter;
			Node* current = addrCounter_to_Node(currentCounter); //current 의 64비트에서 56비트 노드주소만 가져와 저장한다
			if (current == nullptr)return nullptr; //주소가 0이라면 리턴
			long long int nextCounter;
			if (current->nextNode == nullptr) //다음 노드가 null이라면 next 주소는 0이다.
				nextCounter = 0 + counter;
			else
				nextCounter = Node_to_addrCounter(current->nextNode) + counter;  //next 의 주소와 counter를 64비트로 변환한다.
			if (_InterlockedCompareExchange64(&headCounter, nextCounter, currentCounter)
				== currentCounter)return addrCounter_to_Node(currentCounter);
		}
		return nullptr;
	}
};
LinkedList* FreeList = new LinkedList();
LinkedList* HeadList = new LinkedList();
static void ThreadBody()// 스레드 끼리 병행수행하는 몸체
{
	for (int i = 0; i < 20000; i++)
	{
		for (int j = 0; j < 5; j++)
			HeadList->push(FreeList->pop());  //FreeList에서 꺼낸 node를 HeadList에 push
		for (int j = 0; j < 3; j++)
			FreeList->push(HeadList->pop());  //HeadList에서 꺼낸 node를 FreeList에 push
	}
}
int main()
{
	for (int i = 0; i < 1000000; i++) //FreeList에 미사용 노드 생성
	{
		Node* n = new Node(i);
		FreeList->push(n);
	}
	//3개의 스레드를 생성
	thread t1(ThreadBody);
	thread t2(ThreadBody);
	thread t3(ThreadBody);
	t1.join();
	t2.join();
	t3.join();
	//3개의 스레드 실행이 끝나면 HeadList의 headCounter 에서 head 주소만 가져온다
	//HeadList의 노드 개수를 출력하는 과정
	int count = 0;
	long long int n = HeadList->headCounter;
	Node* p = addrCounter_to_Node(n);
	if (p == nullptr)
	{
		cout << "HeadList Node 개수: 0" << endl;
		return 0;
	}
	while (p != nullptr)
	{
		count++;
		//cout << count << "번째 data: " << p->data << endl;
		p = p->nextNode;
	}

	cout << "HeadList Node 개수: " << count << endl;
	return 0;
}