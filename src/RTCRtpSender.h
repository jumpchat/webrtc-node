#ifndef WEBRTC_RTCRTPSENDER_H
#define WEBRTC_RTCRTPSENDER_H

#include "Common.h"

namespace WebRTC {
class RTCRtpSender: public Nan::ObjectWrap {
public:
    static NAN_MODULE_INIT(Init);
    static v8::Local<v8::Value> New(rtc::scoped_refptr<webrtc::RtpSenderInterface> sender);

private:
    RTCRtpSender();
    ~RTCRtpSender() final;

    static NAN_METHOD(New);
    static NAN_GETTER(GetTrack);

protected:
    rtc::scoped_refptr<webrtc::RtpSenderInterface> _sender;
};
};

#endif
