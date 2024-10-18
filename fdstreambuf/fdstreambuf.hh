#ifndef Bendun_Fdstreambuf
#define Bendun_Fdstreambuf

#include <streambuf>
#include <vector>

struct fdstreambuf : std::streambuf
{
	static constexpr size_t Capacity = 4096;

	int fd;
	std::vector<char> in, out;

	inline fdstreambuf(int fd)
		: fd(fd)
	{
		setg(nullptr, nullptr, nullptr);
		setp(nullptr, nullptr);
	}

	fdstreambuf(fdstreambuf const&) = delete;
	fdstreambuf(fdstreambuf &&)     = default;

	// Ensures that at least one character is in buffer by reading more data if it's available
	int_type underflow() override;

	// Ensures that at least one character can be written and stores it if it's not EOF
	int_type overflow(int_type ch = traits_type::eof()) override;

	// Flushes content of out buffer
	int sync() override;

	~fdstreambuf() override = default;
};
#endif

#ifdef Fdstreambuf_Implementation

#include <unistd.h>

fdstreambuf::int_type fdstreambuf::underflow()
{
	// If we have data available in buffer, we return current character
	if (gptr() < egptr()) return traits_type::to_int_type(*gptr());

	if (in.empty()) in.resize(Capacity);

	// TODO Better error reporting
	auto n = ::read(fd, in.data(), in.size());
	if (n <= 0) {
		auto const end = &in.back() + 1;
		setg(end, end, end);
		return traits_type::eof();
	}

	setg(in.data(), in.data(), in.data() + n);
	return traits_type::to_int_type(in[0]);
}

fdstreambuf::int_type fdstreambuf::overflow(fdstreambuf::int_type ch)
{
	if (pptr() == nullptr) {
		out.resize(Capacity);
		setp(out.data(), &out.back() + 1);
		goto write;
	}

	if (pptr() >= epptr() && sync() == -1) {
		return ~traits_type::eof();
	}

write:
	if (ch != traits_type::eof()) {
		*pptr() = traits_type::to_char_type(ch);
		pbump(1);
	}

	return ~traits_type::eof();
}

int fdstreambuf::sync()
{
	auto p = out.data();
	auto n = pptr() - out.data() + 1;
	for (;;) {
		auto w = ::write(fd, p, n);
		// TODO Better error reporting
		if (w < 0)         return -1;
		if ((n -= w) == 0) break;
		p += w;
	}

	setp(out.data(), out.data() + out.size());
	return 0;
}
#endif
