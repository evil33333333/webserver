#pragma once
#include <array>
#include <iostream>
typedef enum {
	Get,
	Post,
} Method;

std::array<std::string, 2> MethodStrings = {
	"GET",
	"POST",
};