#include "../source/limit_policy.h"
#include <cassert>

using alefbet::pctrl::LimitScope;
using alefbet::pctrl::evaluateLimits;

int main() {
    auto result = evaluateLimits(0, 0, 0, 0);
    assert(!result.isLimited());
    assert(!result.isExpired());

    result = evaluateLimits(120, 30, 0, 0);
    assert(result.scope == LimitScope::Global);
    assert(result.remainingMinutes == 90);

    result = evaluateLimits(0, 300, 60, 25);
    assert(result.scope == LimitScope::Title);
    assert(result.remainingMinutes == 35);

    result = evaluateLimits(0, 0, 60, 60);
    assert(result.scope == LimitScope::Title);
    assert(result.isExpired());

    result = evaluateLimits(120, 110, 60, 20);
    assert(result.scope == LimitScope::Global);
    assert(result.remainingMinutes == 10);

    result = evaluateLimits(120, 30, 60, 55);
    assert(result.scope == LimitScope::Title);
    assert(result.remainingMinutes == 5);

    result = evaluateLimits(60, 60, 30, 30);
    assert(result.scope == LimitScope::Global);
    assert(result.isExpired());

    result = evaluateLimits(30, 65535, 0, 0);
    assert(result.scope == LimitScope::Global);
    assert(result.isExpired());
}
