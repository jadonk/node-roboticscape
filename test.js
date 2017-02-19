var rc = require('./build/Release/roboticscape');
var fs = require('fs');
var assert = require('assert');

assert.equal(rc.initialize(), false);
fs.readFile('/var/run/robotics_cape.pid', readRCpid);
assert.equal(rc.get_state(), "UNINITIALIZED");
console.log("Setting state RUNNING")
rc.set_state("RUNNING");
assert.equal(rc.get_state(), "RUNNING");

rc.set_pause_pressed_func(onPausePressed);
rc.set_pause_released_func(onPauseReleased);
rc.set_mode_pressed_func(onModePressed);
rc.set_mode_released_func(onModeReleased);
console.log("Press buttons now")
setTimeout(onTimeout, 5000);

function onTimeout() {
    console.log("Setting state PAUSED")
    rc.set_state("PAUSED");
    assert.equal(rc.get_state(), "PAUSED");
    console.log("Setting state EXITING")
    rc.set_state("EXITING");
    assert.equal(rc.get_state(), "EXITING");
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
