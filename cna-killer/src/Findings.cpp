// SPDX-License-Identifier: MIT
#include "Findings.hpp"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <vector>

#if defined(__GNUG__)
#include <cxxabi.h>
#endif

namespace CnaKiller
{
    const char* ToString(FindingKind kind)
    {
        switch (kind)
        {
            case FindingKind::UnexpectedException: return "UNEXPECTED-EXCEPTION";
            case FindingKind::Mismatch:            return "MISMATCH";
            case FindingKind::MissingRefusal:      return "MISSING-REFUSAL";
            case FindingKind::WrongRefusal:        return "WRONG-REFUSAL";
            case FindingKind::Leak:                return "LEAK";
        }
        return "UNKNOWN";
    }

    std::string TypeName(const std::type_info& type)
    {
#if defined(__GNUG__)
        int status = 0;
        const std::unique_ptr<char, void (*)(void*)> demangled(
            abi::__cxa_demangle(type.name(), nullptr, nullptr, &status), std::free);
        if (status == 0 && demangled)
            return demangled.get();
#endif
        return type.name();
    }

    std::string ExceptionTypeName(const std::exception& exception)
    {
        return TypeName(typeid(exception));
    }

    Findings::Findings(ChaosLog& log, const bool strict)
        : log_(log)
        , strict_(strict)
    {
    }

    void Findings::Report(const FindingKind kind, const std::string& action, const std::string& what,
                          const std::string& detail)
    {
        ++total_;
        const std::string key = std::string(ToString(kind)) + '|' + action + '|' + what;
        Entry& entry = entries_[key];
        if (entry.count == 0)
        {
            entry.kind = kind;
            entry.action = action;
            entry.what = what;
            entry.firstDetail = detail;
            entry.firstTick = tick_;
        }
        ++entry.count;

        if (entry.count <= kLoggedOccurrences)
        {
            log_.Note("FINDING " + std::string(ToString(kind)) + " tick=" + std::to_string(tick_) +
                      " action=" + action + " :: " + what + " :: " + detail);
        }
        else if (entry.count == kLoggedOccurrences + 1)
        {
            log_.Note("FINDING " + std::string(ToString(kind)) + " action=" + action + " :: " + what +
                      " :: further occurrences are only counted");
        }

        if (strict_)
            throw StrictStop();
    }

    void Findings::WriteSummary()
    {
        std::vector<const Entry*> ordered;
        ordered.reserve(entries_.size());
        for (const auto& [key, entry] : entries_)
            ordered.push_back(&entry);
        std::stable_sort(ordered.begin(), ordered.end(), [](const Entry* a, const Entry* b) {
            return static_cast<int>(a->kind) < static_cast<int>(b->kind);
        });

        const std::string header = "summary: " + std::to_string(entries_.size()) +
            " distinct findings (" + std::to_string(total_) + " occurrences), " +
            std::to_string(checks_) + " verifications passed, " + std::to_string(refusals_) +
            " XNA refusals reproduced";
        log_.Note(header);
        std::cout << "cna-killer: " << header << "\n";
        for (const Entry* entry : ordered)
        {
            const std::string line = std::string(ToString(entry->kind)) + " x" +
                std::to_string(entry->count) + " action=" + entry->action + " first-tick=" +
                std::to_string(entry->firstTick) + " :: " + entry->what + " :: " + entry->firstDetail;
            log_.Note("summary " + line);
            std::cout << "  " << line << "\n";
        }
        std::cout.flush();
    }
}
