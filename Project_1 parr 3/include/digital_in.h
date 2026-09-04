class Digital_in
{
    public:
        Digital_in(uint8_t pin);
        void init();
        bool is_hi();

    private:
        uint8_t pinMask;
};
