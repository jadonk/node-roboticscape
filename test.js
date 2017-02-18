var rc = require('./build/Release/roboticscape');
var fs = require('fs');
var assert = require('assert');

assert.equal(rc.initialize(), false);
fs.readFile('/var/run/robotics_cape.pid', readRCpid);
assert.equal(rc.get_state(), "UNINITIALIZED");

function readRCpid(err, data) {
    assert.equal(data, process.pid);
}
