// SPDX-License-Identifier: MIT
#include "CliOptions.hpp"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <random>
#include <sstream>

namespace CnaKiller
{
    namespace
    {
        std::string ToLower(std::string text)
        {
            std::transform(text.begin(), text.end(), text.begin(),
                           [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
            return text;
        }

        std::optional<std::string> GetEnv(const char* name)
        {
            const char* value = std::getenv(name);
            if (value == nullptr || value[0] == '\0')
                return std::nullopt;
            return std::string(value);
        }

        /** @brief Splits "--flag=value" into ("--flag", "value"); returns false if there is no '='. */
        bool SplitEquals(const std::string& arg, std::string& flag, std::string& value)
        {
            const auto pos = arg.find('=');
            if (pos == std::string::npos)
                return false;
            flag = arg.substr(0, pos);
            value = arg.substr(pos + 1);
            return true;
        }

        bool ParseU64(const std::string& text, std::uint64_t& out)
        {
            try
            {
                std::size_t consumed = 0;
                const unsigned long long parsed = std::stoull(text, &consumed);
                if (consumed != text.size())
                    return false;
                out = static_cast<std::uint64_t>(parsed);
                return true;
            }
            catch (...)
            {
                return false;
            }
        }

        bool ParseDouble(const std::string& text, double& out)
        {
            try
            {
                std::size_t consumed = 0;
                out = std::stod(text, &consumed);
                return consumed == text.size();
            }
            catch (...)
            {
                return false;
            }
        }

        std::uint64_t RandomSeed()
        {
            std::random_device rd;
            // random_device may only supply 32 bits of entropy on some platforms; combine two
            // draws so the default seed still spans the full 64-bit space System::Random accepts.
            const std::uint64_t hi = static_cast<std::uint64_t>(rd());
            const std::uint64_t lo = static_cast<std::uint64_t>(rd());
            return (hi << 32) ^ lo;
        }
    }

    const char* ToString(Intensity value)
    {
        switch (value)
        {
            case Intensity::Low:       return "low";
            case Intensity::Medium:    return "medium";
            case Intensity::High:      return "high";
            case Intensity::Nightmare: return "nightmare";
        }
        return "medium";
    }

    bool TryParseIntensity(const std::string& text, Intensity& out)
    {
        const std::string lower = ToLower(text);
        if (lower == "low")       { out = Intensity::Low;       return true; }
        if (lower == "medium")    { out = Intensity::Medium;    return true; }
        if (lower == "high")      { out = Intensity::High;      return true; }
        if (lower == "nightmare") { out = Intensity::Nightmare; return true; }
        return false;
    }

    void PrintHelp(const std::string& programName)
    {
        std::cout <<
            "cna-killer -- a deliberately malicious CNA fuzz tester disguised as a game\n"
            "\n"
            "Usage: " << programName << " [options]\n"
            "\n"
            "Options:\n"
            "  --seed=N            Seed the chaos engine's RNG with N (uint64). Reusing a seed\n"
            "                      reproduces the exact same sequence of chaos actions.\n"
            "  --intensity=LEVEL   One of: low, medium, high, nightmare. Default: medium.\n"
            "  --max-ticks=N       Exit cleanly after N Update() ticks. Default: unlimited.\n"
            "  --duration=SECONDS  Exit cleanly after SECONDS of wall-clock run time.\n"
            "  --stop-at-tick=N    Exit right before tick N runs (for bisecting a crash).\n"
            "  --log=PATH          Reproduction log path. Default: cna-killer-<seed>.log\n"
            "  --only=LIST         Run only these actions or families (comma-separated).\n"
            "  --exclude=LIST      Never run these actions or families (comma-separated).\n"
            "  --strict            Stop at the first finding (exit status 3).\n"
            "  --list-actions      Print every action and its family, then exit.\n"
            "  -h, --help          Show this help and exit.\n"
            "\n"
            "Environment variables (overridden by the matching flag above):\n"
            "  CNA_KILLER_SEED, CNA_KILLER_INTENSITY, CNA_KILLER_MAX_TICKS,\n"
            "  CNA_KILLER_DURATION, CNA_KILLER_LOG, CNA_KILLER_ONLY, CNA_KILLER_EXCLUDE,\n"
            "  CNA_KILLER_STRICT (1 = on)\n";
    }

    CliOptions CliOptions::Parse(int argc, char** argv)
    {
        CliOptions options;

        // Layer 1: built-in defaults (already set by the member initializers above).

        // Layer 2: environment variables.
        if (const auto env = GetEnv("CNA_KILLER_SEED"))
        {
            std::uint64_t parsed = 0;
            if (ParseU64(*env, parsed))
            {
                options.seed = parsed;
                options.seedExplicit = true;
            }
        }
        if (const auto env = GetEnv("CNA_KILLER_INTENSITY"))
        {
            Intensity parsed{};
            if (TryParseIntensity(*env, parsed))
                options.intensity = parsed;
        }
        if (const auto env = GetEnv("CNA_KILLER_MAX_TICKS"))
        {
            std::uint64_t parsed = 0;
            if (ParseU64(*env, parsed))
                options.maxTicks = parsed;
        }
        if (const auto env = GetEnv("CNA_KILLER_DURATION"))
        {
            double parsed = 0.0;
            if (ParseDouble(*env, parsed))
                options.maxSeconds = parsed;
        }
        if (const auto env = GetEnv("CNA_KILLER_LOG"))
        {
            options.logPath = *env;
        }
        if (const auto env = GetEnv("CNA_KILLER_ONLY"))
        {
            options.onlyActions = *env;
        }
        if (const auto env = GetEnv("CNA_KILLER_EXCLUDE"))
        {
            options.excludedActions = *env;
        }
        if (const auto env = GetEnv("CNA_KILLER_STRICT"))
        {
            options.strict = *env == "1";
        }

        // Layer 3: command-line flags, highest priority.
        const std::string programName = (argc > 0) ? argv[0] : "cna-killer";
        for (int i = 1; i < argc; ++i)
        {
            const std::string arg = argv[i];

            if (arg == "-h" || arg == "--help")
            {
                options.showHelp = true;
                continue;
            }
            if (arg == "--strict")
            {
                options.strict = true;
                continue;
            }
            if (arg == "--list-actions")
            {
                options.listActions = true;
                continue;
            }

            std::string flag = arg;
            std::string value;
            bool hasValue = SplitEquals(arg, flag, value);
            if (!hasValue && i + 1 < argc &&
                (flag == "--seed" || flag == "--intensity" || flag == "--max-ticks" ||
                 flag == "--duration" || flag == "--stop-at-tick" || flag == "--log" ||
                 flag == "--only" || flag == "--exclude"))
            {
                value = argv[++i];
                hasValue = true;
            }

            if (flag == "--seed")
            {
                std::uint64_t parsed = 0;
                if (!hasValue || !ParseU64(value, parsed))
                {
                    options.parseError = true;
                    options.errorMessage = "--seed requires a non-negative integer argument";
                    return options;
                }
                options.seed = parsed;
                options.seedExplicit = true;
            }
            else if (flag == "--intensity")
            {
                Intensity parsed{};
                if (!hasValue || !TryParseIntensity(value, parsed))
                {
                    options.parseError = true;
                    options.errorMessage = "--intensity must be one of: low, medium, high, nightmare";
                    return options;
                }
                options.intensity = parsed;
            }
            else if (flag == "--max-ticks")
            {
                std::uint64_t parsed = 0;
                if (!hasValue || !ParseU64(value, parsed))
                {
                    options.parseError = true;
                    options.errorMessage = "--max-ticks requires a non-negative integer argument";
                    return options;
                }
                options.maxTicks = parsed;
            }
            else if (flag == "--duration")
            {
                double parsed = 0.0;
                if (!hasValue || !ParseDouble(value, parsed))
                {
                    options.parseError = true;
                    options.errorMessage = "--duration requires a number of seconds";
                    return options;
                }
                options.maxSeconds = parsed;
            }
            else if (flag == "--stop-at-tick")
            {
                std::uint64_t parsed = 0;
                if (!hasValue || !ParseU64(value, parsed))
                {
                    options.parseError = true;
                    options.errorMessage = "--stop-at-tick requires a non-negative integer argument";
                    return options;
                }
                options.stopAtTick = parsed;
            }
            else if (flag == "--log")
            {
                if (!hasValue || value.empty())
                {
                    options.parseError = true;
                    options.errorMessage = "--log requires a path argument";
                    return options;
                }
                options.logPath = value;
            }
            else if (flag == "--only" || flag == "--exclude")
            {
                if (!hasValue || value.empty())
                {
                    options.parseError = true;
                    options.errorMessage = flag + " requires a comma-separated list of actions";
                    return options;
                }
                (flag == "--only" ? options.onlyActions : options.excludedActions) = value;
            }
            else
            {
                options.parseError = true;
                options.errorMessage = "unrecognized option: " + arg;
                return options;
            }
        }

        // Layer 4: a seed no one asked for still has to be picked, deterministically-per-run,
        // from real entropy -- and then reported/logged so the run can be replayed later.
        if (!options.seedExplicit)
            options.seed = RandomSeed();

        if (options.logPath.empty())
            options.logPath = "cna-killer-" + std::to_string(options.seed) + ".log";

        (void)programName;
        return options;
    }
}
