#ifndef WEBRTC_MEDIASTREAM_H
#define WEBRTC_MEDIASTREAM_H

#include "Common.h"
#include "EventEmitter.h"
// #include "Observers.h"

namespace WebRTC {
class MediaStream : public Nan::ObjectWrap, public EventEmitter, public webrtc::ObserverInterface {
public:
    enum MediaStreamEvent {
        kMediaStreamChanged
    };

    static NAN_MODULE_INIT(Init);
    static v8::Local<v8::Value> New(rtc::scoped_refptr<webrtc::MediaStreamInterface> mediaStream);

    static rtc::scoped_refptr<webrtc::MediaStreamInterface> Unwrap(v8::Local<v8::Object> value);
    static rtc::scoped_refptr<webrtc::MediaStreamInterface> Unwrap(v8::Local<v8::Value> value);

private:
    MediaStream();
    ~MediaStream() final;

    static NAN_METHOD(New);
    static NAN_METHOD(AddTrack);
    static NAN_METHOD(Clone);
    static NAN_METHOD(GetTrackById);
    static NAN_METHOD(GetAudioTracks);
    static NAN_METHOD(GetVideoTracks);
    static NAN_METHOD(GetTracks);
    static NAN_METHOD(RemoveTrack);

    static NAN_GETTER(GetActive);
    static NAN_GETTER(GetEnded);
    static NAN_GETTER(GetId);
    static NAN_GETTER(GetOnAddTrack);
    static NAN_GETTER(GetOnRemoveTrack);

    static NAN_SETTER(ReadOnly);
    static NAN_SETTER(SetOnAddTrack);
    static NAN_SETTER(SetOnRemoveTrack);

    void CheckState();
    void On(Event* event) final;

    // webrtc::ObserverInterface
    void OnChanged() final;

protected:
    bool _active;
    bool _ended;

    Nan::Persistent<v8::Function> _onaddtrack;
    Nan::Persistent<v8::Function> _onremovetrack;

    rtc::scoped_refptr<webrtc::MediaStreamInterface> _stream;

    webrtc::AudioTrackVector _audio_tracks;
    webrtc::VideoTrackVector _video_tracks;

    static Nan::Persistent<v8::Function> constructor;
};
};

#endif
