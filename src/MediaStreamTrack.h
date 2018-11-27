#ifndef WEBRTC_MEDIASTREAMTRACK_H
#define WEBRTC_MEDIASTREAMTRACK_H

#include "Common.h"
#include "EventEmitter.h"

namespace WebRTC {
enum MediaStreamTrackEvent {
    kMediaStreamTrackChanged
};

class MediaStreamTrack : public Nan::ObjectWrap, public EventEmitter {
public:
    static NAN_MODULE_INIT(Init);

    static v8::Local<v8::Value> New(rtc::scoped_refptr<webrtc::AudioTrackInterface> audioTrack);
    static v8::Local<v8::Value> New(rtc::scoped_refptr<webrtc::VideoTrackInterface> videoTrack);

    static rtc::scoped_refptr<webrtc::MediaStreamTrackInterface> Unwrap(v8::Local<v8::Object> value);
    static rtc::scoped_refptr<webrtc::MediaStreamTrackInterface> Unwrap(v8::Local<v8::Value> value);

private:
    MediaStreamTrack();
    ~MediaStreamTrack() final;

    static NAN_METHOD(New);
    static NAN_METHOD(GetConstraints);
    static NAN_METHOD(ApplyConstraints);
    static NAN_METHOD(GetSettings);
    static NAN_METHOD(GetCapabilities);
    static NAN_METHOD(Clone);
    static NAN_METHOD(AddRenderer);
    static NAN_METHOD(RemoveRenderer);
    static NAN_METHOD(Stop);

    static NAN_GETTER(GetEnabled);
    static NAN_GETTER(GetId);
    static NAN_GETTER(GetKind);
    static NAN_GETTER(GetLabel);
    static NAN_GETTER(GetMuted);
    static NAN_GETTER(GetReadOnly);
    static NAN_GETTER(GetReadyState);
    static NAN_GETTER(GetRemote);
    static NAN_GETTER(GetOnStarted);
    static NAN_GETTER(GetOnMute);
    static NAN_GETTER(GetOnUnMute);
    static NAN_GETTER(GetOnOverConstrained);
    static NAN_GETTER(GetOnEnded);

    static NAN_SETTER(SetReadOnly);
    static NAN_SETTER(SetEnabled);
    static NAN_SETTER(SetOnStarted);
    static NAN_SETTER(SetOnMute);
    static NAN_SETTER(SetOnUnMute);
    static NAN_SETTER(SetOnOverConstrained);
    static NAN_SETTER(SetOnEnded);

    void CheckState();
    void On(Event* event) final;

protected:
    bool isAudioTrack;
    bool isVideoTrack;

    rtc::scoped_refptr<webrtc::MediaStreamTrackInterface> _track;
    rtc::scoped_refptr<webrtc::MediaSourceInterface> _source;
    // rtc::scoped_refptr<MediaStreamTrackObserver> _observer;

    webrtc::MediaStreamTrackInterface::TrackState _track_state;
    webrtc::MediaSourceInterface::SourceState _source_state;

    Nan::Persistent<v8::Function> _onstarted;
    Nan::Persistent<v8::Function> _onmute;
    Nan::Persistent<v8::Function> _onunmute;
    Nan::Persistent<v8::Function> _onoverconstrained;
    Nan::Persistent<v8::Function> _onended;

    static Nan::Persistent<v8::Function> constructor;
};
};

#endif
