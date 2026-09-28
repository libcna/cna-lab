#include "BackroomsGame.hpp"

#include <cstdint>
#include <exception>
#include <iostream>
#include <string>

int main(int argc, char** argv) {
    try {
        std::uint64_t seed = 0xBACC0005ULL;
        bool streamTest=false;
        int level=0, multiSampleCount=4;
        double x=2.5,z=2.5;
        double walkSpeed=Backrooms::BackroomsGame::kDefaultWalkSpeed;
        double runSpeed=Backrooms::BackroomsGame::kDefaultRunSpeed;
        double streamTestMetres=9600.0;
        float verticalFov=Backrooms::BackroomsGame::kDefaultVerticalFov;
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
            else if (argument=="--walk-speed" && i+1<argc)
                walkSpeed=std::stod(argv[++i]);
            else if (argument=="--run-speed" && i+1<argc)
                runSpeed=std::stod(argv[++i]);
            else if (argument=="--fov" && i+1<argc)
                verticalFov=std::stof(argv[++i]);
            else if (argument=="--msaa" && i+1<argc)
                multiSampleCount=std::stoi(argv[++i]);
            else if (argument=="--stream-test") streamTest=true;
            else if (argument=="--stream-test-metres" && i+1<argc) {
                streamTest=true;
                streamTestMetres=std::stod(argv[++i]);
            }
            else {
                std::cerr << "Usage: cna_backrooms [--seed unsigned-integer] "
                             "[--level 0|1|2] [--position x z] "
                             "[--walk-speed m/s] [--run-speed m/s] [--fov degrees] "
                             "[--msaa 0|4] [--stream-test] [--stream-test-metres distance]\n";
                return 2;
            }
        }
        Backrooms::BackroomsGame game(seed,streamTest,level,x,z,
                                      walkSpeed,runSpeed,streamTestMetres,verticalFov,multiSampleCount);
        game.Run();
    } catch (const std::exception& e) {
        std::cerr << "cna-backrooms: " << e.what() << '\n';
        return 1;
    }
}
