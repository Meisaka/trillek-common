#include "systems/vcomputer-system.hpp"

#include "trillek-game.hpp"
#include "components/component.hpp"
#include "components/system-component.hpp"
#include "hardware/cpu.hpp"
#include "trillek.hpp"
#include "logging.hpp"

#include "vcomputer.hpp"
#include "tr3200/tr3200.hpp"
#include "auxiliar.hpp"
#include "devices/gkeyb.hpp"
#include "devices/tda.hpp"

namespace trillek {

using namespace component;

VComputerSystem::VComputerSystem() : system(game.GetSystemComponent()) {
    event::Dispatcher<KeyboardEvent>::GetInstance()->Subscribe(this);
    event::EventQueue<HardwareAction>::Subscribe(this);
    event::EventQueue<InteractEvent>::Subscribe(this);
};

VComputerSystem::~VComputerSystem() { }

void VComputerSystem::HandleEvents(frame_tp timepoint) {
    static frame_tp last_tp;
    this->delta = frame_unit(timepoint - last_tp);
    last_tp = timepoint;
    auto count = this->delta.count() * 0.000000001;
    event::EventQueue<HardwareAction>::ProcessEvents(this);
    event::EventQueue<InteractEvent>::ProcessEvents(this);
    OnTrue(Bitmap<Component::VComputer>(),
        [&](id_t entity_id) {
            auto& vcom = Get<Component::VComputer>(entity_id);
            vcom.vc->Update(count);
        });
    OnTrue(Bitmap<Component::VDisplay>(),
        [&](id_t entity_id) {
            auto& disp = system.Get<Component::VDisplay>(entity_id);
            disp.ScreenUpdate();
        });
}

void VComputerSystem::OnEvent(const HardwareAction& event) {
    switch(event.cid) {
    case Component::VDisplay:
        if(Has<Component::VDisplay>(event.entity_id)) {
            auto& disp = system.Get<Component::VDisplay>(event.entity_id);
            disp.LinkDevice();
        }
        break;
    case Component::VKeyboard:
        if(Has<Component::VKeyboard>(event.entity_id)) {
            auto& keyb = system.Get<Component::VKeyboard>(event.entity_id);
            keyb.LinkDevice();
        }
        break;
    }
}

void VComputerSystem::OnEvent(const InteractEvent& event) {
    LOGMSG(INFO) << "VCom: got interact event " << event.entity << ", " << ActionText::Get(event.act) << ", " << event.num;
    switch(event.act) {
    case Action::IA_POWER:
        if(event.num == (uint32_t)Component::VDisplay && Has<Component::VDisplay>(event.entity)) {
            game.GetSystemComponent().Get<Component::VDisplay>(event.entity).PowerToggle();
            return;
        }
        else if(event.num == (uint32_t)Component::VComputer && Has<Component::VComputer>(event.entity)) {
            game.GetSystemComponent().Get<Component::VComputer>(event.entity).PowerToggle();
            return;
        }
        break;
    case Action::IA_USE:
        if(event.num == (uint32_t)Component::VKeyboard && Has<Component::VKeyboard>(event.entity)) {
            auto& cm = game.GetSystemComponent().Get<Component::VKeyboard>(event.entity);
            if(cm.IsActive()) {
                cm.SetActive(false);
            }
            else {
                cm.SetActive(true);
            }
            return;
        }
        break;
    }
}

void VComputerSystem::Notify(const KeyboardEvent* key_event) {

}

} // namespace trillek
