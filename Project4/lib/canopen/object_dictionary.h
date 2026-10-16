#pragma once
#include <stdint.h>
#include <node_app.h>

const uint32_t SDO_OK = 0;
const uint32_t ABORT_READ_ONLY = 0x06010002; // attempt to write a read only object
const uint32_t ABORT_NO_OBJECT = 0x06020000; // object does not exist
const uint32_t ABORT_STATE     = 0x08000022; // not allowed in the present device state

uint32_t od_read(NodeApp &app, uint16_t index, uint32_t &value, uint8_t &size);
uint32_t od_write(NodeApp &app, uint16_t index, uint32_t value);
