#include "monitor.h"
#include "logger.h"
#include "helpers.h"
#include "database/settings.h"
#include "database/database.h"
#include "notifications_controller.h"
#include "limit_policy.h"
#include <chrono>
#include <algorithm>
#include <list>
//#include "pctrl_screen.hpp"

using namespace alefbet::pctrl::logger;
using namespace alefbet::pctrl::helpers;
using namespace alefbet::pctrl::database;
using namespace std::chrono_literals;

constexpr std::chrono::seconds PollDelay = 1s;
constexpr std::chrono::minutes AccountingInterval = 1min;
constexpr std::chrono::nanoseconds PollDelayInNanos = std::chrono::duration_cast<std::chrono::nanoseconds>(PollDelay);

namespace alefbet::pctrl::srv {   

    void Monitor::start() {
        if(running_) return;
        running_ = true;
    }

    void Monitor::loop() {        
        logDebug("[Monitor] Starting monitoring loop\n");        
        structs::UserData user;
        u64 pid = 0;

        while(true) {
            if(!running_) {
                //Do nothing
                svcSleepThread(500'000'000); // Wait for 500ms
                continue;
            }

            if(!notified_) {
                logInfo("[Monitoring] Parental control is now enabled.\n");
                NotificationsController::notifyMonitoringStarted();
                notified_ = true;
            }
            
            logDebug("[Monitor] Monitoring loop has started\n");
            auto settings = loadSettings();

            // Verify whether the service is enabled
            if(settings.contains(SETTING_ENABLED)) {
                const auto setting = settings[SETTING_ENABLED];
                if(setting.int_value == 0) {
                    logInfo("[Monitor] Parental Control is now disabled.\n");
                    stop();
                    continue;
                }
            }            

            logDebug("[Monitor] Update usages\n");
            // Query the active application and user
            // and update the database
            pid = getRunningApplicationPid();
            
            if(pid != 0) { 
                const auto titleId = getRunningApplicationTitleId(pid);
                user = getCurrentUser();

                if(user.isValid()) {
                    const auto now = std::chrono::steady_clock::now();
                    const bool applicationChanged = currentTitle_ != titleId || !(currentUser_ == user);
                    if(applicationChanged) {
                        lastNotifiedRemaining_ = -1;
                    }

                    const auto userId = accountUidToString(user.uid);
                    const auto globalLimit = getDailyLimitForUser(userId);
                    const auto titleLimit = getDailyLimitForTitle(userId, titleId);
                    auto globalUsage = getUserUsageTimeForToday(user.uid);
                    auto titleUsage = getUserTitleUsageTimeForToday(user.uid, titleId);
                    auto limits = evaluateLimits(globalLimit, globalUsage, titleLimit, titleUsage);

                    const auto elapsed = applicationChanged
                        ? AccountingInterval
                        : std::chrono::duration_cast<std::chrono::minutes>(now - lastAccountedAt_);
                    if(!limits.isExpired() && elapsed >= AccountingInterval) {
                        const auto minutesToAdd = static_cast<u16>(std::min<s64>(elapsed.count(), UINT16_MAX));
                        const auto& entry = addToHistory(user.uid, titleId, minutesToAdd);
                        if(!entry.isValid()) {
                            logError("[Monitor] The database entry is corrupted\n");
                            continue;
                        }

                        lastAccountedAt_ = now;
                        globalUsage = getUserUsageTimeForToday(user.uid);
                        titleUsage = getUserTitleUsageTimeForToday(user.uid, titleId);
                        limits = evaluateLimits(globalLimit, globalUsage, titleLimit, titleUsage);
                    }

                    currentTitle_ = titleId;
                    currentUser_ = user;

                    if(limits.isLimited()) {
                        logDebug("[Monitor] Remaining time for user %s and title %llu is %i minutes\n", user.nickname.c_str(), titleId, limits.remainingMinutes);

                        if(settings.contains(SETTING_SHOW_REMAINING_TIME)
                            && settings[SETTING_SHOW_REMAINING_TIME].int_value > 0
                            && lastNotifiedRemaining_ != limits.remainingMinutes
                            && shouldSendNotification(limits.remainingMinutes)) {
                            NotificationsController::notifyRemainingTime(limits.remainingMinutes);
                            lastNotifiedRemaining_ = limits.remainingMinutes;
                        }

                        if(limits.isExpired()) {
                            logInfo("[Monitor] Timeout for user %s and title %llu\n", user.nickname.c_str(), titleId);
                            if(limits.scope == LimitScope::Title) {
                                if(!terminateCurrentApplication()) {
                                    service_->showScreenTimeout();
                                }
                            } else {
                                service_->showScreenTimeout();
                            }
                        }
                    }
                } else {
                    logDebug("[Monitor] No user found\n");
                }
            } else {
                logDebug("[Monitor] No title running\n");

                if(currentTitle_ > 0) {
                    logDebug("[Monitor] The game has been closed\n");

                    currentTitle_ = 0;
                    currentUser_.clear();
                    lastNotifiedRemaining_ = -1;
                }
            }

            logDebug("[Monitoring] loop\n");
            svcSleepThread(PollDelayInNanos.count());
        }

        logInfo("[Monitor] Stopped monitoring.\n");
    }

    void Monitor::stop() {
        logInfo("[Monitor] Stopping monitor\n");
        running_ = false;
        notified_ = false;
    }

    bool Monitor::shouldSendNotification(int remainingTimeInMinutes) {
        // Remaining time notitications are sent every 15 minutes and every minute during the last 5 minutes
        return remainingTimeInMinutes % 15 == 0 || remainingTimeInMinutes <= 5;
    }
}