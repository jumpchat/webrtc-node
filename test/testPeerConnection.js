const assert = require('assert');
const rtc = require('..');

// rtc.setDebug(true);

describe('RTCPeerConnection', function() {
    this.timeout(10000);

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


    it('pc connection', function(done) {
        var config = {
            iceServers: [{
                urls: [
                    "stun:stun.l.google.com:19302",
                    "stun:stun1.l.google.com:19302",
                    "stun:stun2.l.google.com:19302",
                    "stun:stun3.l.google.com:19302",
                    "stun:stun4.l.google.com:19302",
                ]
            }]
        }
        var pc1 = new rtc.RTCPeerConnection(config);
        var pc2 = new rtc.RTCPeerConnection(config);

        afterEach(function() {
            pc1.close();
            pc2.close();
        });

        pc1.onicecandidate = function(event) {
            // console.log('pc1 ice candidate', event);
            if (event.candidate) {
                pc2.addIceCandidate(event.candidate);
            }
        }

        pc2.onicecandidate = function(event) {
            // console.log('pc2 ice candidate', event);
            if (event.candidate) {
                pc1.addIceCandidate(event.candidate);
            }
        }


        pc1.oniceconnectionstatechange = function(event) {
            // console.log('iceConnectionState ', pc1.iceConnectionState, pc2.iceConnectionState);
            if (pc1.iceConnectionState == 'completed' &&
                pc2.iceConnectionState == 'connected') {
                    pc1.close();
                    pc2.close();
                    done();
                }
        }

        pc2.oniceconnectionstatechange = function(event) {
            // console.log('iceConnectionState ', pc1.iceConnectionState, pc2.iceConnectionState);
            if (pc1.iceConnectionState == 'completed' &&
                pc2.iceConnectionState == 'connected') {
                    pc1.close();
                    pc2.close();
                    done();
                }
        }

        pc1.ontrack = function(track) {
            track.enabled = false;
        }

        pc2.ontrack = function(track) {
            track.enabled = false;
        }

        var localSDP1;
        var localSDP2;
        rtc.mediaDevices.getUserMedia({
            video: false,
            audio: true
        }).then(function(stream) {
            pc1.addStream(stream);
            return pc1.createOffer({audio: true, video: true})
        }).then(function(sdp) {
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
        })
    });

    it('pc remote stream', function(done) {
        var config = {
            iceServers: [{
                urls: [
                    "stun:stun.l.google.com:19302",
                    "stun:stun1.l.google.com:19302",
                    "stun:stun2.l.google.com:19302",
                    "stun:stun3.l.google.com:19302",
                    "stun:stun4.l.google.com:19302",
                ]
            }],
            sdpSemantics: 'unified-plan'
        }
        var pc1 = new rtc.RTCPeerConnection(config);
        var pc2 = new rtc.RTCPeerConnection(config);
        var stream;

        afterEach(function() {
            pc1.close();
            pc2.close();
        });

        pc1.onicecandidate = function(event) {
            if (event.candidate) {
                pc2.addIceCandidate(event.candidate);
            }
        }

        pc2.onicecandidate = function(event) {
            if (event.candidate) {
                pc1.addIceCandidate(event.candidate);
            }
        }

        pc1.onaddstream = function(event) {
        }

        pc2.onaddstream = function(event) {
            stream = event.stream;
        }

        pc2.ontrack = function(event) {
            // console.log('ontrack', event);
        }

        var localSDP1;
        var localSDP2;
        rtc.mediaDevices.getUserMedia({
            video: false,
            audio: true
        }).then(function(stream) {
            pc1.addStream(stream);
            return pc1.createOffer({audio: true, video: true})
        }).then(function(sdp) {
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
            assert.ok(stream);
            assert.ok(stream.getAudioTracks().length, 1);
            done();
        })
    });
});
