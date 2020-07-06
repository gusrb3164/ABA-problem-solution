# ABA-Problem

CAS 방식을 사용한 Linked List 다중스레드 구현에서 발생할 수 있는 ABA Problem 해결 방법

TimeStamp를 C++ 언어로 구현, 
일반적으로 포인터가 가리키는 주소값은 64비트중에서 48비트만 사용하기 때문에 
64비트 정수비트에서 56비트는 주소값을 저장하고, 나머지 8비트는 pop 연산이 CAS하는 과정 사이에 몇번 일어나는지 TimeStamp를 저장한다.

이를 통해서 총 64비트짜리 int변수를 CAS 해서 ABA Problem이 발생하는걸 예방해준다.
