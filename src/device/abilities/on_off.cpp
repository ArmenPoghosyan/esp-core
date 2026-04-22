#include "device/abilities/on_off.h"

void OnOff::turn_on() {
	state = true;
}

void OnOff::turn_off() {
	state = false;
}

bool OnOff::is_on() const {
	return state;
}

void OnOff::toggle_on_off() {
	state = !state;
}

