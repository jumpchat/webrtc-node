#ifndef WEBRTC_RTCRTPTRANSCEIVER_H
#define WEBRTC_RTCRTPTRANSCEIVER_H

#include "Common.h"

namespace WebRTC {
class RTCRtpTransceiver : public Nan::ObjectWrap {
public:
    static NAN_MODULE_INIT(Init);
    static v8::Local<v8::Value> New(rtc::scoped_refptr<webrtc::RtpTransceiverInterface> transceiver);

private:
    RTCRtpTransceiver();
    ~RTCRtpTransceiver() final;

    static NAN_METHOD(New);

    static NAN_GETTER(GetCurrentDirection);
    static NAN_GETTER(GetDirection);
    static NAN_GETTER(GetMid);
    static NAN_GETTER(GetReceiver);
    static NAN_GETTER(GetSender);
    static NAN_GETTER(GetStopped);

    static NAN_SETTER(SetDirection);
    static NAN_SETTER(SetStopped);

protected:
    rtc::scoped_refptr<webrtc::RtpTransceiverInterface> _transceiver;
    static std::string TransceiverDirectionToString(webrtc::RtpTransceiverDirection dir);
    static webrtc::RtpTransceiverDirection TransceiverDirectionFromString(const std::string &str);
};
};

#endif
