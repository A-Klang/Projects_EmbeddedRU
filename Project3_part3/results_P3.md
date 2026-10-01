# Project 3 - Results

## Part 1

### State diagram
The device starts in Initialization, which resets the motor controller and then moves on to Operational by itself (completion transition, no trigger). The reset command takes it from Operational back to Initialization.

![Part 1 state diagram](out/docs/diagrams/src/part1_states/part1_states.png)

- Initialization, entry: motor off, position reset, $K_p = 40$, $ref = 0$, clock restarted.
- Initialization, exit: the Boot-up message is sent, since the device is now ready to receive commands.
- Operational, entry: LED on. Do: one control update every 1ms. Exit: motor off.

### Resource allocation
Same wiring as Project 2 (encoder on D3/D4, AIN1 on D8, AIN2 on D9). What changed:

| Resource | Use in Project 3 | Notes |
|---|---|---|
| D13 (PB5), onboard LED | Status LED, owned by the states | Was toggled by the encoder ISR in Project 2, removed from `Encoder` so the states are the only ones writing to it |
| Timer1 COMPA interrupt (1ms) | Samples speed and sets a `control_tick` flag | The control update is moved out of the ISR and into `Operational::on_step()` |
| USART0 RX | Keyboard commands from the laptop | `r` = reset, read in `main()` with `Serial.available()`/`Serial.read()` |
| USART0 TX | Boot-up message and the CSV log every 10ms | Same as Project 2, 115200 baud |

Memory (from the PlatformIO build output), before and after linking in the state machine:

| | RAM | Flash |
|---|---|---|
| Before | 290 B | 4582 B |
| After | 332 B | 5038 B |

So the state machine costs 42 B of RAM and 456 B of Flash. Most of the RAM increase is the vtables, since avr-gcc puts them in SRAM and not in Flash.

### Implementation (state behaviour pattern)
The state machine follows the state pattern from L3.1, using the same names.

**`State` (base class, `state.h`)**
- Holds a `Context* context_` pointer back to the state machine, so a state can trigger its own transitions.
- Declares `on_entry()`, `on_exit()`, `on_step()` and `on_reset()` as `virtual` with empty bodies. A state only overrides the handlers it needs, so an event that isn't in the diagram for that state is ignored by default (e.g. `reset` in Initialization).

**`Context` (`context.h/.cpp`)**
- Holds a `State* state_` pointer to the current state.
- `transition_to(next)`: runs `on_exit()` of the old state, switches the pointer, gives the new state its `context_` pointer and runs `on_entry()` of the new state. `on_entry()` is last so a new transition from inside an entry action doesn't get overwritten.
- `step()` and `reset()` just forward to the current state (`state_->on_step()`, `state_->on_reset()`). Since the handlers are virtual, the version of the actual state is called, so `Context` never has to check which state it's in.
- `start()` enters the initial state. It is called from `main()` after all hardware init, since a global object is constructed before `main()` runs and the pins wouldn't be set up yet.
- `on_step()` implements the `do` activity from the diagram, `step()` is called once per 1ms tick.

**Concrete states (`nmt_states.h/.cpp`)**
- `Initialization` and `Operational` inherit from `State` and override the handlers from the diagram, one to one.
- The states are static global objects instead of `new Concrete_state` as in the lecture example. With only 2kB RAM we don't want the heap, and this way the memory use is known at compile time.
- The Init ==> Operational transition is done in `Initialization::on_step()`, so it happens on the first tick. Doing it inside `on_entry()` would call `transition_to()` from inside `transition_to()`.

**Changes to `Encoder`**
- `stop_motor()`: sets `dir_pin` low and the PWM to 0. Both pins have to be low, with `dir_pin` high and PWM 0 the H-bridge would drive the motor at full speed in reverse.
- `reset_position()`: sets `pos` to 0 and also clears the 10ms sliding window. Otherwise the next 10 speed samples would be computed as $0 - p_{old}$, a large fake negative speed that the controller would react to.

**Control loop timing**
- The Timer1 ISR still calls `sample_speed()` every 1ms, since the speed calculation needs a fixed $\Delta t$ (same as Project 2). It then sets `control_tick = true`.
- `main()` checks the flag, clears it and calls `context.step()`. In Operational this runs the P controller and sets PWM and direction, so $f_{update}$ is still 1kHz.
- This way the state decides if a control update should run at all. In Initialization no control update is done, with the control in the ISR it would always run.
- `control_tick` is `volatile` since it is changed by the ISR and polled in `main()`.

**Reset command and test profile**
- `main()` reads one character at a time from serial, `r` calls `context.reset()`. Only `Operational` overrides `on_reset()`, which calls `transition_to(&initialization)`.
- For testing, `main()` sets the reference from the time since the last (re)boot: $0$ RPM until 1s, $40$ RPM until 8s, then $60$ RPM. Since Initialization restarts the clock, the same step sequence is run again after every reset.

### Test
The motor ran the step sequence, `r` was pressed at steady state at 60 RPM (~14.8s) and the step sequence was run again. The serial output was saved with the `log2file` monitor filter. Since `time_ms` restarts from 0 on reset, the plot adds an offset after each Boot-up line so the time axis is continuous.

![Step response with reset](logs/part1_reset.png)

- Boot-up is printed at power-on and again right after `r`, and `time_ms` restarts from 8ms ==> the device went Operational ==> Initialization ==> Operational.
- After the reset the speed goes from 57 RPM to ~0 in ~70ms. `stop_motor()` only acts for the 1ms spent in Initialization, after that Operational runs with $ref = 0$, so the controller brakes the motor with reverse voltage ($u = 40 \cdot (0 - 34.3) = -1371$, clamped to $-255$).
- At standstill the speed switches between $0$ and $\pm 2.86$ RPM, which is one encoder count in the 10ms window (the speed resolution from Project 2).
- After the new step to 40 RPM the speed reaches ~37 RPM in ~30ms, same as before the reset.
- The LED stayed on during the whole run.
- Serial print interval: $10ms \pm 1ms$, no gaps, so the main loop never blocked.

### Interpretation
The state machine behaves as the diagram: Initialization runs once at boot and after every reset, and the device ends up in Operational by itself. The control behaviour is the same as in Project 2, the speed settles at ~37 RPM for 40 RPM and ~54-57 RPM for 60 RPM. This is the steady-state error of a P controller, which the PI controller in Part 3 should remove.
