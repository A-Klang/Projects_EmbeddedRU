#include "canopen.hpp"

#include <cstring>

namespace canopen {
namespace {

constexpr std::uint16_t kDeviceType = 0x1000;
constexpr std::uint16_t kControllerType = 0x2000;
constexpr std::uint16_t kKp = 0x2001;
constexpr std::uint16_t kTi = 0x2002;
constexpr std::uint16_t kOutputDuty = 0x2010;
constexpr std::uint16_t kVelocityActual = 0x606c;
constexpr std::uint16_t kProfileAcceleration = 0x6083;
constexpr std::uint16_t kProfileDeceleration = 0x6084;
constexpr std::uint16_t kTargetVelocity = 0x60ff;

DictionaryResult abort_result(std::uint32_t code) {
    DictionaryResult result{};
    result.abort_code = code;
    return result;
}

DictionaryResult value_u8(std::uint8_t value) {
    DictionaryResult result{};
    result.ok = true;
    result.size = 1;
    result.data[0] = value;
    return result;
}

DictionaryResult value_u16(std::uint16_t value) {
    DictionaryResult result{};
    result.ok = true;
    result.size = 2;
    encode_u16(value, result.data.data());
    return result;
}

DictionaryResult value_u32(std::uint32_t value) {
    DictionaryResult result{};
    result.ok = true;
    result.size = 4;
    encode_u32(value, result.data.data());
    return result;
}

DictionaryResult value_float(float value) {
    static_assert(sizeof(float) == sizeof(std::uint32_t));
    std::uint32_t bits = 0;
    std::memcpy(&bits, &value, sizeof(bits));
    return value_u32(bits);
}

bool expect_size(std::uint8_t actual, std::uint8_t expected) {
    return actual == expected;
}

}  // namespace

std::uint16_t decode_u16(const std::uint8_t* bytes) {
    return static_cast<std::uint16_t>(bytes[0]) |
           (static_cast<std::uint16_t>(bytes[1]) << 8U);
}

std::uint32_t decode_u32(const std::uint8_t* bytes) {
    return static_cast<std::uint32_t>(bytes[0]) |
           (static_cast<std::uint32_t>(bytes[1]) << 8U) |
           (static_cast<std::uint32_t>(bytes[2]) << 16U) |
           (static_cast<std::uint32_t>(bytes[3]) << 24U);
}

void encode_u16(std::uint16_t value, std::uint8_t* bytes) {
    bytes[0] = static_cast<std::uint8_t>(value);
    bytes[1] = static_cast<std::uint8_t>(value >> 8U);
}

void encode_u32(std::uint32_t value, std::uint8_t* bytes) {
    bytes[0] = static_cast<std::uint8_t>(value);
    bytes[1] = static_cast<std::uint8_t>(value >> 8U);
    bytes[2] = static_cast<std::uint8_t>(value >> 16U);
    bytes[3] = static_cast<std::uint8_t>(value >> 24U);
}

ObjectDictionary::ObjectDictionary(ProcessData& process_data,
                                   const INmtStateMachine& nmt)
    : process_data_(process_data), nmt_(nmt) {}

DictionaryResult ObjectDictionary::read(std::uint16_t index,
                                        std::uint8_t subindex) const {
    if (subindex != 0) {
        return abort_result(kAbortObjectMissing);
    }

    switch (index) {
        case kDeviceType:
            return value_u32(process_data_.device_type);
        case kControllerType:
            return value_u8(process_data_.controller_type);
        case kKp:
            return value_float(process_data_.kp);
        case kTi:
            return value_float(process_data_.ti);
        case kOutputDuty:
            return value_u16(static_cast<std::uint16_t>(
                process_data_.output_duty_tenths_percent));
        case kVelocityActual:
            return value_u32(
                static_cast<std::uint32_t>(process_data_.velocity_actual_rpm));
        case kProfileAcceleration:
            return value_u32(process_data_.profile_acceleration_rpm_s);
        case kProfileDeceleration:
            return value_u32(process_data_.profile_deceleration_rpm_s);
        case kTargetVelocity:
            return value_u32(
                static_cast<std::uint32_t>(process_data_.target_velocity_rpm));
        default:
            return abort_result(kAbortObjectMissing);
    }
}

DictionaryResult ObjectDictionary::write(std::uint16_t index,
                                         std::uint8_t subindex,
                                         const std::uint8_t* data,
                                         std::uint8_t size) {
    if (subindex != 0) {
        return abort_result(kAbortObjectMissing);
    }

    const bool configuration_object =
        index == kControllerType || index == kKp || index == kTi ||
        index == kProfileAcceleration || index == kProfileDeceleration;
    if (configuration_object && !configuration_write_allowed()) {
        return abort_result(kAbortPresentState);
    }

    switch (index) {
        case kControllerType:
            if (!expect_size(size, 1)) {
                return abort_result(kAbortTypeMismatch);
            }
            if (data[0] > 1) {
                return abort_result(kAbortValueRange);
            }
            process_data_.controller_type = data[0];
            break;
        case kKp: {
            if (!expect_size(size, 4)) {
                return abort_result(kAbortTypeMismatch);
            }
            const std::uint32_t bits = decode_u32(data);
            std::memcpy(&process_data_.kp, &bits, sizeof(bits));
            break;
        }
        case kTi: {
            if (!expect_size(size, 4)) {
                return abort_result(kAbortTypeMismatch);
            }
            const std::uint32_t bits = decode_u32(data);
            std::memcpy(&process_data_.ti, &bits, sizeof(bits));
            break;
        }
        case kProfileAcceleration:
            if (!expect_size(size, 4)) {
                return abort_result(kAbortTypeMismatch);
            }
            process_data_.profile_acceleration_rpm_s = decode_u32(data);
            break;
        case kProfileDeceleration:
            if (!expect_size(size, 4)) {
                return abort_result(kAbortTypeMismatch);
            }
            process_data_.profile_deceleration_rpm_s = decode_u32(data);
            break;
        case kTargetVelocity:
            if (!expect_size(size, 4)) {
                return abort_result(kAbortTypeMismatch);
            }
            process_data_.target_velocity_rpm =
                static_cast<std::int32_t>(decode_u32(data));
            break;
        case kDeviceType:
        case kOutputDuty:
        case kVelocityActual:
            return abort_result(kAbortReadOnly);
        default:
            return abort_result(kAbortObjectMissing);
    }

    DictionaryResult result{};
    result.ok = true;
    return result;
}

void ObjectDictionary::reset_application_defaults() {
    process_data_ = ProcessData{};
}

void ObjectDictionary::reset_communication_values() {
    process_data_.target_velocity_rpm = 0;
    process_data_.output_duty_tenths_percent = 0;
}

bool ObjectDictionary::configuration_write_allowed() const {
    return nmt_.state() == NmtState::PreOperational;
}

CanopenNode::CanopenNode(ICan& can,
                         INmtStateMachine& nmt,
                         ObjectDictionary& dictionary,
                         ProcessData& process_data,
                         std::uint16_t tpdo_divider)
    : can_(can),
      nmt_(nmt),
      dictionary_(dictionary),
      process_data_(process_data),
      tpdo_divider_(tpdo_divider == 0 ? 1 : tpdo_divider) {}

void CanopenNode::start() {
    nmt_.start();
    send_bootup();
}

void CanopenNode::process(const CanFrame& frame) {
    switch (frame.id) {
        case kCobNmt:
            process_nmt(frame);
            break;
        case kCobSdoRequest:
            process_sdo(frame);
            break;
        case kCobRpdo1:
            process_rpdo1(frame);
            break;
        default:
            break;
    }
}

void CanopenNode::on_control_update() {
    if (nmt_.state() != NmtState::Operational) {
        control_updates_ = 0;
        return;
    }

    ++control_updates_;
    if (control_updates_ >= tpdo_divider_) {
        control_updates_ = 0;
        send_tpdo1();
    }
}

void CanopenNode::process_nmt(const CanFrame& frame) {
    if (frame.dlc < 2 || (frame.data[1] != 0 && frame.data[1] != kNodeId)) {
        return;
    }

    if (frame.data[0] == static_cast<std::uint8_t>(NmtCommand::ResetNode)) {
        dictionary_.reset_application_defaults();
    } else if (frame.data[0] ==
               static_cast<std::uint8_t>(NmtCommand::ResetCommunication)) {
        dictionary_.reset_communication_values();
    }

    if (nmt_.handle_command(frame.data[0])) {
        control_updates_ = 0;
        send_bootup();
    }
}

void CanopenNode::process_sdo(const CanFrame& frame) {
    if (frame.dlc != 8 ||
        (nmt_.state() != NmtState::PreOperational &&
         nmt_.state() != NmtState::Operational)) {
        return;
    }

    const std::uint8_t command = frame.data[0];
    const std::uint16_t index = decode_u16(&frame.data[1]);
    const std::uint8_t subindex = frame.data[3];

    if (command == 0x40) {
        const DictionaryResult result = dictionary_.read(index, subindex);
        if (result.ok) {
            send_sdo_upload(index, subindex, result);
        } else {
            send_sdo_abort(index, subindex, result.abort_code);
        }
        return;
    }

    const bool download = (command & 0xe0U) == 0x20U;
    const bool expedited = (command & 0x02U) != 0;
    const bool size_indicated = (command & 0x01U) != 0;
    if (!download || !expedited || !size_indicated) {
        send_sdo_abort(index, subindex, kAbortCommand);
        return;
    }

    const std::uint8_t unused = (command >> 2U) & 0x03U;
    const std::uint8_t size = 4U - unused;
    const DictionaryResult result =
        dictionary_.write(index, subindex, &frame.data[4], size);
    if (result.ok) {
        send_sdo_download_ack(index, subindex);
    } else {
        send_sdo_abort(index, subindex, result.abort_code);
    }
}

void CanopenNode::process_rpdo1(const CanFrame& frame) {
    if (nmt_.state() != NmtState::Operational || frame.dlc < 4) {
        return;
    }
    process_data_.target_velocity_rpm =
        static_cast<std::int32_t>(decode_u32(frame.data.data()));
}

void CanopenNode::send_bootup() {
    CanFrame frame{};
    frame.id = kCobBootup;
    frame.dlc = 1;
    frame.data[0] = 0;
    can_.send(frame);
}

void CanopenNode::send_tpdo1() {
    CanFrame frame{};
    frame.id = kCobTpdo1;
    frame.dlc = 6;
    encode_u32(static_cast<std::uint32_t>(process_data_.velocity_actual_rpm),
               frame.data.data());
    encode_u16(
        static_cast<std::uint16_t>(process_data_.output_duty_tenths_percent),
        &frame.data[4]);
    can_.send(frame);
}

void CanopenNode::send_sdo_upload(std::uint16_t index,
                                  std::uint8_t subindex,
                                  const DictionaryResult& result) {
    CanFrame frame{};
    frame.id = kCobSdoResponse;
    frame.dlc = 8;
    const std::uint8_t unused = 4U - result.size;
    frame.data[0] = static_cast<std::uint8_t>(0x43U | (unused << 2U));
    encode_u16(index, &frame.data[1]);
    frame.data[3] = subindex;
    for (std::uint8_t i = 0; i < result.size; ++i) {
        frame.data[4 + i] = result.data[i];
    }
    can_.send(frame);
}

void CanopenNode::send_sdo_download_ack(std::uint16_t index,
                                        std::uint8_t subindex) {
    CanFrame frame{};
    frame.id = kCobSdoResponse;
    frame.dlc = 8;
    frame.data[0] = 0x60;
    encode_u16(index, &frame.data[1]);
    frame.data[3] = subindex;
    can_.send(frame);
}

void CanopenNode::send_sdo_abort(std::uint16_t index,
                                 std::uint8_t subindex,
                                 std::uint32_t abort_code) {
    CanFrame frame{};
    frame.id = kCobSdoResponse;
    frame.dlc = 8;
    frame.data[0] = 0x80;
    encode_u16(index, &frame.data[1]);
    frame.data[3] = subindex;
    encode_u32(abort_code, &frame.data[4]);
    can_.send(frame);
}

}  // namespace canopen
