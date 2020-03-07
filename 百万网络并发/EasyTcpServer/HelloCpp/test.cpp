#include <functional>
#include <list>

int funA(int a, int b)
{
	printf("funA\n");
	return 0;
}


int main()
{

	//std::list<int> a;
	//std::function<int(int,int)> call = funA;
	//int n = call(11, 11);

	std::function<int(int)> call;

	int n = 5;
	call = [=/*外部变量捕获列表*/](/*参数列表*/int a) mutable-> int/*返回值类型*/
	{
		n++;
		printf("%d\n", n);
		printf("%d\n", a);
		return 2;
	};

	call(10);
	return 0;
}