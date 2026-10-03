// SPDX-License-Identifier: MIT
#pragma once

#include <cstddef>
#include <cstdint>
#include <exception>
#include <map>
#include <string>
#include <typeinfo>

#include "ChaosLog.hpp"

namespace CnaKiller
{
    /**
     * @brief What kind of wrong behaviour a finding records, most severe first.
     *
     * A crash is not among them: a crash ends the process, and ChaosLog's signal banner plus the
     * log's last action line are its record.
     */
    enum class FindingKind
    {
        /** A call XNA accepts threw, or a call XNA refuses threw something other than an exception. */
        UnexpectedException,
        /** Data read back is not the data written, or a rendered pixel is not the XNA result. */
        Mismatch,
        /** A call XNA refuses with an exception completed silently. */
        MissingRefusal,
        /** A call XNA refuses was refused with a different exception type than XNA's. */
        WrongRefusal,
        /** A resource count or memory use kept growing while the pools stayed bounded. */
        Leak,
    };

    /** @brief Returns the upper-case log name of a finding kind. */
    const char* ToString(FindingKind kind);

    /** @brief Thrown to end the run at the first finding under --strict. */
    struct StrictStop final : std::exception
    {
        [[nodiscard]] const char* what() const noexcept override
        {
            return "strict mode: stopping at the first finding";
        }
    };

    /** @brief The demangled dynamic type of an exception, e.g. "System::ArgumentException". */
    std::string ExceptionTypeName(const std::exception& exception);

    /** @brief The demangled name of a static type. */
    std::string TypeName(const std::type_info& type);

    /**
     * @brief Collects what cna-killer caught CNA doing wrong without crashing.
     *
     * Each finding is keyed by kind, action and a stable description, so the same defect hit a
     * thousand times is one entry with a count; the first few occurrences are written to the log
     * in full, with the tick, and the rest only counted. The summary at the end of a run lists
     * every distinct finding once.
     */
    class Findings
    {
    public:
        Findings(ChaosLog& log, bool strict);

        /** @brief Records the tick the next findings belong to. */
        void SetTick(std::uint64_t tick) { tick_ = tick; }

        /**
         * @brief Records one finding.
         *
         * @param kind   What went wrong.
         * @param action The chaos action that observed it.
         * @param what   A stable description; findings with the same kind/action/what are one entry.
         * @param detail The specifics of this occurrence (sizes, values, exception text).
         * @throws StrictStop under --strict.
         */
        void Report(FindingKind kind, const std::string& action, const std::string& what,
                    const std::string& detail);

        /** @brief Counts an XNA refusal that CNA reproduced correctly. */
        void CountRefusal() { ++refusals_; }

        /** @brief Counts a verification that passed. */
        void CountCheck() { ++checks_; }

        [[nodiscard]] std::size_t Distinct() const { return entries_.size(); }
        [[nodiscard]] std::size_t Total() const { return total_; }

        /** @brief Writes the per-finding summary to the log and to stdout. */
        void WriteSummary();

    private:
        struct Entry
        {
            FindingKind kind;
            std::string action;
            std::string what;
            std::string firstDetail;
            std::uint64_t firstTick = 0;
            std::size_t count = 0;
        };

        static constexpr std::size_t kLoggedOccurrences = 3;

        ChaosLog& log_;
        bool strict_;
        std::uint64_t tick_ = 0;
        std::map<std::string, Entry> entries_;
        std::size_t total_ = 0;
        std::size_t refusals_ = 0;
        std::size_t checks_ = 0;
    };

    /**
     * @brief Runs @p call, which XNA refuses by throwing @p Expected (or a type derived from it).
     *
     * A matching exception is counted as a correct refusal. Anything else thrown is a
     * WrongRefusal, and a call that returns normally is a MissingRefusal.
     */
    template <typename Expected, typename Call>
    void ExpectRefusal(Findings& findings, const std::string& action, const std::string& what,
                       Call&& call)
    {
        try
        {
            call();
        }
        catch (const Expected&)
        {
            findings.CountRefusal();
            return;
        }
        catch (const StrictStop&)
        {
            throw;
        }
        catch (const std::exception& exception)
        {
            findings.Report(FindingKind::WrongRefusal, action, what,
                            "XNA throws " + TypeName(typeid(Expected)) + "; CNA threw " +
                                ExceptionTypeName(exception) + ": " + exception.what());
            return;
        }
        findings.Report(FindingKind::MissingRefusal, action, what,
                        "XNA throws " + TypeName(typeid(Expected)) + "; CNA returned normally");
    }

    /**
     * @brief Runs @p call on input XNA may accept or refuse (a corrupted file, say).
     *
     * Returns true when the call succeeded. A System::Exception is an acceptable refusal; an
     * exception outside the System hierarchy is a WrongRefusal, because a ported game's
     * `catch (InvalidOperationException)` cannot catch it.
     */
    template <typename Call>
    bool Survive(Findings& findings, const std::string& action, const std::string& what, Call&& call);
}

#include "System/Exception.hpp"

namespace CnaKiller
{
    template <typename Call>
    bool Survive(Findings& findings, const std::string& action, const std::string& what, Call&& call)
    {
        try
        {
            call();
            return true;
        }
        catch (const StrictStop&)
        {
            throw;
        }
        catch (const System::Exception&)
        {
            findings.CountRefusal();
            return false;
        }
        catch (const std::exception& exception)
        {
            findings.Report(FindingKind::WrongRefusal, action, what,
                            "refused with a non-System exception " + ExceptionTypeName(exception) +
                                ": " + exception.what());
            return false;
        }
    }
}
