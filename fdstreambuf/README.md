# fdstreambuf

Provides C++ stream compatiliby for POSIX file descriptors

__Important__: Stream doesn't own file descriptor, so it won't close the descriptor or flush data implicitly when beeing destructed. User must ensure that before close buffer is synced, as shown in examples.

## Usage

File I/O:

```cpp
#include <algorithm>
#include <cassert>
#include <iterator>
#include <fcntl.h>

#define Fdstreambuf_Implementation
#include <fdstreambuf.hh>

void copy(char const* source_file, char const* destination_file)
{
  auto const ensure = [](int fd) { if (fd < 0) { perror("copy: "); exit(1); } return fd; };

  fdstreambuf out = ensure(open(destination_file, O_WRONLY | O_CREAT, 0644));
  fdstreambuf in = ensure(open(source_file, O_RDONLY));

  std::copy(
    std::istreambuf_iterator<char>(&in), {},
    std::ostreambuf_iterator<char>(&out));

  out.sync();

  close(out.fd);
  close(in.fd);
}

int main()
{
  copy(__FILE__, __FILE__ ".1");
}
```

Simple HTTP server:

```cpp
#include <ostream>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define Fdstreambuf_Implementation
#include <fdstreambuf.hh>

std::string_view Http_Header =
  "HTTP/1.1 200 OK\r\n"
  "Server: fdstreambuf example HTTP server\r\n"
  "Content-Type: text/html\r\n"
  "Connection: Closed\r\n"
  "\r\n";

void http()
{
  auto const ensure = [](int v) { if (v < 0) { perror("http: "); exit(1); } return v; };

  auto const sock = ensure(socket(AF_INET, SOCK_STREAM, 0));

  struct sockaddr_in sa;
  sa.sin_family = AF_INET;
  sa.sin_addr.s_addr = inet_addr("127.0.0.1");
  sa.sin_port = htons(8080);

  ensure(bind(sock, (struct sockaddr*)&sa, sizeof(sa)));
  ensure(listen(sock, 5));

  struct sockaddr incoming;
  socklen_t incoming_len = sizeof(incoming);
  for (size_t connections = 0;;) {
    fdstreambuf client = ensure(accept(sock, &incoming, &incoming_len));
    std::ostream response(&client);

    std::copy(Http_Header.begin(), Http_Header.end(), std::ostreambuf_iterator(&client));
    response << "<h1> Hello world! </h1>\n";
    response << "<p> This message has been send " << connections++ << " times </p>\n";

    auto status = client.sync();
    assert(status == 0);
    close(client.fd);
  }

  close(sock);
}
```
