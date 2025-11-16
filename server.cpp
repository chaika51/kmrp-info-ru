#include <iostream>
#include <string>
#include <sstream>
#include <thread>
#include <vector>
#include <cstring>
#ifdef _WIN32
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#endif

std::string build_page(const std::string &title, const std::string &body) {
    std::stringstream html;
    html << "<!DOCTYPE html><html><head><meta charset='UTF-8'>"
         << "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
         << "<title>" << title << "</title></head><body style='font-family:sans-serif;'>"
         << "<h1>" << title << "</h1>" << body
         << "<br><br><a href='/' style='color:blue;'>Вернуться на главную</a>"
         << "</body></html>";
    return html.str();
}

std::string route(const std::string &path) {
    if (path == "/") {
        return build_page("Главная", 
            "<ul>"
            "<li><a href='/game'>О игре</a></li>"
            "<li><a href='/admin'>Как попасть в команду Администрации</a></li>"
            "<li><a href='/discord'>Дискорд сервер</a></li>"
            "<li><a href='/shop'>Наш магазин</a></li>"
            "</ul>");
    }
    if (path == "/game") {
        return build_page("О игре", "<p>Информация об игре...</p>");
    }
    if (path == "/admin") {
        return build_page("Как попасть в Администрацию", "<p>Условия и требования...</p>");
    }
    if (path == "/discord") {
        return build_page("Дискорд сервер", "<p>Ссылка на Discord: <b>ваша_ссылка</b></p>");
    }
    if (path == "/shop") {
        return build_page("Наш Магазин", "<p>Товары магазина...</p>");
    }
    return build_page("404", "<p>Страница не найдена.</p>");
}

void handle_client(int client_socket) {
    char buffer[4096];
    memset(buffer, 0, sizeof(buffer));
    recv(client_socket, buffer, sizeof(buffer), 0);

    std::string request(buffer);
    std::string path = "/";

    if (request.rfind("GET", 0) == 0) {
        size_t start = request.find(" ") + 1;
        size_t end = request.find(" ", start);
        path = request.substr(start, end - start);
    }

    std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=UTF-8\r\n\r\n" + route(path);

    send(client_socket, response.c_str(), response.size(), 0);

#ifdef _WIN32
    closesocket(client_socket);
#else
    close(client_socket);
#endif
}

int main() {
#ifdef _WIN32
    WSADATA wsa;
    WSAStartup(MAKEWORD(2,2), &wsa);
#endif

    int server_socket = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(8080);

    bind(server_socket, (sockaddr*)&server_addr, sizeof(server_addr));
    listen(server_socket, 5);

    std::cout << "Сервер запущен на порту 8080" << std::endl;

    while (true) {
        int client_socket = accept(server_socket, nullptr, nullptr);
        std::thread(handle_client, client_socket).detach();
    }

#ifdef _WIN32
    WSACleanup();
#endif

    return 0;
}
