var rc = require('./build/Release/roboticscape');
var fs = require('fs');
var assert = require('assert');

assert.equal(rc.initialize(), false);
fs.readFile('/var/run/robotics_cape.pid', readRCpid);
assert.equal(rc.state(), "UNINITIALIZED");
console.log("Setting state RUNNING")
rc.state("RUNNING");
assert.equal(rc.state(), "RUNNING");
rc.led("GREEN", true);
rc.led("RED", 0);
rc.on("PAUSE_PRESSED", onPausePressed);
rc.on("PAUSE_RELEASED", onPauseReleased);
rc.on("MODE_PRESSED", onModePressed);
rc.on("MODE_RELEASED", onModeReleased);
console.log("Press buttons now");
rc.motors("ENABLE");
setTimeout(testMotors, 2000);

function testMotors() {
    console.log("Running all motors forward");
    rc.motors("ENABLE");
    rc.motors(0.3);
    setTimeout(nextMotors, 2000);
}

function nextMotors() {
    console.log("Running motors in alternate directions");
    rc.motor(1, 0.3);
    rc.motor(2, -0.3)
    setTimeout(doneWithMotors, 2000);
}

function doneWithMotors() {
    console.log("Disabling all motors");
    rc.motors("DISABLE");
    setTimeout(setRed, 1000);
}

function setRed() {
    rc.led("RED", 0x32);
    setTimeout(onTimeout, 2000);
}

function onTimeout() {
    console.log("Setting state PAUSED")
    rc.state("PAUSED");
    assert.equal(rc.state(), "PAUSED");
    console.log("Setting state EXITING")
    rc.state("EXITING");
    assert.equal(rc.state(), "EXITING");
    rc.led("GREEN", 0);
    rc.led("RED", false);
    process.exit();
}
function readRCpid(err, data) {
    assert.equal(data, process.pid);
}

function onPausePressed() {
    console.log("Pause button pressed");
}

function onPauseReleased() {
    console.log("Pause button released");
}

function onModePressed() {
    console.log("Mode button pressed");
}

function onModeReleased() {
    console.log("Mode button released");
}
