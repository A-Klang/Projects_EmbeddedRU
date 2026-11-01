while true; do
    x=$(gpioget -c 0 --numeric 23)
    gpioset -c 0 -t0 21=$x
done