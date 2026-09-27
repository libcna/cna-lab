#include "BackroomsGame.hpp"

#include <cstdint>
#include <exception>
#include <iostream>
#include <string>

int main(int argc, char** argv) {
    try {
        std::uint64_t seed = 0xBACC0005ULL;
        bool streamTest=false;
        int level=0;
        double x=2.5,z=2.5;
        for (int i=1;i<argc;++i) {
            const std::string argument=argv[i];
            if (argument=="--seed" && i+1<argc)
                seed=std::stoull(argv[++i],nullptr,0);
            else if (argument=="--level" && i+1<argc)
                level=std::stoi(argv[++i]);
            else if (argument=="--position" && i+2<argc) {
                x=std::stod(argv[++i]);
                z=std::stod(argv[++i]);
            }
            else if (argument=="--stream-test") streamTest=true;
            else {
                std::cerr << "Usage: cna_backrooms [--seed unsigned-integer] "
                             "[--level 0|1|2] [--position x z] "
                             "[--stream-test]\n";
                return 2;
            }
        }
        Backrooms::BackroomsGame game(seed,streamTest,level,x,z);
        game.Run();
    } catch (const std::exception& e) {
        std::cerr << "cna-backrooms: " << e.what() << '\n';
        return 1;
    }
}
