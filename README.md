# node-roboticscape
Node.js bindings for libroboticscape for BeagleBoard.org (BeagleBone Blue)[https://beagleboard.org/blue] or (Robotics Cape)[https://beagleboard.org/RoboticsCape]

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
rc.on("PAUSE_PRESSED", function() { console.log("PAUSE pressed"); });
rc.motor("ENABLE");
rc.motor(0.3);
var enc1 = rc.encoder(1);

/* Set the state to RUNNING */
rc.state("EXITING");

/* The robotics cape userspace interface is automatically freed on exit */
```
