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
setTimeout(setRed, 4000);
setTimeout(onTimeout, 5000);

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

function setRed() {
    rc.led("RED", 0x32);
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
