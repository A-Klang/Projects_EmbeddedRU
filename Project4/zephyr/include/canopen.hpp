#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace canopen {

constexpr std::uint8_t kNodeId = 1;
constexpr std::uint32_t kCobNmt = 0x000;
constexpr std::uint32_t kCobTpdo1 = 0x180 + kNodeId;
constexpr std::uint32_t kCobRpdo1 = 0x200 + kNodeId;
constexpr std::uint32_t kCobSdoResponse = 0x580 + kNodeId;
constexpr std::uint32_t kCobSdoRequest = 0x600 + kNodeId;
constexpr std::uint32_t kCobBootup = 0x700 + kNodeId;

constexpr std::uint32_t kAbortCommand = 0x05040001;
constexpr std::uint32_t kAbortReadOnly = 0x06010002;
constexpr std::uint32_t kAbortObjectMissing = 0x06020000;
constexpr std::uint32_t kAbortTypeMismatch = 0x06070010;
constexpr std::uint32_t kAbortValueRange = 0x06090030;
constexpr std::uint32_t kAbortPresentState = 0x08000022;

struct CanFrame {
    std::uint32_t id = 0;
    std::uint8_t dlc = 0;
    std::array<std::uint8_t, 8> data{};
};

class ICan {
public:
    virtual ~ICan() = default;
    virtual bool send(const CanFrame& frame) = 0;
};

enum class NmtState : std::uint8_t {
    Initializing = 0x00,
    Stopped = 0x04,
    Operational = 0x05,
    PreOperational = 0x7f,
};

enum class NmtCommand : std::uint8_t {
    Start = 0x01,
    Stop = 0x02,
    EnterPreOperational = 0x80,
    ResetNode = 0x81,
    ResetCommunication = 0x82,
};

class INmtStateMachine {
public:
    virtual ~INmtStateMachine() = default;
    virtual void start() = 0;
    virtual bool handle_command(std::uint8_t command) = 0;
    virtual void fault() = 0;
    virtual NmtState state() const = 0;
};

struct ProcessData {
    std::uint32_t device_type = 0;
    std::uint8_t controller_type = 1;
    float kp = 40.0F;
    float ti = 0.1F;
    std::int16_t output_duty_tenths_percent = 0;
    std::int32_t velocity_actual_rpm = 0;
    std::uint32_t profile_acceleration_rpm_s = 0;
    std::uint32_t profile_deceleration_rpm_s = 0;
    std::int32_t target_velocity_rpm = 0;
};

struct DictionaryResult {
    bool ok = false;
    std::uint32_t abort_code = 0;
    std::uint8_t size = 0;
    std::array<std::uint8_t, 4> data{};
};

class ObjectDictionary {
public:
    ObjectDictionary(ProcessData& process_data, const INmtStateMachine& nmt);

    DictionaryResult read(std::uint16_t index, std::uint8_t subindex) const;
    DictionaryResult write(std::uint16_t index,
                           std::uint8_t subindex,
                           const std::uint8_t* data,
                           std::uint8_t size);
    void reset_application_defaults();
    void reset_communication_values();

private:
    bool configuration_write_allowed() const;

    ProcessData& process_data_;
    const INmtStateMachine& nmt_;
};

class CanopenNode {
public:
    CanopenNode(ICan& can,
                INmtStateMachine& nmt,
                ObjectDictionary& dictionary,
                ProcessData& process_data,
                std::uint16_t tpdo_divider = 20);

    void start();
    void process(const CanFrame& frame);
    void on_control_update();

private:
    void process_nmt(const CanFrame& frame);
    void process_sdo(const CanFrame& frame);
    void process_rpdo1(const CanFrame& frame);
    void send_bootup();
    void send_tpdo1();
    void send_sdo_upload(std::uint16_t index,
                         std::uint8_t subindex,
                         const DictionaryResult& result);
    void send_sdo_download_ack(std::uint16_t index, std::uint8_t subindex);
    void send_sdo_abort(std::uint16_t index,
                        std::uint8_t subindex,
                        std::uint32_t abort_code);

    ICan& can_;
    INmtStateMachine& nmt_;
    ObjectDictionary& dictionary_;
    ProcessData& process_data_;
    std::uint16_t tpdo_divider_;
    std::uint16_t control_updates_ = 0;
};

std::uint16_t decode_u16(const std::uint8_t* bytes);
std::uint32_t decode_u32(const std::uint8_t* bytes);
void encode_u16(std::uint16_t value, std::uint8_t* bytes);
void encode_u32(std::uint32_t value, std::uint8_t* bytes);

}  // namespace canopen
