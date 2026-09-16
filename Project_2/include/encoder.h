#include <digital_in.h>
#include <digital_out.h>

class Encoder
{
    public:
        Encoder(uint8_t c1_pin, uint8_t c2_pin, uint8_t led_pin, uint8_t AIN1_port, uint8_t AIN2_port);
        void init();
        void update(); // called from ISR(PCINT2_vect), not meant to be called directly
        void sample_speed();
        int get_position();
        float get_speed();
        int counter = 0;


    private:
        Digital_in c1;
        Digital_in c2;
        Digital_out led;
        Digital_out AIN1;
        Digital_out AIN2;
        bool last_c1;
        volatile int pos;
        volatile int last_sample_pos;
        volatile float speed_rpm;

        static constexpr float period_ms = 1;
        static constexpr float counts_per_rev = 2100.0;
};

// The single encoder instance. Defined in encoder.cpp (pin wiring lives
// there, next to the interrupt setup it belongs with) so ISR(PCINT2_vect)
// can reach it directly by name, with no runtime pointer indirection.
extern Encoder enc;