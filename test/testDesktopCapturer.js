const assert = require('assert');
const rtc = require('..');
const PNG = require('pngjs').PNG;
const fs = require('fs');

// rtc.setDebug(true);

function saveFrameYUV(frame, filename, cb) {

    function clamp(n,low,high){
        if(n<low){return(low);}
        if(n>high){return(high);}
        return parseInt(n);
    }

    var png = new PNG({
        filterType: -1,
        width: frame.width,
        height: frame.height,
    });

    var YData = new Uint8Array(frame.y);
    var UData = new Uint8Array(frame.u);
    var VData = new Uint8Array(frame.v);

    var idx = 0;
    for (var y = 0; y < frame.height; y++) {
        for (var x = 0; x < frame.width; x++) {
          // console.log(x + "x" + y)
          var Y = YData[y*frame.width + x];
          var uvIndex = parseInt(y/2)*parseInt(frame.width/2) + parseInt(x/2);
          var U = UData[uvIndex] - 128;
          var V = VData[uvIndex] - 128;
          var R = Y + V * 1.4075;
          var G = Y - U * 0.3455 - V * 0.7169;
          var B = Y + U * 1.7790;

          png.data[idx++] = clamp(Math.floor(R),0,255);
          png.data[idx++] = clamp(Math.floor(G),0,255);
          png.data[idx++] = clamp(Math.floor(B),0,255);
          png.data[idx++] = 0xff;
        }
    }

    var filePath = __dirname + '/../' + filename;
    console.log('Saving', filename);

    png.pack()
        .pipe(fs.createWriteStream(filePath)
            .on('finish', cb));
}

function saveFrameRGBA(frame, filename, cb) {
    var png = new PNG({
        filterType: -1,
        width: frame.width,
        height: frame.height,
    });

    png.data = new Uint8Array(frame.data);
    var filePath = __dirname + '/../' + filename;
    console.log('Saving', filename);

    png.pack()
        .pipe(fs.createWriteStream(filePath)
            .on('finish', cb));
}

describe('ScreencastCapturer', function() {
    this.timeout(5000);

    var stream;
    before(function() {
        rtc.mediaDevices.getUserMedia({
            audio: false,
            video: {
                optional: [{
                    sourceId: 'desktop'
                }]
            }
        }, function(_stream) {
            stream = _stream;
        });
    });

    // it('desktop stream should exist', function() {
    //     assert.ok(stream)
    // });

    it('capture desktop stream', function(done) {
        var renderer = new rtc.MediaVideoRenderer();
        renderer.onframe = function(frame) {
            renderer.onframe = null;
            assert.ok(frame);
            saveFrameYUV(frame, 'desktop-frame-yuv.png', done);
            stream.getVideoTracks().forEach(function(t) {
                t.enabled = false;
                stream.removeTrack(t);
            });
        }
        stream.getVideoTracks().forEach(function(t) {
            t.addRenderer(renderer);
        });
    })
});

describe('DesktopCapturer', function() {

    it('desktop created', function() {
        var dc = new rtc.DesktopCapturer();
        assert.ok(dc);
    });

    it('desktop source list', function(done) {
        var dc = new rtc.DesktopCapturer();
        var sources = dc.getSourceList();
        assert.ok(sources);
        assert.ok(sources.length > 0);
        done();
    });

    it('desktop select source', function(done) {
        var dc = new rtc.DesktopCapturer();
        var sources = dc.getSourceList();
        dc.selectSource(sources[0].id);
        done();
    });

    it('desktop capture frame', function(done) {
        var dc = new rtc.DesktopCapturer();
        dc.onframe = function(frame) {
            dc.stop();
            dc.onframe = null;
            assert.ok(frame);
            saveFrameRGBA(frame, 'desktop-frame-rgba.png', done);
        }
        var sources = dc.getSourceList();
        dc.selectSource(sources[0].id);
        dc.start();
        dc.captureFrame();
    });
});
