#include <iostream>
#include <string>
#include <sstream>
#include <cstdio>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

int main() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);   // IPv4, TCP
    if (server_fd < 0) { perror("socket"); return 1; }

    // Lets you restart the server immediately without "address in use" errors
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;   // listen on all network interfaces
    addr.sin_port = htons(8080);         // htons = convert to network byte order

    if (bind(server_fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind"); return 1;
    }
    if (listen(server_fd, 10) < 0) {     // 10 = queue length for waiting clients
        perror("listen"); return 1;
    }
    std::cout << "Listening on http://localhost:8080\n";

	while(true) {
		int client_fd = accept(server_fd, nullptr, nullptr);
		if (client_fd < 0) { perror("accept"); continue;};

		char buffer[4096];
		ssize_t n = read(client_fd, buffer, sizeof(buffer) - 1);
		if (n <= 0) { close(client_fd); continue; }
		std::string request(buffer, n);

		std::istringstream stream(request);
		std::string method, path, version;
		stream >> method >> path >> version;
		std::cout << method << " " << path << " " << version << std::endl;

		        std::string status, body;
        if (path == "/") {
            status = "200 OK";
            body = "<h1>Welcome!</h1><p>Try <a href=\"/hello\">/hello</a></p>";
        } else if (path == "/hello") {
            status = "200 OK";
            body = "<h1>Hello from C++!</h1>";
        } else {
            status = "404 Not Found";
            body = "<h1>404 - Page not found</h1>";
        }

        std::string response =
            "HTTP/1.1 " + status + "\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: " + std::to_string(body.size()) + "\r\n"
            "Connection: close\r\n"
            "\r\n" + body;

        write(client_fd, response.c_str(), response.size());
        close(client_fd);
    }

    close(server_fd);
	return 0;
}
