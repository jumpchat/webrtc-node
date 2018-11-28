const assert = require('assert');
const rtc = require('..');

// rtc.setDebug(true);

describe('MediaStream', function() {
    this.timeout(5000);

    var stream;
    before(function() {
        rtc.mediaDevices.getUserMedia({
            audio: true,
            video: true
        }, function(_stream) {
            stream = _stream;
        });
    });

    it('stream created & valid', function() {
        assert.ok(stream);
        assert.ok(stream.getAudioTracks().length > 0);
        assert.ok(stream.getVideoTracks().length > 0);
    });

    it('stream addTrack/removeTrack', function() {
        assert.ok(stream);
        var audioTrack = stream.getAudioTracks()[0];
        stream.removeTrack(audioTrack);
        assert.ok(stream.getAudioTracks().length == 0);
        stream.addTrack(audioTrack);
        assert.ok(stream.getAudioTracks().length == 1);

        var videoTrack = stream.getVideoTracks()[0];
        stream.removeTrack(videoTrack);
        assert.ok(stream.getVideoTracks().length == 0);
        stream.addTrack(videoTrack);
        assert.ok(stream.getVideoTracks().length == 1);
    });

    it('stream clone', function() {
        assert.ok(stream);
        var clonedStream = stream.clone();
        assert.ok(stream.id != clonedStream.id);
        assert.deepEqual(stream.getAudioTracks(), clonedStream.getAudioTracks());
        assert.deepEqual(stream.getVideoTracks(), clonedStream.getVideoTracks());
    });

    it('stream check track properties', function() {
        assert.ok(stream.id);
        assert.ok(stream.active == true);
        assert.ok(stream.ended == false);
    });

    it('stream onaddtrack/onremovetrack', function(done) {
        var videoTrack = stream.getVideoTracks()[0];
        stream.onremovetrack = function(track) {
            assert.equal(videoTrack.id, track.id);
            stream.onaddtrack = function(track) {
                assert.equal(videoTrack.id, track.id);
                done();
            };
            stream.addTrack(videoTrack);
        };
        stream.removeTrack(videoTrack);
    });


});
