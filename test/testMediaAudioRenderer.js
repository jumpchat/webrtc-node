const assert = require('assert');
const rtc = require('..');

// rtc.setDebug(true);

describe('MediaAudioRenderer', function() {
    this.timeout(5000);

    var stream;
    before(function() {
        rtc.mediaDevices.getUserMedia({
            audio: true,
            video: false
        }, function(_stream) {
            stream = _stream;
        });
    });

    it('stream audio tracks > 1', function(done) {
        assert.ok(stream.getAudioTracks().length > 0);
        done();
    });

    // WebRTC Bug? Local streams cannot be renderer in audio for some reason.  Possibly because of feedback?
    // it('renderer callback should be called', function(done) {
    //     var renderer = new rtc.MediaAudioRenderer();
    //     renderer.ondata = function(frame) {
    //         assert.ok(frame);
    //         renderer.onframe = null;
    //         done();
    //     }
    //     stream.getAudioTracks().forEach(function(t) {
    //         t.addRenderer(renderer);
    //     });
    // });
});
