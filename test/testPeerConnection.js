const assert = require('assert');
const rtc = require('..');

// rtc.setDebug(true);

describe('RTCPeerConnection', function() {
    var pc1;
    var pc2;
    before(function() {
    });

    it('pc created', function(done) {
        var pc = new rtc.RTCPeerConnection();
        assert.ok(pc);
        done();
    });

    it('pc getConfiguration url style', function(done) {
        var pc = new rtc.RTCPeerConnection({
            iceServers: [{
                url: "stun:stun.l.google.com:19302",
            }]
        });
        var config = pc.getConfiguration();
        assert.equal(config.bundlePolicy, 'balanced');
        assert.equal(config.iceServers[0].urls[0], 'stun:stun.l.google.com:19302');
        pc.close();
        done();
    });

    it('pc getConfiguration urls style', function(done) {
        var pc = new rtc.RTCPeerConnection({
            iceServers: [{
                urls: "stun:stun.l.google.com:19302",
            }]
        });
        var config = pc.getConfiguration();
        assert.equal(config.bundlePolicy, 'balanced');
        assert.equal(config.iceServers[0].urls[0], 'stun:stun.l.google.com:19302');
        pc.close();
        done();
    });

    it('pc getConfiguration urls[] style', function(done) {
        var pc = new rtc.RTCPeerConnection({
            iceServers: [{
                urls: [
                    "stun:stun.l.google.com:19302",
                    "stun:stun1.l.google.com:19302",
                    "stun:stun2.l.google.com:19302",
                    "stun:stun3.l.google.com:19302",
                    "stun:stun4.l.google.com:19302",
                ]
            }]
        });
        var config = pc.getConfiguration();
        assert.equal(config.bundlePolicy, 'balanced');
        assert.equal(config.iceServers[0].urls[0], 'stun:stun.l.google.com:19302');
        assert.equal(config.iceServers[0].urls.length, 5);
        pc.close();
        done();
    });


    it('pc createOffer cb', function(done) {
        var pc = new rtc.RTCPeerConnection();
        pc.createOffer(function(sdp) {
            assert.ok(sdp);
            pc.close();
            done();
        }, console.error, {audio: true, video: true});
    });

    it('pc createOffer promise', function() {
        var pc = new rtc.RTCPeerConnection();
        return pc.createOffer({audio: true, video: true}).then(function(sdp) {
            assert.ok(sdp);
            pc.close();
        });
    });

    it('pc setLocalDescription cb', function() {
        var pc = new rtc.RTCPeerConnection();
        return pc.createOffer(function(sdp) {
            assert.ok(sdp);
            pc.setLocalDescription(sdp, function() {
                assert.ok(true);
                pc.close();
            })
        });
    });

    it('pc setLocalDescription promise', function() {
        var pc = new rtc.RTCPeerConnection();
        return pc.createOffer({audio: true, video: true}).then(function(sdp) {
            assert.ok(sdp.sdp);
            return pc.setLocalDescription(sdp);
        }).then(function() {
            assert.ok(true);
            pc.close();
        });
    });

    it('pc setRemoteDescription promise', function() {
        var pc1 = new rtc.RTCPeerConnection();
        var pc2 = new rtc.RTCPeerConnection();
        var localSDP1;
        var localSDP2;
        return pc1.createOffer({audio: true, video: true}).then(function(sdp) {
            assert.ok(sdp.sdp);
            localSDP1 = sdp;
            return pc1.setLocalDescription(localSDP1);
        }).then(function() {
            assert.ok(true);
            return pc2.setRemoteDescription(localSDP1);
        }).then(function() {
            assert.ok(true);
            return pc2.createAnswer();
        }).then(function(sdp) {
            assert.ok(sdp.sdp);
            localSDP2 = sdp;
            return pc2.setLocalDescription(localSDP2);
        }).then(function(sdp) {
            assert.ok(true);
            return pc1.setRemoteDescription(localSDP2);
        }).then(function() {
            assert.deepEqual(pc1.localDescription, localSDP1);
            assert.deepEqual(pc1.remoteDescription, localSDP2);
            assert.deepEqual(pc2.localDescription, localSDP2);
            assert.deepEqual(pc2.remoteDescription, localSDP1);
            pc1.close();
            pc2.close();
        })
    });

});
