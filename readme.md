Arduino sketches for [CMU](https://cmu.edu) Prof. [Susan Finger](https://www.cmu.edu/cee/people/faculty/finger.html)'s students to use for data logging of high-speed oscillations of a sensor. (Specifically: they are pouring water through a pipe in series with a flow meter, which pulls a pin high and low at hundreds of hertz as the water flows.)

See the comment blocks at the top of both sketches for technical details, wiring, etc.

# count-logger

Periodically displays the number of milliseconds since the Arduino started, and the number of pulses observed since startup. These records are written to the serial monitor as well as to an attached SD card. (N.b.: the sketch will not output any data without an SD card successfully initializing.)

# frequency-finder

Calculates the instantaneous frequency (in hertz) of the oscillation of the sensor, and reports this via the serial monitor. Performs smoothing to reduce jitter in the frequency data. Does not record data to an SD card.

# distance-logger

Reads an ultrasonic ranger and records values (in centimeters) on an SD card and also sends serial updates.

---

This software is released to the public domain by the author.
