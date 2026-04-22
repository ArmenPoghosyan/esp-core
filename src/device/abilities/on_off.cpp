#include "device/abilities/on_off.h"

void OnOff::turn_on() {
	set_state(true);
}

void OnOff::turn_off() {
	set_state(false);
}

bool OnOff::is_on() const {
	return get_state();
}

void OnOff::toggle_on_off() {
	set_state(!get_state());
}

