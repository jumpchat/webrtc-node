const assert = require("assert");
const rtc = require("..");

// rtc.setDebug(true);

describe("MediaDevices", function() {
    it("rtc.mediaDevices should exist", function() {
        assert.ok(rtc.mediaDevices);
    });

    it("rtc.mediaDevices.getUserMedia should exist", function() {
        assert.ok(rtc.mediaDevices.getUserMedia);
    });

    it("rtc.mediaDevices.getUserMedia get device with promise", function(done) {
        rtc.mediaDevices.getUserMedia({
            video: true,
            audio: true
        })
        .then(function(stream) {
            assert.ok(stream);
            done();
        })
        .catch(function(e) {
            assert.ok(false);
            done();
        });
    });

    it("rtc.mediaDevices.getUserMedia get device with callback", function(done) {
        rtc.mediaDevices.getUserMedia(
            {
                video: true,
                audio: true
            },
            function(stream) {
                assert.ok(stream);
                done();
            },
            function() {
                assert.ok(false);
                done();
            }
        );
    });
});
