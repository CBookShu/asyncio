#ifndef ASYNCIO_WIN32_SELECTOR_H
#define ASYNCIO_WIN32_SELECTOR_H
#include <asyncio/asyncio_ns.h>
#include <asyncio/selector/event.h>
#include <iostream>
#include <cstdlib>
#include <WinSock2.h>
#include <unordered_map>
#include <ranges>
#include <thread>
#include <chrono>

#pragma comment(lib, "ws2_32.lib")

ASYNCIO_NS_BEGIN
struct UserWsaPoll {
	WSAPOLLFD p;
	void* ud;
};

struct Win32Selector {
private:
	std::unordered_map<SOCKET, UserWsaPoll> fds_;
public:
	Win32Selector() {
		WSADATA wsaData;
		(void)WSAStartup(MAKEWORD(2, 2), &wsaData);

	}
	~Win32Selector() {
		WSACleanup();
	}
	
	std::vector<Event> select(int timeout /* ms */) {
		using namespace std::chrono_literals;

		SetLastError(0);
		std::vector< WSAPOLLFD> pollfds; pollfds.reserve(fds_.size());
		std::vector<void*> uds; uds.reserve(fds_.size());
		for (auto& it : fds_) {
			pollfds.push_back(it.second.p);
			uds.push_back(it.second.ud);
		}
		int iResult = 0;
		if (pollfds.empty()) {
			std::this_thread::sleep_for(1ms * timeout);
		}
		else {
			iResult = ::WSAPoll(pollfds.data(), pollfds.size(), timeout);
			if (iResult == SOCKET_ERROR) {
				std::cout << "WSAGetLastError:" << WSAGetLastError() << std::endl;
			}
		}
		int opCount = 0;
		int totalCount = pollfds.size();
		std::vector<Event> result;
		for (int i = 0; i < totalCount && opCount < iResult; ++i) {
			if (pollfds[i].revents == 0) {
				continue;
			}
			opCount++;

			auto handle_info = reinterpret_cast<HandleInfo*>(uds[i]);
			if (handle_info->handle != nullptr && handle_info->handle != (Handle*)&handle_info->handle) {
				result.emplace_back(Event{
						.handle_info = *handle_info
				});
			}
			else {
				// mark event ready, but has no response callback
				handle_info->handle = (Handle*)&handle_info->handle;
			}
		}
		return result;
	}
	bool is_stop() { return fds_.empty(); }
	void register_event(const Event& event) {
		UserWsaPoll ev;
		ev.p.fd = event.fd;
		ev.p.events = event.flags == Event::EVENT_READ ? POLLIN : POLLOUT;
		ev.p.revents = 0;
		ev.ud = const_cast<HandleInfo*>(&event.handle_info);
		fds_[event.fd] = ev;
	}
	void remove_event(const Event& event) {
		fds_.erase(event.fd);
	}
};
ASYNCIO_NS_END
#endif	//ASYNCIO_WIN32_SELECTOR_H