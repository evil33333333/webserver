#pragma once
#include <iostream>
#include <vector>
#include "route.h"
#include "method.h"

class InetApi {
public:
	static Route* CreateNewRoute(std::string path, Method method, void* callback_func, void* failure_func);
	static void Listen(std::vector<Route*> Routes);
	static void ListenToRoute(Route* route);
	static void ResponseWriter(SOCKET* sock, std::string& return_data, std::vector<std::string>& headers, uint8_t status_code);
};
