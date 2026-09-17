# Project 2 - Results

## Part 1
No results, see code.

## Part 2
### Maximum motorspeed = $79.6$ RPM

### Time constant of the motor under operating load:
- Calculated 63% of max speed ==> $79.6*0.63 = 50.1$
- Counted number of interrupts with $period_{ms}=1$ while speed is between $0$ and $50.1$ $=24$ ==> 24ms.

## Part 3
### Minimum required update rate
$f_{update} = 10/$tao = $10/(24*10^{-3})$ = 416.7

### Timing requirements & resource allocation

**Task 1: Counting encoder pulses**
To count all the encoder pulses we use the interrupt from previously to do the following:
- At max speed (79.6 RPM), pulses will arrive every $\frac{1}{(79.6/60)*2100} = 359$ microseconds in the worst-case scenario.
- Each needs to be handled before the next one arrives but this is handled by the interrupt running every time a pin-change is detected regardless of other CPU activity.

**Task 2: Computing speed over a stable time base**
Since speed = $\frac{\Delta p}{\Delta t}$, $\Delta t$ needs to be constant (stable time base) and it must tick at 416.7 Hz (period_ms <= 2.4ms) to meet update requirement above. Timer1 in CTC mode guarantees fixed, hardware-timed interval which is independent of software execution time. Currently period_ms = 1ms which satisfies condition with margin.

**Task 3: Updating the control output at the required rate**
Control update should be based on a fresh measurement so we call P_controller inside the existing Timer1 interrupt directly after sample_speed.

**Task 4: Storing and transmitting the response to the laptop**
Inside the interrupt (timer1) we store the speed and control output inside some volatile variable (array?) that we can then, from main(), access to write/plot over serial every N:th amount of ticks.


**Task 5: Stable PWM output for the motor**
We want to choose a $f_{pwm}$ that is sufficiently faster than the $f_{update}$ that the average output voltage "looks" stable even if we are actually just switching the pin on/off (5V/0V) instead of "pulsing" which is what might happen if we set 5V/0V switch not often enough.
