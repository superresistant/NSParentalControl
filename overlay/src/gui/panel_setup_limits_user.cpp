#include "panel_setup_limits_user.h"
#include "panel_setup_limit_editor.h"
using namespace alefbet::pctrl::helpers;

SetupLimitsUserPanel::SetupLimitsUserPanel(const UserData& user)
: user_(user) {
}

tsl::elm::Element* SetupLimitsUserPanel::createUI() {
    rootFrame_ = new tsl::elm::OverlayFrame("Parental Control", "Limits for " + user_.nickname);
    rootList_ = new tsl::elm::List();

    rootList_->addItem(new tsl::elm::CategoryHeader("Default"));
    ApplicationData allGames{0, "All games"};
    auto allGamesEntry = new tsl::elm::ListItem(allGames.name, "Configure");
    rootList_->addItem(allGamesEntry);
    allGamesEntry->setClickListener([user = user_, allGames](u64 keys) {
        if(keys & HidNpadButton_A) {
            tsl::changeTo<SetupLimitEditorPanel>(user, allGames);
            return true;
        }
        return false;
    });

    rootList_->addItem(new tsl::elm::CategoryHeader("Per-game limits"));
    for(const auto& application: getApplicationsList()) {
        auto entry = new tsl::elm::ListItem(application.name, "Configure");
        rootList_->addItem(entry);
        entry->setClickListener([user = user_, application](u64 keys) {
            if(keys & HidNpadButton_A) {
                tsl::changeTo<SetupLimitEditorPanel>(user, application);
                return true;
            }
            return false;
        });
    }

    rootFrame_->setContent(rootList_);
    return rootFrame_;
}

void SetupLimitsUserPanel::update() {
}

bool SetupLimitsUserPanel::handleInput(u64 keysDown, u64 keysHeld, const HidTouchState& touchPos, HidAnalogStickState joyStickPosLeft, HidAnalogStickState joyStickPosRight) {
    if(keysDown & HidNpadButton_B) {
        tsl::goBack();
        return true;
    }
    return false;
}
