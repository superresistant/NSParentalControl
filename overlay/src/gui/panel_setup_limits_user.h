#pragma once

#include "switch_helpers.h"
#include <tesla.hpp>

using namespace alefbet::pctrl::helpers;

class SetupLimitsUserPanel : public tsl::Gui {
public:
    SetupLimitsUserPanel(const UserData& user);

    tsl::elm::Element* createUI() override;
    void update() override;
    bool handleInput(u64 keysDown, u64 keysHeld, const HidTouchState& touchPos, HidAnalogStickState joyStickPosLeft, HidAnalogStickState joyStickPosRight) override;

private:
    UserData user_;
    tsl::elm::OverlayFrame* rootFrame_ = nullptr;
    tsl::elm::List* rootList_ = nullptr;
};
