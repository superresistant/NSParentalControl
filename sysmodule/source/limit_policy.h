#pragma once

#include <algorithm>
#include <cstdint>

namespace alefbet::pctrl {

enum class LimitScope {
    None,
    Global,
    Title
};

struct LimitEvaluation {
    LimitScope scope = LimitScope::None;
    std::uint16_t remainingMinutes = 0;

    bool isLimited() const {
        return scope != LimitScope::None;
    }

    bool isExpired() const {
        return isLimited() && remainingMinutes == 0;
    }
};

constexpr std::uint16_t remainingMinutes(std::uint16_t limit, std::uint16_t usage) {
    return usage < limit ? limit - usage : 0;
}

constexpr LimitEvaluation evaluateLimits(
    std::uint16_t globalLimit,
    std::uint16_t globalUsage,
    std::uint16_t titleLimit,
    std::uint16_t titleUsage) {
    const bool hasGlobalLimit = globalLimit > 0;
    const bool hasTitleLimit = titleLimit > 0;

    if(!hasGlobalLimit && !hasTitleLimit) {
        return {};
    }

    if(!hasTitleLimit) {
        return {LimitScope::Global, remainingMinutes(globalLimit, globalUsage)};
    }

    if(!hasGlobalLimit) {
        return {LimitScope::Title, remainingMinutes(titleLimit, titleUsage)};
    }

    const auto globalRemaining = remainingMinutes(globalLimit, globalUsage);
    const auto titleRemaining = remainingMinutes(titleLimit, titleUsage);
    return globalRemaining <= titleRemaining
        ? LimitEvaluation{LimitScope::Global, globalRemaining}
        : LimitEvaluation{LimitScope::Title, titleRemaining};
}

}
