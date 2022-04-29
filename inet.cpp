#include <iostream>
#include <winsock2.h>
#include <thread>
#include <vector>
#include <string>
#include <map>
#include "route.h"
#include "inet.h"
#include "method.h"


// remove later
static std::vector<std::string> bad_response_headers = {"User-Agent: FuckedUp"};

std::vector<std::string> SplitString(std::string s, std::string delimiter);

Route* InetApi::CreateNewRoute(std::string path, Method method, void* callback_func, void* failure_func) {
	Route route{ path, method, callback_func, failure_func };
	return &route;
}

void InetApi::Listen(std::vector<Route*> Routes) {
	while (true) {
		WSAData wsData;
		if (!WSAStartup(MAKEWORD(2, 2), &wsData)) {
			return;
		}
		SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
		if (sock == INVALID_SOCKET) {
			return;
		}
		struct sockaddr_in listener{};
		listener.sin_family = AF_INET;
		listener.sin_addr.S_un.S_addr = INADDR_ANY;
		listener.sin_port = htons(80);
		if (!bind(sock, (const sockaddr*)&listener, sizeof(listener))) {
			return;
		}
		listen(sock, 3);
		int sizeofsin = sizeof(struct sockaddr_in);
		SOCKET connection = accept(sock, (struct sockaddr*)&listener, &sizeofsin);
		std::vector<char> stream;
		// Receives all data into a vector with increments of 1024 bytes
		while (recv(connection, stream.data(), 1024, 0));
		std::thread([&]() {
			bool route_found = false;
			std::string buffer;
			for (auto const& character : stream) {
				buffer += character;
			}
			for (auto const& route : Routes) {
				// if the request contains the correct method and the correct path, run the code
				if (buffer.substr(0, 32).find(route->path) != std::string::npos && buffer.substr(0, 4).find(MethodStrings[route->method]) != std::string::npos) {
					route_found = true;
					std::string data = SplitString(buffer, "\r\n\r\n")[1];
					std::vector<std::string> buffer_lines = SplitString(buffer, "\r\n");
					std::vector<std::string> headers;
					for (int i = 1; i < buffer_lines.size(); i++) {
						if (static_cast<unsigned long long>(i) + 2 != buffer_lines.size()) {
							headers.push_back(buffer_lines[i]);
						}
					}
					((void(*)(Route*, std::string&, std::vector<std::string>&, SOCKET*))route->callback_func)(route, data, headers, &sock);
				}
			}
			if (!route_found) {
				const char* failure = "{\"response\": \"failure\"}";
				send(sock, failure, sizeof(failure), 0);
			}
		}).detach();
	}
}

void InetApi::ListenToRoute(Route* route) {
	while (true) {
		WSAData wsData;
		if (!WSAStartup(MAKEWORD(2, 2), &wsData)) {
			return;
		}
		SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
		if (sock == INVALID_SOCKET) {
			return;
		}
		struct sockaddr_in listener {};
		listener.sin_family = AF_INET;
		listener.sin_addr.S_un.S_addr = INADDR_ANY;
		listener.sin_port = htons(80);
		if (!bind(sock, (const sockaddr*)&listener, sizeof(listener))) {
			return;
		}
		listen(sock, 3);
		int sizeofsin = sizeof(struct sockaddr_in);
		SOCKET connection = accept(sock, (struct sockaddr*)&listener, &sizeofsin);
		std::vector<char> stream;
		// Receives all data into a vector with increments of 1024 bytes
		while (recv(connection, stream.data(), 1024, 0));
		std::thread([&]() {
			std::string buffer;
			for (auto const& character : stream) {
				buffer += character;
			}
				// the route 
			if (buffer.substr(0, 32).find(route->path) != std::string::npos && buffer.substr(0, 4).find(MethodStrings[route->method]) != std::string::npos) {
				std::string data = SplitString(buffer, "\r\n\r\n")[1];
				std::vector<std::string> buffer_lines = SplitString(buffer, "\r\n");
				std::vector<std::string> headers;
				for (int i = 1; i < buffer_lines.size(); i++) {
					if (static_cast<unsigned long long>(i) + 2 != buffer_lines.size()) {
						headers.push_back(buffer_lines[i]);
					}
				}
				((void(*)(Route*, std::string&, std::vector<std::string>&, SOCKET*))route->callback_func)(route, data, headers, &sock);
			}
			else {
				if (route->failure_func != nullptr) {
					((void(*)(SOCKET*, std::vector<std::string>&))route->failure_func)(&sock, bad_response_headers);
				}
			}
		}).detach();
	}
}

void InetApi::ResponseWriter(SOCKET* sock, std::string& return_data, std::vector<std::string>& headers, uint8_t status_code) {
	std::string all_response_bytes = "HTTP/1.1 " + std::to_string(status_code) + "\r\n";
	for (auto const& header : headers) {
		all_response_bytes.append(header + "\r\n");
	}
	all_response_bytes.append("\r\n\r\n" + return_data);
	send(*sock, all_response_bytes.c_str(), all_response_bytes.size(), 0);
	closesocket(*sock);
}


std::vector<std::string> SplitString(std::string s, std::string delimiter) {
	size_t pos_start = 0, pos_end, delim_len = delimiter.length();
	std::string token;
	std::vector<std::string> res;

	while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos) {
		token = s.substr(pos_start, pos_end - pos_start);
		pos_start = pos_end + delim_len;
		res.push_back(token);
	}

	res.push_back(s.substr(pos_start));
	return res;
}