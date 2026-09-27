#include "BackroomsGame.hpp"

#include <cstdint>
#include <exception>
#include <iostream>
#include <string>

int main(int argc, char** argv) {
    try {
        std::uint64_t seed = 0xBACC0005ULL;
        if (argc == 3 && std::string(argv[1]) == "--seed")
            seed = std::stoull(argv[2], nullptr, 0);
        else if (argc != 1) {
            std::cerr << "Usage: cna_backrooms [--seed unsigned-integer]\n";
            return 2;
        }
        Backrooms::BackroomsGame game(seed);
        game.Run();
    } catch (const std::exception& e) {
        std::cerr << "cna-backrooms: " << e.what() << '\n';
        return 1;
    }
}
