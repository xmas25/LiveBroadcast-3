#include <utils/codec/FlvManager.h>
#include <iostream>

std::string file = "/root/server/1.flv";

int main()
{
	FlvManager flv_manager(file);
	flv_manager.ParseFile(99571884);

	std::cout << "over\n";
	int temp;
	std::cin >> temp;
}