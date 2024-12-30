#include <coroutine>
#include <iostream>
#include <utility>
#include <span>
#include <cstdint>
#include <memory>
#include <cassert>
#include <functional>
#include <system_error>
#include <tuple>

#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/ip.h>


std::vector<std::function<bool()>> executor_list;

namespace coro
{
	struct Promise
	{
		std::suspend_never initial_suspend() { return {}; }
		std::suspend_never final_suspend() noexcept { return {}; }

		void get_return_object() {}
		void return_void() {}

		void unhandled_exception()
		{
			assert(false && "not implemented yet");
		}
	};

	struct Task : std::coroutine_handle<Promise>
	{
		using promise_type = Promise;
	};

	template<typename T>
	struct Poll
	{
		T result;
		std::function<bool(T&)> poll;

		template<typename U>
		Poll(U&& t)
			: result{}, poll{std::forward<U>(t)}
		{
		}

		bool await_ready()
		{
			return poll(result);
		}

		void await_suspend(std::coroutine_handle<> handle)
		{
			// Schedule

			executor_list.emplace_back([handle, this] mutable -> bool {
				if (poll(result)) {
					handle.resume();
					return true;
				}
				return false;
			});
		}

		T await_resume()
		{
			return std::move(result);
		}
	};

	// TODO: Sort out lifetimes. This function cannot accept lvalue references, everything must be moved or copied
	void spawn(auto &&f, auto&& ...args)
	{
		executor_list.push_back([f = std::move(f), ...args = std::move(args)]() -> bool {
			f(std::move(args)...);
			return true;
		});
	}
}

namespace coro::net
{
	struct Socket
	{
		int sock_fd;

		Poll<std::span<std::uint8_t>> read(std::span<std::uint8_t>) const;
		Poll<std::span<std::uint8_t>> write(std::span<std::uint8_t>) const;
		Poll<Socket> accept() const;
	};

	Poll<Socket> Socket::accept() const
	{
		return [sock_fd = sock_fd](Socket &s) mutable -> bool {
			struct sockaddr saddr = {};
			socklen_t socklen = sizeof(saddr);

			s.sock_fd = ::accept(sock_fd, &saddr, &socklen);
			if (s.sock_fd < 0) {
				if (errno == EWOULDBLOCK) {
					return false;
				}
				assert(false && "TODO: error handling");
			}

			auto result = fcntl(s.sock_fd, F_SETFL, O_NONBLOCK);
			assert(result >= 0 && "TODO: error handling");
			return true;
		};
	}

	Poll<std::span<std::uint8_t>> Socket::read(std::span<std::uint8_t> buf) const
	{
		return [sock_fd = sock_fd, buf](std::span<std::uint8_t>& return_buf) -> bool {
			auto recv = ::read(sock_fd, buf.data(), buf.size());
			if (recv < 0) {
				if (errno == EWOULDBLOCK || errno == EAGAIN) {
					return false;
				}
				perror("read");
				exit(2);
			}
			return_buf = std::span{buf.data(), (std::size_t)recv};
			return true;
		};
	}

	Poll<std::span<std::uint8_t>> Socket::write(std::span<std::uint8_t> buf) const
	{
		return [sock_fd = sock_fd, buf](std::span<std::uint8_t>& return_buf) -> bool {
			auto written = ::write(sock_fd, buf.data(), buf.size());
			if (written < 0) {
				if (errno == EWOULDBLOCK || errno == EAGAIN) {
					return false;
				}
				perror("write");
				exit(2);
			}
			return_buf = std::span{buf.data() + written, buf.size() - written};
			return true;
		};
	}

	Socket listen(std::string interface, std::uint16_t port)
	{
		int result;

		Socket s;
		s.sock_fd = socket(AF_INET, SOCK_STREAM, 0);
		assert(s.sock_fd >= 0 && "TODO: error handling");

		result = fcntl(s.sock_fd, F_SETFL, O_NONBLOCK);
		assert(result >= 0 && "TODO: error handling");

		int enable = 1;
		result = setsockopt(s.sock_fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable));
		assert(result >= 0 && "TODO: error handling");

		sockaddr_in sockaddr = {}; // TODO: Use address provided by interface parameter instead of loopback
		sockaddr.sin_family = AF_INET;
		sockaddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
		sockaddr.sin_port = htons(port);

		result = bind(s.sock_fd, (struct sockaddr*)&sockaddr, sizeof(sockaddr));
		if (result < 0) {
			perror("bind");
			std::exit(2);
		}

		result = ::listen(s.sock_fd, 8);
		if (result < 0) {
			perror("listen");
			std::exit(2);
		}

		return s;
	}
}

coro::Task echo(coro::net::Socket s)
{
	for (;;) {
		std::cout << "Waiting for connection\n";
		auto client = co_await s.accept();
		std::cout << "Accepted connection\n";

		coro::spawn([](auto client) -> coro::Task {
			std::uint8_t buffer[1024];

			for (;;) {
				auto recv = co_await client.read(buffer);
				if (recv.empty()) break;
				while ((recv = co_await client.write(recv)).size()) {}
			}
			close(client.sock_fd);
		}, client);
	}
}

int main()
{
	auto s = coro::net::listen("127.0.0.1", 8080);
	coro::spawn(echo, s);

	decltype(executor_list) executing;

	for (;;) {
		std::swap(executor_list, executing);
		for (auto f : executing) {
			if (!f()) {
				executor_list.emplace_back(std::move(f));
			}
		}
		executing.clear();
	}

	close(s.sock_fd);
}
