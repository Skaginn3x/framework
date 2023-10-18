# Motors interface under tfc
## SpeedRatio
A concept that
maps a motors capable
range from 1-100%

### Stopping
 (-1<=>1) is outside the motors
 range by definition and
 the motor shall be stopped
 if a speed is applied in that range.

### Rationalization
We have a plethora of
use cases where precise motor
speeds are not essential.
We also utilize a large range
of frequencies, rpm's and gear
ratios in our motors, causing
many other physical quantities
to be a bad fit for simple general
purpose use cases.

## Activating a running motor
If activating a running motor
the motor shall run with the new
parameters provided, fe. changing
speeds. The new speed shall be used.

This also applies if the motor
is switched from running continuously 
to running with a set end point. The
motor shall than stop after the distance
or time provided.


## Run modes
Each motor interface can decide
its basic capability aside from 
the basic run mode at a fractional
speed. If the motor interface cannot
complete it's given task a std::error_code
shall be returned with an enum and string as to
why.

The run modes can be thought of
as 
- just run
- just run for some time
- just run for some amount
- just go there

These are then hidden behind different apis
to facilitate different use cases.
f.e. 
a pump uses the `.pump` function call
while a conveyor uses the `.convey` function.

### Why complicate simple motors
Many different motor have different properties
Servos and synchronous motors have positions.
But a async motor with a nominal mm/s @ 50hz
also has a decent distance estimate, an 
async motor coupled with a tachometer or an
encoder has even better precision.



## Interface idea
```cpp
int main(){
    motor_types m{};

    /// Liquid transport
    m.pump(); // config speed
    m.pump(10 l / s);
    m.pump(10 l / min, 10 l, [](const std::error_code&){});
    m.pump(10 l / min, 10 minutes, [](const std::error_code&){});

    /// Linear transport
    m.convey(10m/s);
    m.convey(10m/s, 10meters, [](const std::error_code&){});
    m.convey(10m/s, 10 minutes, [](const std::error_code&){});
    m.convey(10 meters, [](const std::error_code&){});
    m.convey(10 minutes, [](const std::error_code&){});

    // Absolute positioning
    // 0                               100
    // |               <=>             |
    m.move(10m);
    m.move(10m, [](const std::error_code&){}););
    m.move_home([](const std::error_code&){});

    /// Rotational transport
    m.rotate(1rpm);
    m.rotate(1rpm, 90°, [](const std::error_code&){});
    m.rotate(1rpm, 10 minutes, [](const std::error_code&){});

    /// All types
    m.run(50%); // Run with SpeedRatio
    m.stop();   // Stop freewheel

    // Stop with deceleration specified
    // specifing a deceleration overloads
    // the configured deceleration but
    // it does not replace it!
    //
    // If a sequence of
    // m.run(50%);
    // m.stop(100ms);
    // m.run(50%);
    // m.stop(); <- This uses the configured deceleration
    m.stop(100ms);
    m.quick_stop(); // Stop the motor with quick stop

    /// Getting notified of a continously running motor
    m.notify(10l, ()[]{});
    m.notify(10m, ()[]{});
    m.notify(10 * 2*pi, ()[]{});


    /// Special cases
    m.run(0%);  // Stop the motor
    m.run(); // Start the motor at configured speed

    // Calling convey, rotate or move
    m.run(50%);
    m.run(-50%);
}
```