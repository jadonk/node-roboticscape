# node-roboticscape
Node.js bindings for libroboticscape for BeagleBoard.org [BeagleBone Blue](https://beagleboard.org/blue) or [Robotics Cape](https://beagleboard.org/RoboticsCape)

## libroboticscape documentation
http://www.strawsondesign.com/#!manual-init-cleanup

# Installation

## From npm

```sh
$ sudo npm install --global roboticscape
```

## From source

```sh
$ git clone https://github.com/jadonk/node-roboticscape
$ cd node-roboticscape
$ sudo npm install --global
```

# Usage

```js
var rc = require('roboticscape');

/* Allocate the userspace usage of the robotics cape features */
rc.initialize();

/* Set the state to RUNNING */
rc.state("RUNNING");

/* Exercise the robotics cape hardware */
rc.led("GREEN", true);
rc.on("PAUSE_PRESSED", function() { 
    console.log("PAUSE pressed");
    
    /* Set the state to EXITING */
    rc.state("EXITING");
});
rc.motor("ENABLE");
rc.motor(0.3);

/* Read encoder every second until PAUSE button pressed */
setInterval(function() {
    if(rc.state() == "RUNNING") {
        var enc1 = rc.encoder(1);
        console.log("encoder 1 = " + enc1);
    } else {
        /* The robotics cape userspace interface is automatically freed on exit */
        process.exit();
    }
}, 1000);
```

# API

### Table of Contents

-   [initialize](#initialize)
-   [state](#state)
-   [led](#led)
-   [on](#on)
-   [motor](#motor)
-   [encoder](#encoder)

## initialize

Allocate the userspace usage of the robotics cape features and initialize the hardware functions.

See the [Init and Cleanup libroboticscape documentation](http://www.strawsondesign.com/#!manual-init-cleanup).

**Examples**

```javascript
var init = rc.initialize();
```

Returns **[Number](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Number)** where
* -2 : invalid contents in PID_FILE
* -1 : existing project failed to close cleanly and had to be killed
* 0 : No existing program is running
* 1 : An existing program was running but it shut down cleanly.

## state

Set or read the libroboticscape program state.

See the [Program Flow State libroboticscape documentation](http://www.strawsondesign.com/#!manual-flow-state).

**Examples**

```javascript
rc.state("RUNNING");
var state = rc.state();
```

**Parameters**

-  `state` **[String](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/String)** state to enter must be one of
   "UNINITIALIZED", "RUNNING", "PAUSED" or "EXITING". If undefined, the current state will be returned instead of entering a new state.

Returns **[String](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/String)** when no arguments are provided with one of the following values
* "UNINITIALIZED"
* "RUNNING"
* "PAUSED"
* "EXITING"

## led

Set the red and green user LEDs.

See the [LEDs libroboticscape documentation](http://www.strawsondesign.com/#!manual-leds).

**Examples**

```javascript
rc.led("GREEN", true);
```

**Parameters**

-  `led` **[String](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/String)** led name to set state should be either
   "RED" or "GREEN".
-  `state` **[Boolean](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Boolean)** true for on and false for off.

## on

Register event callbacks. Currently, only the button press events have been implemented.

See the [Buttons libroboticscape documentation](http://www.strawsondesign.com/#!manual-buttons).

**Examples**

```javascript
rc.on("MODE_RELEASED", function() { console.log("MODE button released."); });
```

**Parameters**

-  `event` **[String](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/String)** name 
   of event should be one of "PAUSE\_PRESSED", "PAUSE\_RELEASED", "MODE\_PRESSED" or "MODE\_RELEASED".
-  `callback` **[Function](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Function)** function
   doesn\'t take any arguments.

## motor

Control the DC motor outputs.

See the [DC Motors libroboticscape documentation](http://www.strawsondesign.com/#!manual-dc-motors).

**Examples**

```javascript
rc.motor("ENABLE");
rc.motor(1, 0.3);
```

**Parameters**

-  `motor` **[Number](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Number)** the 
   motor to target.  (Optional, defaults to setting the value for all motors. Remove if not used; do not just set 
   to `undefined`.)
-  `value` **[String](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/String) | [Number](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Number)** value
   or mode to set motor or motors. Can be one of "ENABLE", "DISABLE", "FREE_SPIN", "BRAKE" or a number from
   -1 to 1. The "ENABLE" and "DISABLE" arguments can currently only be applied when no motor is specified.

## encoder

Read and set the count on the quadrature encoder input counters.

See the [Quadrature Encoder Counting libroboticscape documentation](http://www.strawsondesign.com/#!manual-encoders).

**Examples**

```javascript
var enc1 = rc.encoder(1);
rc.encoder(1, 0);
```

**Parameters**

-  `encoder` **[Number](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Number)** the 
   index of the encoder to read or set.
-  `value` **[Number](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Number)** value
   to which to set the encoder count.  (Optional, defaults to reading only.)

Returns **[Number](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Number)** current
   encoder count.

## adc

Read a channel of the analog-to-digital converter.

See the [Analog to Digital Converter libroboticscape documentation](http://www.strawsondesign.com/#!manual-adc).

**Examples**

```javascript
var value = rc.adc(1);
```

**Parameters**

-  `channel` **[Number](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Number) | [String](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/String)** the 
   channel to read. Possible values are "BATTERY", "DC_JACK", 0, 1, 2 or 3.

Returns **[Number](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Number)** voltage
   measured by the ADC on the specified channel.
