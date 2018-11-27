
#ifndef WEBRTC_DESKTOPCAPTURER_H
#define WEBRTC_DESKTOPCAPTURER_H

#include "Common.h"
#include "EventEmitter.h"

namespace WebRTC {
class DesktopCapturer : public Nan::ObjectWrap,
                      public EventEmitter,
                      public webrtc::DesktopCapturer::Callback {
public:
    static NAN_MODULE_INIT(Init);

private:
    DesktopCapturer();
    ~DesktopCapturer() final;

    static NAN_METHOD(New);
    static NAN_METHOD(Start);
    static NAN_METHOD(Stop);
    static NAN_METHOD(CaptureFrame);
    static NAN_METHOD(GetSourceList);
    static NAN_METHOD(SelectSource);
    static NAN_GETTER(GetOnFrame);
    static NAN_SETTER(SetOnFrame);

    virtual void OnCaptureResult(webrtc::DesktopCapturer::Result result, std::unique_ptr<webrtc::DesktopFrame> frame);
    void On(Event* event) final;

protected:
    std::unique_ptr<webrtc::DesktopCapturer> _capturer;
    Nan::Persistent<v8::Function> _onframe;

    static Nan::Persistent<v8::Function> constructor;
};
};

#endif
