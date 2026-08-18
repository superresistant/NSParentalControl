#pragma once

#include "switch_helpers.h"
#include <tesla.hpp>

class SetupLimitEditorPanel : public tsl::Gui {
public:
    SetupLimitEditorPanel(const alefbet::pctrl::helpers::UserData& user, const alefbet::pctrl::helpers::ApplicationData& application);

    tsl::elm::Element* createUI() override;
    void update() override;
    bool handleInput(u64 keysDown, u64 keysHeld, const HidTouchState& touchPos, HidAnalogStickState joyStickPosLeft, HidAnalogStickState joyStickPosRight) override;

private:
    u16 valueRanged(int value, int diff, int min, int max);

    alefbet::pctrl::helpers::UserData user_;
    alefbet::pctrl::helpers::ApplicationData application_;
    tsl::elm::OverlayFrame* rootFrame_ = nullptr;
    tsl::elm::List* rootList_ = nullptr;
    bool hoursSelected_ = true;
    u16 hours_ = 0;
    u16 minutes_ = 0;
};
