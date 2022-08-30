#include <iostream>
#include <string>

std::string msg = "Hello, world!";

int main(int argc, char** argv)
{
    std::cout << msg << std::endl;

    int x, y;
    std::cin >> x >> y;
    std::cout << x + y << std::endl;

    std::cout << "Arguments: ";
    for (int i = 1; i < argc; i++)
        std::cout << argv[argc] << " ";
    std::cout << std::endl;
}