#incldue <stdafx.h>
#include <GitTest.h>
// This  File is Test
int GitTest() {
	// 조건 변경 
	for (int i = 0; i < 100; ) {
		std::cout << i << std::endl;
		if (i >= 50)
			std::cout << "i is over 50 : " << i << std::endl;
		else
			std::cout << "i is under 50 : " << i << std::endl;
		i = i + 2;
	}
	return 0;
}