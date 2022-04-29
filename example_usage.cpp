#include <iostream>
#include "inet.h"
#include <windows.h>
#include <vector>
#include <memory>

int main() {
	std::unique_ptr<Route> route(InetApi::CreateNewRoute("/", Method::Post, callback_func, nullptr));
	// listen to one route
	InetApi::ListenToRoute(route.get());

	// listen to multiple
	InetApi::Listen(std::vector<Route*>{route.get(), route.get()});
}

void callback_func(Route* route, std::string& data, std::vector<std::string>& headers, SOCKET* sock) {
	if (data.find("Privet!") != std::string::npos) {
		std::string response = "{\"status\": \"ok\"}";
		InetApi::ResponseWriter(sock, response, headers, 200);
	}
	else {
		std::string response = "{\"status\": \"fail\"}";
		InetApi::ResponseWriter(sock, response, headers, 200);
	}
}

void failure_func(SOCKET* sock, std::vector<std::string>& headers) {
	std::string response = "{\"status\": \"severe_fail\"}";
	InetApi::ResponseWriter(sock, response, headers, 500);
}
