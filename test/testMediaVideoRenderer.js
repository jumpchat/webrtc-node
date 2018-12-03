const assert = require('assert');
const rtc = require('..');

// rtc.setDebug(true);

describe('MediaVideoRenderer', function() {
    this.timeout(5000);

    var stream;
    before(function() {
        rtc.mediaDevices.getUserMedia({
            audio: false,
            video: true
        }, function(_stream) {
            stream = _stream;
        });
    });

    it('stream video tracks > 1', function(done) {
        assert.ok(stream.getVideoTracks().length > 0);
        done();
    });

    it('renderer callback should be called', function(done) {
        var renderer = new rtc.MediaVideoRenderer();
        renderer.onframe = function(frame) {
            assert.ok(frame);
            renderer.onframe = null;
            done();
        }
        stream.getVideoTracks().forEach(function(t) {
            t.addRenderer(renderer);
        });
    });
});
