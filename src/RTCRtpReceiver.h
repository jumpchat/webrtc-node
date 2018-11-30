#ifndef WEBRTC_RTCRTPRECEIVER_H
#define WEBRTC_RTCRTPRECEIVER_H

#include "Common.h"

namespace WebRTC {
class RTCRtpReceiver: public Nan::ObjectWrap {
public:
    static NAN_MODULE_INIT(Init);
    static v8::Local<v8::Value> New(rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver);

private:
    RTCRtpReceiver();
    ~RTCRtpReceiver() final;

    static NAN_METHOD(New);
    static NAN_GETTER(GetTrack);

protected:
    rtc::scoped_refptr<webrtc::RtpReceiverInterface> _receiver;
};
};

#endif
