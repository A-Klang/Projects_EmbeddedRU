class Digital_in
{
    public:
        Digital_in(uint8_t pin);
        void init();
        int position();

    private:
        uint8_t pinMask;
};