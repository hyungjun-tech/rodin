#incldue <stdafx.h>
#include <GitTest.h>
// This  File is Test
// 주석을 잘못 달았네요.. 이게 맞는 것임. 
// 조건 변경 
int in_func() {
	for (int i = 0; i < 100; ) {
		std::cout << i << std::endl;
		if (i >= 50)
			std::cout << "i is over 50 : " << i << std::endl;
		else
			std::cout << "i is under 50 : " << i << std::endl;
		i = i + 2;
	}
}
int GitTest() {
	in_func();
	return 0;
}
int branch_test() {
	// 이것은 브랜치 테스트 함수 입니다. 
	return 0;
}
int brach_test2() {
	// 이것은 브랜치 테스트 함수 2입니다. 
	return 0;
}
int branch_test3_sinji() {
	// 이것은 브랜치 테스트 함수 3입니다. 
	return 0;
}
int fork_test() {
	// 이것은 포크 테스트 입니다.
	// 다시 user1 추가 변경입니다. 
}