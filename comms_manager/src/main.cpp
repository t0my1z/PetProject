#include <asio.hpp>
#include <iostream>

int main() {
    asio::io_context io_context;

    std::cout << "Hello, Asio!" << std::endl;

    return 0;
}