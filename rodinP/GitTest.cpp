#incldue <stdafx.h>
#include <GitTest.h>
// This  File is Test
// �ּ��� �߸� �޾ҳ׿�.. �̰� �´� ����. 
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
int branch_test() {
	// �̰��� �귣ġ �׽�Ʈ �Լ� �Դϴ�. 
	return 0;
}
int brach_test2() {
	// �̰��� �귣ġ �׽�Ʈ �Լ� 2�Դϴ�. 
	return 0;
}
int branch_test3_sinji() {
	// �̰��� �귣ġ �׽�Ʈ �Լ� 3�Դϴ�. 
	return 0;
}
int fork_test() {
	// �̰��� ��ũ �׽�Ʈ �Դϴ�.
	// �ٽ� user1 �߰� �����Դϴ�. 
}