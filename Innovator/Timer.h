#pragma once

#include <chrono>
#include <iostream>

class Timer {
public:
	Timer(const std::string & message) : 
		message(message),
		t0(std::chrono::high_resolution_clock::now())
	{}

	~Timer()
	{
		auto elapsedtime = std::chrono::high_resolution_clock::now() - this->t0;
		auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(elapsedtime);
		auto milliseconds = nanoseconds.count() / 1000000.0f;
		std::cout << "Timer: " << this->message << " ran for " << milliseconds << " ms." << std::endl;
	}

	std::string message;
	std::chrono::time_point<std::chrono::steady_clock> t0;
};
