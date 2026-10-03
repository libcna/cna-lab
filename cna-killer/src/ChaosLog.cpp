// SPDX-License-Identifier: MIT
#include "ChaosLog.hpp"

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <iomanip>
#include <iterator>
#include <sstream>

#if defined(__unix__) || defined(__APPLE__)
    #define CNA_KILLER_HAVE_POSIX_SIGNALS 1
    #include <csignal>
    #include <unistd.h>
#else
    #define CNA_KILLER_HAVE_POSIX_SIGNALS 0
#endif

namespace CnaKiller
{
#if CNA_KILLER_HAVE_POSIX_SIGNALS
    namespace
    {
        // Everything the signal handler touches must be async-signal-safe: plain data,
        // pre-formatted text, and the write(2) syscall. No malloc, no iostream, no locking.
        constexpr std::size_t kBannerCapacity = 512;
        char g_crashBanner[kBannerCapacity] = {};
        std::size_t g_crashBannerLen = 0;

        constexpr int kWatchedSignals[] = {SIGSEGV, SIGABRT, SIGFPE, SIGILL, SIGBUS};
        struct sigaction g_previousHandlers[std::size(kWatchedSignals)] = {};
        bool g_handlersInstalled = false;

        extern "C" void HandleFatalSignal(int signum)
        {
            if (g_crashBannerLen > 0)
                (void)::write(STDERR_FILENO, g_crashBanner, g_crashBannerLen);

            // Restore whatever was previously installed for this exact signal, then re-raise
            // so the process terminates (and core-dumps) the normal way instead of looping.
            for (std::size_t i = 0; i < std::size(kWatchedSignals); ++i)
            {
                if (kWatchedSignals[i] == signum)
                {
                    ::sigaction(signum, &g_previousHandlers[i], nullptr);
                    break;
                }
            }
            ::raise(signum);
        }

        void InstallHandlers()
        {
            if (g_handlersInstalled)
                return;
            struct sigaction action{};
            action.sa_handler = &HandleFatalSignal;
            ::sigemptyset(&action.sa_mask);
            // SA_NODEFER: a second fatal signal while the first is being handled (plausible --
            // we are mid-crash inside a fuzz tester) should still reach this handler rather than
            // being blocked, so its banner has a chance to be written too.
            action.sa_flags = SA_NODEFER;
            for (std::size_t i = 0; i < std::size(kWatchedSignals); ++i)
                ::sigaction(kWatchedSignals[i], &action, &g_previousHandlers[i]);
            g_handlersInstalled = true;
        }

        void RestoreHandlers()
        {
            if (!g_handlersInstalled)
                return;
            for (std::size_t i = 0; i < std::size(kWatchedSignals); ++i)
                ::sigaction(kWatchedSignals[i], &g_previousHandlers[i], nullptr);
            g_handlersInstalled = false;
        }

        void RefreshBanner(std::uint64_t seed, std::uint64_t tick, const std::string& actionName)
        {
            const int written = std::snprintf(
                g_crashBanner, kBannerCapacity,
                "\ncna-killer: fatal signal while running action '%s' at tick %llu "
                "(seed=%llu) -- rerun with `cna-killer --seed %llu` to reproduce.\n",
                actionName.c_str(),
                static_cast<unsigned long long>(tick),
                static_cast<unsigned long long>(seed),
                static_cast<unsigned long long>(seed));
            g_crashBannerLen = (written > 0)
                ? std::min(static_cast<std::size_t>(written), kBannerCapacity - 1)
                : 0;
        }
    }
#endif

    namespace
    {
        std::string Timestamp()
        {
            const auto now = std::chrono::system_clock::now();
            const std::time_t nowTime = std::chrono::system_clock::to_time_t(now);
            std::tm tmBuffer{};
#if defined(_WIN32)
            gmtime_s(&tmBuffer, &nowTime);
#else
            gmtime_r(&nowTime, &tmBuffer);
#endif
            std::ostringstream out;
            out << std::put_time(&tmBuffer, "%Y-%m-%dT%H:%M:%SZ");
            return out.str();
        }
    }

    ChaosLog::ChaosLog(const std::string& path, std::uint64_t seed, const std::string& intensityName,
                       int argc, char** argv)
        : path_(path), file_(path, std::ios::app), seed_(seed)
    {
        std::ostringstream argvLine;
        for (int i = 0; i < argc; ++i)
        {
            if (i > 0)
                argvLine << ' ';
            argvLine << argv[i];
        }

        WriteLine("# cna-killer reproduction log");
        WriteLine("# started=" + Timestamp());
        WriteLine("# seed=" + std::to_string(seed_));
        WriteLine("# intensity=" + intensityName);
        WriteLine("# argv=" + argvLine.str());
        WriteLine("# replay: rerun with --seed " + std::to_string(seed_) +
                   " to reproduce this exact chaos sequence tick-for-tick.");

#if CNA_KILLER_HAVE_POSIX_SIGNALS
        RefreshBanner(seed_, 0, "<startup>");
        InstallHandlers();
#endif
    }

    ChaosLog::~ChaosLog()
    {
#if CNA_KILLER_HAVE_POSIX_SIGNALS
        RestoreHandlers();
#endif
    }

    void ChaosLog::WriteLine(const std::string& line)
    {
        file_ << line << '\n';
        file_.flush();
    }

    void ChaosLog::BeginAction(std::uint64_t tick, const std::string& actionName, const std::string& details)
    {
        std::ostringstream line;
        line << "[tick=" << tick << "] action=" << actionName;
        if (!details.empty())
            line << ' ' << details;
        WriteLine(line.str());

#if CNA_KILLER_HAVE_POSIX_SIGNALS
        RefreshBanner(seed_, tick, actionName);
#endif
    }

    void ChaosLog::Note(const std::string& text)
    {
        WriteLine("# " + text);
    }
}
