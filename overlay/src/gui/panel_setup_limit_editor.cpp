#include "panel_setup_limit_editor.h"
#include "helpers/ipc_helpers.h"
#include <chrono>

using namespace alefbet::pctrl;
using namespace alefbet::pctrl::helpers;
using namespace std::chrono;

constexpr tsl::Color EditorColorSelected = tsl::style::color::ColorHighlight;
constexpr tsl::Color EditorColorWhite = tsl::Color(0xffff);

SetupLimitEditorPanel::SetupLimitEditorPanel(const UserData& user, const ApplicationData& application)
: user_(user), application_(application) {
}

tsl::elm::Element* SetupLimitEditorPanel::createUI() {
    const std::string title = application_.titleId == 0 ? "All games" : application_.name;
    rootFrame_ = new tsl::elm::OverlayFrame("Parental Control", title);
    rootList_ = new tsl::elm::List();

    const auto limit = application_.titleId == 0
        ? ipc::getDailyLimit(user_)
        : ipc::getTitleDailyLimit(user_, application_.titleId);
    const auto duration = std::chrono::minutes{limit};
    const auto hoursPart = duration_cast<hours>(duration);
    const auto minutesPart = duration_cast<minutes>(duration - hoursPart);
    hours_ = hoursPart.count();
    minutes_ = minutesPart.count();

    rootList_->addItem(new tsl::elm::CategoryHeader(
        application_.titleId == 0 ? "Daily limit (0 = unlimited)" : "Daily limit (0 = use all-games)"));
    rootList_->addItem(new tsl::elm::CustomDrawer([this](tsl::gfx::Renderer* renderer, s32 x, s32 y, s32 w, s32 h) {
        const s32 yPos = y + 35;
        renderer->drawString("Limit", false, x, yPos, 20, renderer->a(EditorColorWhite));
        renderer->drawString("mn", false, w-40, yPos, 20, renderer->a(!hoursSelected_ ? EditorColorSelected : EditorColorWhite));
        std::string value = std::to_string(minutes_);
        renderer->drawString(value.c_str(), false, w-60 -(minutes_ > 9 ? 10 : 0), yPos, 20, renderer->a(!hoursSelected_ ? EditorColorSelected : EditorColorWhite));
        renderer->drawString("h", false, w-90, yPos, 20, renderer->a(hoursSelected_ ? EditorColorSelected : EditorColorWhite));
        value = std::to_string(hours_);
        renderer->drawString(value.c_str(), false, w-110 -(hours_ > 9 ? 10 : 0), yPos, 20, renderer->a(hoursSelected_ ? EditorColorSelected : EditorColorWhite));
    }), 60);
    rootList_->addItem(new tsl::elm::CategoryHeader("Left/right: select   Up/down: change"));
    rootFrame_->setContent(rootList_);
    return rootFrame_;
}

void SetupLimitEditorPanel::update() {
}

bool SetupLimitEditorPanel::handleInput(u64 keysDown, u64 keysHeld, const HidTouchState& touchPos, HidAnalogStickState joyStickPosLeft, HidAnalogStickState joyStickPosRight) {
    if(keysDown & HidNpadButton_B) {
        const u16 limit = hours_ * 60 + minutes_;
        if(application_.titleId == 0) {
            ipc::setDailyLimit(user_, limit);
        } else {
            ipc::setTitleDailyLimit(user_, application_.titleId, limit);
        }
        tsl::goBack();
        return true;
    }
    if(keysDown & (HidNpadButton_AnyLeft | HidNpadButton_AnyRight)) {
        hoursSelected_ = !hoursSelected_;
        return true;
    }
    if(keysDown & HidNpadButton_AnyUp) {
        if(hoursSelected_) hours_ = valueRanged(hours_, 1, 0, 12);
        else minutes_ = valueRanged(minutes_, 1, 0, 59);
        return true;
    }
    if(keysDown & HidNpadButton_AnyDown) {
        if(hoursSelected_) hours_ = valueRanged(hours_, -1, 0, 12);
        else minutes_ = valueRanged(minutes_, -1, 0, 59);
        return true;
    }
    return false;
}

u16 SetupLimitEditorPanel::valueRanged(int value, int diff, int min, int max) {
    if(value + diff < min) return max;
    if(value + diff > max) return min;
    return value + diff;
}
