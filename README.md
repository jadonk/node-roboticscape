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

