// SPDX-License-Identifier: MIT
#include <exception>
#include <iostream>

#include "CNA/Platform/Entrypoint.hpp"

#include "ChaosEngine.hpp"
#include "ChaosLog.hpp"
#include "CliOptions.hpp"
#include "CnaKillerGame.hpp"
#include "Findings.hpp"

// Exit status: 0 clean run without findings, 4 clean run with findings, 3 stopped at the first
// finding by --strict, 1 ended by an exception, 2 bad arguments. A crash is a signal.
int main(int argc, char** argv)
{
    const CnaKiller::CliOptions options = CnaKiller::CliOptions::Parse(argc, argv);

    if (options.showHelp)
    {
        CnaKiller::PrintHelp(argc > 0 ? argv[0] : "cna-killer");
        return 0;
    }
    if (options.listActions)
    {
        CnaKiller::ChaosEngine::ListActions(std::cout);
        return 0;
    }
    std::string filterError;
    if (options.parseError || !CnaKiller::ChaosEngine::ValidateFilter(options, filterError))
    {
        std::cerr << "cna-killer: " << (options.parseError ? options.errorMessage : filterError) << "\n\n";
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
    CnaKiller::Findings findings(log, options.strict);

    try
    {
        CnaKiller::CnaKillerGame game(options, log, findings);
        game.Run();
    }
    catch (const CnaKiller::StrictStop&)
    {
        log.Note("stopped at the first finding (--strict)");
        findings.WriteSummary();
        std::cerr << "\ncna-killer: stopped at the first finding; reproduce with --seed " << options.seed << "\n";
        return 3;
    }
    catch (const std::exception& ex)
    {
        log.Note(std::string("terminated by uncaught exception: ") + ex.what());
        findings.WriteSummary();
        std::cerr << "\ncna-killer: uncaught exception: " << ex.what()
                  << "\ncna-killer: reproduce with --seed " << options.seed << "\n";
        return 1;
    }
    catch (...)
    {
        log.Note("terminated by an uncaught non-std::exception");
        findings.WriteSummary();
        std::cerr << "\ncna-killer: terminated by an unknown exception"
                  << "\ncna-killer: reproduce with --seed " << options.seed << "\n";
        return 1;
    }

    log.Note("clean shutdown");
    findings.WriteSummary();
    std::cout << "cna-killer: clean shutdown after the configured stop condition." << std::endl;
    return findings.Distinct() == 0 ? 0 : 4;
}
