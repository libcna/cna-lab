#include "CnaLabGame.hpp"

#include <exception>
#include <iostream>
#include <string_view>

int main(int argc, char* argv[])
{
    const bool smokeMedia = argc == 2 && std::string_view(argv[1]) == "--smoke-media";

    try
    {
        CnaLabGame game(smokeMedia);
        game.Run();
        if (smokeMedia && !game.SmokeSucceeded())
        {
            std::cerr << "cna-lab: media smoke verification failed\n";
            return 1;
        }
        return 0;
    }
    catch (const std::exception& exception)
    {
        std::cerr << "cna-lab: " << exception.what() << '\n';
        return 1;
    }
}
