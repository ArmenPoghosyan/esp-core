#include "device/device.h"

IDevice::IDevice(DeviceType type, const char* name) : type(type), name(name) {}

DeviceType IDevice::get_type() {
	return type;
}

const char* IDevice::get_name() {
	return name;
}

