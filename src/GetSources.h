#ifndef WEBRTC_GETSOURCES_H
#define WEBRTC_GETSOURCES_H

#include "Common.h"

namespace WebRTC {

class MediaConstraints;

class GetSources {
public:
    static void Init(v8::Handle<v8::Object> exports);

    static rtc::scoped_refptr<webrtc::AudioTrackInterface> GetAudioSource(const rtc::scoped_refptr<MediaConstraints>& constraints);
    static rtc::scoped_refptr<webrtc::AudioTrackInterface> GetAudioSource(const std::string id, const rtc::scoped_refptr<MediaConstraints>& constraints);

    static rtc::scoped_refptr<webrtc::VideoTrackInterface> GetVideoSource(const rtc::scoped_refptr<MediaConstraints>& constraints);
    static rtc::scoped_refptr<webrtc::VideoTrackInterface> GetVideoSource(const std::string id, const rtc::scoped_refptr<MediaConstraints>& constraints);

    static v8::Local<v8::Value> GetDevices();

private:
    static void GetDevices(const Nan::FunctionCallbackInfo<v8::Value>& info);
};
};

#endif
