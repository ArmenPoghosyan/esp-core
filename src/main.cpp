#include "includes.h"

#include "device/type/light.h"
#include "device/ability/on_off.h"
#include "device/ability/brightness.h"

#include "led/led.h"

LED led(21);
Light<OnOff, Brightness> light(DEVICE_NAME);

void setup()
{
	init_system();
	light.attach_to_led(led);
	light.turn_on();
}

void loop()
{
	loop_system();

	light.set_brightness(random(0, 256));

	delay(100);
}
