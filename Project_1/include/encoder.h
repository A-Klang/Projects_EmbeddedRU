#include <digital_in.h>
#include <digital_out.h>

class Encoder
{
    public:
        Encoder(uint8_t c1_pin, uint8_t c2_pin, uint8_t led_pin);
        void init();
        void sample();
        int position();

    private:
        Digital_in c1;
        Digital_in c2;
        Digital_out led;
        bool last_c1;
        int pos;
};