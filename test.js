var rc = require('./build/Release/roboticscape');
var fs = require('fs');
var assert = require('assert');

assert.equal(rc.initialize(), false);

fs.readFile('/var/run/robotics_cape.pid', readRCpid);

function readRCpid(err, data) {
    assert.equal(data, process.pid);
}
