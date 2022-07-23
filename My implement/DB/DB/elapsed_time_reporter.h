#pragma once
#ifndef _ELAPSED_TIME_REPORTER_HH
#define _ELAPSED_TIME_REPORTER_HH

#include <chrono>
#include <iostream>
#include <string>

class elpased_time_report
{
public:
	elpased_time_report(std::string f, bool print = true) {
		isprint = print;
		if(isprint)
			std::cout <<f<< " :elpase start..." << std::endl;
		start = std::chrono::high_resolution_clock::now();
	}

	~elpased_time_report() {
		
		end = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> ts = end - start;
		if (isprint)
			std::cout << "elpase end, cost : " << ts.count()<< std::endl;
	}

	std::chrono::high_resolution_clock::time_point start;
	std::chrono::high_resolution_clock::time_point end;
	bool isprint;
};

#endif 
