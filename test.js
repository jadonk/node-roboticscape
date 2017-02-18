var rc = require('./build/Release/roboticscape');
var fs = require('fs');
var assert = require('assert');

assert.equal(rc.initialize(), false);
fs.readFile('/var/run/robotics_cape.pid', readRCpid);
assert.equal(rc.get_state(), "UNINITIALIZED");
rc.set_state("RUNNING");
assert.equal(rc.get_state(), "RUNNING");
rc.set_state("PAUSED");
assert.equal(rc.get_state(), "PAUSED");
rc.set_state("EXITING");
assert.equal(rc.get_state(), "EXITING");

function readRCpid(err, data) {
    assert.equal(data, process.pid);
}
