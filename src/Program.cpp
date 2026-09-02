// SPDX-License-Identifier: MIT
#include <exception>
#include <iostream>

#include "CNA/Platform/Entrypoint.hpp"

#include "ChaosLog.hpp"
#include "CliOptions.hpp"
#include "CnaKillerGame.hpp"

int main(int argc, char** argv)
{
    const CnaKiller::CliOptions options = CnaKiller::CliOptions::Parse(argc, argv);

    if (options.showHelp)
    {
        CnaKiller::PrintHelp(argc > 0 ? argv[0] : "cna-killer");
        return 0;
    }
    if (options.parseError)
    {
        std::cerr << "cna-killer: " << options.errorMessage << "\n\n";
        CnaKiller::PrintHelp(argc > 0 ? argv[0] : "cna-killer");
        return 2;
    }

    std::cout << "cna-killer: a deliberately malicious CNA fuzz tester\n"
              << "  seed:       " << options.seed
              << (options.seedExplicit ? " (explicit)" : " (randomly chosen)") << "\n"
              << "  intensity:  " << CnaKiller::ToString(options.intensity) << "\n"
              << "  log:        " << options.logPath << "\n"
              << "  reproduce a crash with: " << (argc > 0 ? argv[0] : "cna-killer")
              << " --seed " << options.seed << "\n"
              << std::endl;

    CnaKiller::ChaosLog log(options.logPath, options.seed, CnaKiller::ToString(options.intensity),
                            argc, argv);

    try
    {
        CnaKiller::CnaKillerGame game(options, log);
        game.Run();
    }
    catch (const std::exception& ex)
    {
        log.Note(std::string("terminated by uncaught exception: ") + ex.what());
        std::cerr << "\ncna-killer: uncaught exception: " << ex.what()
                  << "\ncna-killer: reproduce with --seed " << options.seed << "\n";
        return 1;
    }
    catch (...)
    {
        log.Note("terminated by an uncaught non-std::exception");
        std::cerr << "\ncna-killer: terminated by an unknown exception"
                  << "\ncna-killer: reproduce with --seed " << options.seed << "\n";
        return 1;
    }

    log.Note("clean shutdown");
    std::cout << "cna-killer: clean shutdown after the configured stop condition." << std::endl;
    return 0;
}
