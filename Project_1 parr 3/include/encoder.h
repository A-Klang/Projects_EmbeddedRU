#include <digital_in.h>
#include <digital_out.h>

class Encoder
{
    public:
        Encoder(uint8_t c1_pin, uint8_t c2_pin, uint8_t led_pin);
        void init();
        void update(); // called from ISR(PCINT2_vect), not meant to be called directly
        int position();

    private:
        Digital_in c1;
        Digital_in c2;
        Digital_out led;
        bool last_c1;
        volatile int pos;
};

// The single encoder instance. Defined in encoder.cpp (pin wiring lives
// there, next to the interrupt setup it belongs with) so ISR(PCINT2_vect)
// can reach it directly by name, with no runtime pointer indirection.
extern Encoder enc;