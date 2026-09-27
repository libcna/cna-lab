#include "BackroomsGame.hpp"

#include <cstdint>
#include <exception>
#include <iostream>
#include <string>

int main(int argc, char** argv) {
    try {
        std::uint64_t seed = 0xBACC0005ULL;
        bool streamTest=false;
        for (int i=1;i<argc;++i) {
            const std::string argument=argv[i];
            if (argument=="--seed" && i+1<argc)
                seed=std::stoull(argv[++i],nullptr,0);
            else if (argument=="--stream-test") streamTest=true;
            else {
                std::cerr << "Usage: cna_backrooms [--seed unsigned-integer] "
                             "[--stream-test]\n";
                return 2;
            }
        }
        Backrooms::BackroomsGame game(seed,streamTest);
        game.Run();
    } catch (const std::exception& e) {
        std::cerr << "cna-backrooms: " << e.what() << '\n';
        return 1;
    }
}
