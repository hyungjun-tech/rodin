#incldue <stdafx.h>
#include <GitTest.h>
// This  File is Test
// �ּ� �߰� �׽�Ʈ �� revert �׽�Ʈ 
// ���� ���� 
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
