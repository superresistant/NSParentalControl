#pragma once
#include <switch.h>
#include <list>
#include <string>

namespace alefbet {
    namespace pctrl {
        namespace helpers {

            using UserUid = std::string;
            using UserNickname = std::string;

            typedef struct {
                AccountUid uid;
                UserNickname nickname;

                bool isValid() const {
                    return accountUidIsValid(&uid) && nickname.substr(0, 4) != "ERR#";
                }
            } UserData;

            struct ApplicationData {
                u64 titleId = 0;
                std::string name;
            };

            std::list<UserData> getUsersList();
            std::list<ApplicationData> getApplicationsList();

            UserUid accountUidToString(AccountUid uid);
            AccountUid accountUidFromString(const UserUid& uid);

            std::string getTitleName(u64 titleId);
        }
    }
}