#pragma once
#include <iostream>
#include "inet.h"
typedef struct {
	std::string path;
	Method method;
	void* callback_func;
	void* failure_func;
} Route;