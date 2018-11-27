
#ifndef WEBRTC_VIDEORENDERER_H
#define WEBRTC_VIDEORENDERER_H

#include "Common.h"
#include "EventEmitter.h"

namespace WebRTC {
    enum MediaVideoRendererEvent {
        kMediaVideoRendererOnFrame
    };
    class MediaVideoRenderer : public Nan::ObjectWrap, public EventEmitter, public rtc::VideoSinkInterface<webrtc::VideoFrame> {
    public:
        static NAN_MODULE_INIT(Init);
        static NAN_METHOD(New);

        void SetTrack(rtc::scoped_refptr<webrtc::VideoTrackInterface> track);

   private:
        MediaVideoRenderer();
        ~MediaVideoRenderer() final;

        void On(Event *event) final;
        void OnFrame(const webrtc::VideoFrame& frame) final;

        static void GetOnFrame(v8::Local<v8::String> property, const Nan::PropertyCallbackInfo<v8::Value> &info);
        static void SetOnFrame(v8::Local<v8::String> property, v8::Local<v8::Value> value, const Nan::PropertyCallbackInfo<void> &info);

        rtc::scoped_refptr<webrtc::VideoTrackInterface> _track;
        Nan::Persistent<v8::Function> _onframe;

        static Nan::Persistent<v8::Function> constructor;
    };
};

#endif

