#include <iostream>
#include "Core/Engine.h"

int main()
{
    std::cout << "MyGameEngine Editor\n";
    std::cout << "Engine Version: "
              << MyGameEngine::Engine::GetVersion()
              << '\n';

    return 0;
}
