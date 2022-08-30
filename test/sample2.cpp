#include <iostream>
#include <string>

std::string msg = "Hello, world!";

int main(int argc, char** argv)
{
    std::cout << msg << std::endl;

    int x, y;
    std::cin >> x >> y;
    std::cout << x + y << std::endl;

    std::cout << "Arguments (" << argc << "): ";
    for (int i = 1; i < argc; i++)
        std::cout << argv[i] << " ";
    std::cout << std::endl;
}