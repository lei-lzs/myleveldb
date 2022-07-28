#include "status.h"

#include<iostream>

void test_status()
{
	Status ss = Status::OK();
	std::cout << ss.ToString() << std::endl;

	ss = Status::NotFound("lzs", "can not find");
	std::cout << ss.ToString() << std::endl;

	Status ss1 = ss;
	std::cout << ss1.ToString() << std::endl;

	Status ss2;
	ss2= ss;
	std::cout << ss2.ToString() << std::endl;

	Status ss3(Status::NotSupported("lzs", "not supporteds"));
	std::cout << ss3.ToString() << std::endl;
}