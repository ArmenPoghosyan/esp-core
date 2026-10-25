#pragma once

// This device's certificate and private key (PEM, null-terminated), embedded from
// crt/device.crt and crt/device.key by board_build.embed_txtfiles in platformio.ini.
extern const char device_crt[] asm("_binary_crt_device_crt_start");
extern const char device_key[] asm("_binary_crt_device_key_start");
