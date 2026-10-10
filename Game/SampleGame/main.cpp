#include <iostream>
#include "Core/Engine.h"

int main()
{
    std::cout << "Sample Game\n";
    std::cout << "Engine Version: "
              << MyGameEngine::Engine::GetVersion()
              << '\n';

    return 0;
}
