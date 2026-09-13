# LeftWallFollow

This is the algorithm we have implemented which consists of a logic where the bot follows the adjacent left wall along its entire path finding its way out.

## Project Structure

```
LeftWallFollow/
├── CMakeLists.txt
├── include/
│   ├── ir.h      — IR sensor pin/channel definitions
│   ├── motor.h    — motor GPIO pins, MCPWM function declarations
│   ├── tof.h       — I2C bus config, VL53L0X register addresses
│   └── wifi.h       — tunable-parameters (Kp, Kd, thresholds,
│                       duty limits)
├── main/
│   ├── CMakeLists.txt
│   └── main.c        — app_main(): independent-rate sensor/logic timer,
│                        left-hand-follow, PD wall-centering
└── src/
    ├── ir.c    — ADC oneshot init, channel config
    ├── motor.c  — MCPWM setup, motor run/reverse/brake, turn maneuvers
    ├── tof.c     — I2C init, VL53L0X register read/write, distance calc
    └── wifi.c     — STA WiFi connection, HTTP tuning page
```

## Control Loop

Two independently-timed tasks share one `while(1)` loop, using `esp_timer_get_time()` countdowns rather than a single fixed `vTaskDelay()`:

- **Main logic** (`MAIN_LOGIC_RATE`) — reads all 4 IR channels, runs the left-hand-follow decision, applies PD correction or executes a turn.
- **ToF poll** — reads front distance on its own slower interval, independent of how often the main logic runs.

### Sensor roles

- **ToF sensor** — front-wall brake. Below the configured stop distance, the bot halts and evaluates a turn/U-turn.
- **Front-left / front-right IR** — feed the PD controller for straight-line wall-centering.
- **Diagonal IR (left/right)** — junction detection. Openings must read consistently across several consecutive loop iterations (debounced) before a turn is committed to, filtering single-sample sensor noise.

### Left-hand-follow priority

1. Left diagonal confirms an opening → turn left.
2. Else front is clear (ToF beyond stop distance) → straight, PD-centered.
3. Else right diagonal confirms an opening → turn right.
4. Else (all three blocked) → U-turn.

PD state (`last_error`) is reset immediately after every turn, so the derivative term doesn't react to a stale, pre-turn error value once centering resumes.

### Motor behavior

- **Forward:** both motors, equal duty.
- **Turn left/right:** one wheel at reduced duty, the other at increased duty, for a fixed duration.
- **U-turn:** one motor forward, the other in reverse, for roughly double a single turn's duration.

## Live Tuning over WiFi

On boot the bot joins WiFi and starts an HTTP server. The served page exposes `Kp`, `Kd`, `left_turn_threshold`, `right_turn_threshold`, `wall_distance`, and the duty-cycle limits as live-editable fields — submitting updates the running firmware's `volatile` globals directly, no reflash needed.
