#include "switch_helpers.h"
#include "logger.h"
#include <string_view>
#include <string_view>
#include <ranges>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <sstream>

using namespace alefbet::pctrl::logger;

namespace alefbet {
    namespace pctrl {
        namespace helpers {        

            using std::operator""sv;

            std::list<UserData> getUsersList() {
                std::list<UserData> users;

                Result rc = accountInitialize(AccountServiceType_System);
                if(R_FAILED(rc)) {
                    logError("[Service] Could not connect to service account (%i:%i)\n", R_MODULE(rc), R_DESCRIPTION(rc));
                    return std::list<UserData>{};
                }

                AccountUid _users[ACC_USER_LIST_SIZE];
                s32 count = 0;

                rc = accountListAllUsers(_users, ACC_USER_LIST_SIZE, &count);
                if(R_FAILED(rc)) {
                    logError("[Service] Could not enumerate users (%i:%i)\n", R_MODULE(rc), R_DESCRIPTION(rc));
                    accountExit();
                    return std::list<UserData>{};
                }

                AccountProfile profile;
                AccountUserData user_data;
                AccountProfileBase base;

                for(int i = 0 ; i < count ; i++) {
                    const auto& uid = _users[i];
                    rc = accountGetProfile(&profile, uid); 
                    if(rc != 0) {
                        logError("[Helpers] Could not get account profile (%i:%i)\n", R_MODULE(rc), R_DESCRIPTION(rc));
                        continue;
                    } else {
                        logDebug("[Helpers] accountGetProfile() ok\n");
                    }

                    rc = accountProfileGet(&profile, &user_data, &base);
                    if(rc != 0) {
                        logError("[Helpers] Could not get user data (%i:%i)\n", R_MODULE(rc), R_DESCRIPTION(rc));
                        accountProfileClose(&profile);
                        continue;
                    } else {                        
                        logDebug("[Helpers] accountProfileGet() ok\n");
                    }

                    accountProfileClose(&profile);

                    UserData user;
                    user.uid = uid;
                    user.nickname = base.nickname;

                    users.push_back(user);
                }
                
                accountExit();
                return users;
            }

            std::list<ApplicationData> getApplicationsList() {
                std::list<ApplicationData> applications;
                if(R_FAILED(nsInitialize())) {
                    logError("[Helpers] Could not initialize NS service\n");
                    return applications;
                }

                constexpr s32 PageSize = 32;
                NsApplicationRecord records[PageSize];
                s32 offset = 0;
                s32 count = 0;
                do {
                    if(R_FAILED(nsListApplicationRecord(records, PageSize, offset, &count))) {
                        logError("[Helpers] Could not enumerate applications\n");
                        break;
                    }

                    for(s32 index = 0; index < count; index++) {
                        const auto titleId = records[index].application_id;
                        NsApplicationControlData control{};
                        NacpLanguageEntry* language = nullptr;
                        u64 actualSize = 0;
                        std::string name;
                        if(R_SUCCEEDED(nsGetApplicationControlData(
                            NsApplicationControlSource_Storage,
                            titleId,
                            &control,
                            sizeof(control),
                            &actualSize))
                            && R_SUCCEEDED(nacpGetLanguageEntry(&control.nacp, &language))
                            && language != nullptr) {
                            name = language->name;
                        }

                        if(name.empty()) {
                            std::ostringstream stream;
                            stream << std::uppercase << std::hex << std::setw(16) << std::setfill('0') << titleId;
                            name = stream.str();
                        }
                        applications.push_back({titleId, name});
                    }
                    offset += count;
                } while(count == PageSize);

                nsExit();
                applications.sort([](const auto& left, const auto& right) {
                    return left.name < right.name;
                });
                return applications;
            }

            UserUid accountUidToString(AccountUid uid) {
                return std::to_string(uid.uid[0]) + ":" + std::to_string(uid.uid[1]);
            }

            AccountUid accountUidFromString(const UserUid& uid_str) {
                AccountUid uid;

                std::vector<std::string> parts;
                for (const auto part : std::views::split(uid_str, ":"sv)) {
                    parts.push_back(std::string(part.begin(), part.end()));
                }
                
                if(parts.size() != 2) {
                    logError("[Helpers] Incorrect split of AccountUid : %s\n", uid_str.c_str());
                    return uid;
                }

                uid.uid[0] = std::stoull(parts[0]);
                uid.uid[1] = std::stoull(parts[1]);

                return uid;
            }

            std::string getTitleName(u64 titleId) {
                if(R_FAILED(nsInitialize())) {
                    logError("[Helpers] Could not initialize NS service\n");
                    return "Error #11";
                }

                NsApplicationControlData control{};
                NacpLanguageEntry* language = nullptr;
                u64 actualSize = 0;
                if(R_FAILED(nsGetApplicationControlData(
                    NsApplicationControlSource_Storage,
                    titleId,
                    &control,
                    sizeof(control),
                    &actualSize))) {
                    logError("[Helpers] Could not get application information for %llu\n", titleId);
                    nsExit();
                    return "Error #13";
                }

                const auto result = nacpGetLanguageEntry(&control.nacp, &language);
                const std::string name = R_SUCCEEDED(result) && language != nullptr ? language->name : "Error #14";
                nsExit();
                return name;
            }
        }
    }
}