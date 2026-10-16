#include <object_dictionary.h>
#include <string.h>
#include <math.h>

static uint32_t profile_acc = 0; // 0x6083, not used until Part 3
static uint32_t profile_dec = 0; // 0x6084

static uint32_t from_float(float f) { uint32_t u; memcpy(&u, &f, 4); return u; }
static float to_float(uint32_t u) { float f; memcpy(&f, &u, 4); return f; }

uint32_t od_read(NodeApp &app, uint16_t index, uint32_t &value, uint8_t &size) {
    size = 4;
    switch (index) {
        case 0x1000: value = 0x00000192; break; // device type, CiA 402 drive profile
        case 0x2000: value = app.controller_type(); size = 1; break;
        case 0x2001: value = from_float(app.kp()); break;
        case 0x2002: value = from_float(app.ti()); break;
        case 0x2010: value = (uint16_t)lroundf(app.duty_cycle() * 1000); size = 2; break; // 0.1 %
        case 0x606C: value = (uint32_t)lroundf(app.velocity_actual()); break;
        case 0x6083: value = profile_acc; break;
        case 0x6084: value = profile_dec; break;
        case 0x60FF: value = (uint32_t)app.target_velocity(); break;
        default: return ABORT_NO_OBJECT;
    }
    return SDO_OK;
}

uint32_t od_write(NodeApp &app, uint16_t index, uint32_t value) {
    switch (index) {
        case 0x1000: case 0x2010: case 0x606C:
            return ABORT_READ_ONLY;
        case 0x60FF: // target velocity, writable in every state
            app.set_target_velocity((int32_t)value);
            return SDO_OK;
        case 0x2000: case 0x2001: case 0x2002: case 0x6083: case 0x6084:
            break;
        default:
            return ABORT_NO_OBJECT;
    }

    // Configuration objects, only writable in Pre-operational
    if (app.nmt_state() != NmtState::PreOperational) return ABORT_STATE;

    switch (index) {
        case 0x2000: app.set_controller_type(value); break;
        case 0x2001: app.set_kp(to_float(value)); break;
        case 0x2002: app.set_ti(to_float(value)); break;
        case 0x6083: profile_acc = value; break;
        case 0x6084: profile_dec = value; break;
    }
    return SDO_OK;
}
