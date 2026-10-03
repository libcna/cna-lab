// SPDX-License-Identifier: MIT
#pragma once

#include <cstdint>
#include <string>

namespace CnaKiller
{
    /**
     * @brief How aggressively the chaos engine hammers the CNA runtime.
     *
     * Higher intensities run more chaos actions per tick and let more live resources
     * accumulate before the engine is forced to reclaim them.
     */
    enum class Intensity
    {
        Low,
        Medium,
        High,
        Nightmare
    };

    /**
     * @brief Parsed command-line / environment configuration for a cna-killer run.
     *
     * Every knob here except the seed only changes *how much* chaos happens per tick.
     * The seed is the only source of randomness in the whole program (see ChaosEngine),
     * so two runs with the same seed and the same number of ticks always pick the exact
     * same sequence of malicious actions -- that is what makes a crash reproducible.
     */
    struct CliOptions
    {
        /** @brief Seed fed to the single System::Random instance that drives all chaos. */
        std::uint64_t seed = 0;

        /** @brief True if the seed came from --seed/CNA_KILLER_SEED rather than being random. */
        bool seedExplicit = false;

        /** @brief How many chaos actions are attempted per Update() tick. */
        Intensity intensity = Intensity::Medium;

        /** @brief Stop cleanly after this many ticks. 0 means "no limit". */
        std::uint64_t maxTicks = 0;

        /** @brief Stop cleanly after this many seconds of wall-clock run time. 0 means "no limit". */
        double maxSeconds = 0.0;

        /**
         * @brief Exit right after reaching this tick, before running that tick's actions.
         *
         * Used to bisect a crash: rerun with the same seed and this set to a tick shortly
         * before the one a previous run's log named as the last one reached, to reproduce
         * a small prefix of the same chaos sequence. 0 means "disabled".
         */
        std::uint64_t stopAtTick = 0;

        /** @brief Path to the append-only reproduction log. Empty means auto-generate one. */
        std::string logPath;

        /**
         * @brief Action names or families to run exclusively (comma-separated). Empty runs all.
         *
         * Filtering changes which actions the seed picks, so a filtered run is reproducible only
         * with the same filter -- it exists to narrow a finding down to one subsystem.
         */
        std::string onlyActions;

        /** @brief Action names or families never to run (comma-separated). */
        std::string excludedActions;

        /** @brief Stop at the first finding instead of recording it and carrying on. */
        bool strict = false;

        /** @brief Print every action with its family and exit. */
        bool listActions = false;

        /** @brief True if --help/-h was requested; the caller should print help and exit. */
        bool showHelp = false;

        /** @brief True if the arguments could not be parsed; the caller should exit(2). */
        bool parseError = false;

        /** @brief Human-readable reason for parseError, empty otherwise. */
        std::string errorMessage;

        /**
         * @brief Parses argv and layers CNA_KILLER_* environment variables underneath it.
         *
         * Command-line flags always win over the matching environment variable, which in
         * turn wins over the built-in default.
         */
        static CliOptions Parse(int argc, char** argv);
    };

    /** @brief Returns the lower-case name of an intensity level (e.g. "nightmare"). */
    const char* ToString(Intensity value);

    /** @brief Parses an intensity name (case-insensitive). Returns false if unrecognized. */
    bool TryParseIntensity(const std::string& text, Intensity& out);

    /** @brief Prints the --help usage text for the given program name to stdout. */
    void PrintHelp(const std::string& programName);
}
