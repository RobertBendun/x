#include <cassert>
#include <coroutine>
#include <cstdint>
#include <iostream>
#include <optional>
#include <span>
#include <system_error>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>


struct Epoll_Wait
{
	int fd;
	int events;
};

struct Scheduler
{
	std::vector<std::coroutine_handle<>> immidiate; // Awaiting execution now
	std::vector<std::coroutine_handle<>> scheduled; // Awaiting execution in the future
	std::unordered_map<void*, int> suspended;

	int epoll_fd;

	Scheduler()
	{
		epoll_fd = epoll_create1(0);
		assert(epoll_fd >= 0 && "TODO: error handling");
	}

	~Scheduler()
	{
		close(epoll_fd);
	}

	Scheduler(Scheduler const&) = delete;
	Scheduler(Scheduler&&) = delete;
	Scheduler& operator=(Scheduler const&) = delete;
	Scheduler& operator=(Scheduler &&) = delete;

	void schedule(Epoll_Wait w, std::coroutine_handle<> cont)
	{
		epoll_event ev = {};
		ev.events = w.events;
		ev.data.ptr = cont.address();

		auto result = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, w.fd, &ev);
		if (result < 0) {
			perror("epoll_ctl");
			std::exit(2);
		}
		suspended[ev.data.ptr] = w.fd;
	}

	void schedule(std::coroutine_handle<> coroutine)
	{
		scheduled.push_back(coroutine);
	}

	void main_loop()
	{
		std::vector<epoll_event> events;

		while (scheduled.size() || suspended.size()) {
			immidiate.clear();
			std::swap(immidiate, scheduled);

			for (auto i = 0u; i < immidiate.size(); ++i) {
				auto status = immidiate[i];
				immidiate[i].resume();
				if (immidiate[i].done()) {
					immidiate[i].destroy();
				}
			}

			if (suspended.size()) {
				events.resize(suspended.size());

				auto ready_count = epoll_wait(epoll_fd, events.data(), events.size(), scheduled.size() ? 0 : -1);
				assert(ready_count >= 0 && "TODO: error handling");

				for (auto ev : std::span{events}.subspan(0, ready_count)) {
					auto continuation = std::coroutine_handle<>::from_address(ev.data.ptr);
					schedule(continuation);
					auto it = suspended.find(ev.data.ptr);
					epoll_ctl(epoll_fd, EPOLL_CTL_DEL, it->second, NULL);
					suspended.erase(it);
				}
			}
		}
	}
};

static Scheduler scheduler;

namespace coro
{
	template<typename T>
	struct Task;

	template<typename T>
	struct Promise;

	template<typename P, typename Result>
	struct Promise_Implementation
	{
		Task<Result> get_return_object()
		{
			return {std::coroutine_handle<P>::from_promise(*(P*)this)};
		}

		std::suspend_always initial_suspend() { return {}; }

		void unhandled_exception()
		{
			assert(false && "not implemented yet");
		}

		auto await_transform(Epoll_Wait w)
		{
			struct Epoll_Awaitable : Epoll_Wait
			{
				// TODO: This could be optimized.
				bool await_ready() { return false; }

				void await_suspend(std::coroutine_handle<P> handle)
				{
					scheduler.schedule(*this, handle);
				}

				void await_resume() {}
			};

			return Epoll_Awaitable{w};
		}

		template<typename T>
		auto await_transform(Task<T> task)
		{
			struct Invoke
			{
				std::coroutine_handle<Promise<T>> call;

				bool await_ready() { return false; }

				std::coroutine_handle<> await_suspend(std::coroutine_handle<> self)
				{
					call.promise().continuation = self;
					return call;
				}

				T await_resume()
				{
					return *call.promise().returned;
				}
			};

			return Invoke{std::move(task)};
		}

		auto final_suspend() noexcept
		{
			struct Continue
			{
				bool await_ready() noexcept { return false; }
				void await_resume() noexcept {}

				std::coroutine_handle<> await_suspend(std::coroutine_handle<P> self) noexcept
				{
					if (auto cont = self.promise().continuation; cont) {
						return cont;
					}
					return std::noop_coroutine();
				}
			};

			return Continue{};
		}
	};

	template<typename T>
	struct Promise : Promise_Implementation<Promise<T>, T>
	{
		using result_type = T;
		std::coroutine_handle<> continuation;
		std::optional<T> returned;

		void return_value(T return_value)
		{
			returned = std::move(return_value);
		}
	};

	template<>
	struct Promise<void> : Promise_Implementation<Promise<void>, void>
	{
		using result_type = void;
		std::coroutine_handle<> continuation;
		void return_void() {}
	};

	template<typename T>
	struct [[nodiscard]] Task
	{
		using promise_type = Promise<T>;
		std::coroutine_handle<Promise<T>> handle;
		bool empty = true;

		Task(std::coroutine_handle<Promise<T>> handle)
			: handle(handle), empty(false)
		{
		}

		Task(Task &&other)
			: handle(other.handle), empty(false)
		{
			other = {};
		}

		Task(Task const&) = delete;

		// Scheduler should deallocate
		~Task() { assert(empty); }

		operator std::coroutine_handle<>() && { empty = true; return std::exchange(handle, {}); }
		operator std::coroutine_handle<Promise<T>>() && { empty = true; return std::exchange(handle, {}); }
	};


	Task<int> accept(int server)
	{
		struct sockaddr saddr = {};
		socklen_t socklen = sizeof(saddr);

		co_await Epoll_Wait { .fd = server, .events = EPOLLIN };
		auto client = ::accept(server, &saddr, &socklen);
		assert(client >= 0 && "TODO: error handling");

		auto result = fcntl(client, F_SETFL, O_NONBLOCK);
		assert(result >= 0 && "TODO: error handling");
		co_return client;
	}

	Task<std::span<std::uint8_t>> read(int fd, std::span<uint8_t> data)
	{
		co_await Epoll_Wait { .fd = fd, .events = EPOLLIN };
		auto recv = ::read(fd, data.data(), data.size_bytes());
		assert(recv >= 0 && "TODO: error handling");
		co_return data.subspan(0, recv);
	}

	Task<std::tuple<std::span<std::uint8_t>, std::errc>> send(int fd, std::span<uint8_t> data, int flags)
	{
		co_await Epoll_Wait { .fd = fd, .events = EPOLLOUT };
		auto written = ::send(fd, data.data(), data.size_bytes(), flags);
		if (written <= 0) {
			co_return {{}, std::errc(errno)};
		}
		co_return {data.subspan(written), {}};
	}
}

int listen(std::string interface, std::uint16_t port)
{
	int result;

	int server = socket(AF_INET, SOCK_STREAM, 0);
	assert(server >= 0 && "TODO: error handling");

	result = fcntl(server, F_SETFL, O_NONBLOCK);
	assert(result >= 0 && "TODO: error handling");

	int enable = 1;
	result = setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable));
	assert(result >= 0 && "TODO: error handling");

	sockaddr_in sockaddr = {}; // TODO: Use address provided by interface parameter instead of loopback
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	sockaddr.sin_port = htons(port);

	result = bind(server, (struct sockaddr*)&sockaddr, sizeof(sockaddr));
	if (result < 0) {
		perror("bind");
		std::exit(2);
	}

	result = ::listen(server, 8);
	if (result < 0) {
		perror("listen");
		std::exit(2);
	}

	return server;
}

coro::Task<void> echo(int server_fd)
{
	for (;;) {
		int fd = co_await coro::accept(server_fd);

		scheduler.schedule([](int fd) -> coro::Task<void> {
			std::array<std::uint8_t, 1024> buf;
			for (;;) {
				auto data = co_await coro::read(fd, buf);
				do {
					std::errc ec;
					std::tie(data, ec) = co_await coro::send(fd, data, MSG_NOSIGNAL);
					if (ec != std::errc{}) {
						if (ec != std::errc::broken_pipe) { std::cerr << "error: " << std::make_error_code(ec).message() << "\n"; }
						co_return;
					}
				} while (data.size());
			}
		}(fd));
	}
	co_return;
}

int main()
{
	auto fd = listen("0.0.0.0", 8080);
	scheduler.schedule(echo(fd));
	scheduler.main_loop();
}

