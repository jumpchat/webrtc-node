#ifndef WEBRTC_RTCSTATSRESPONSE_H
#define WEBRTC_RTCSTATSRESPONSE_H

#include "Common.h"
#include "EventEmitter.h"

namespace WebRTC {
class RTCStatsResponse : public Nan::ObjectWrap {
public:
    static NAN_MODULE_INIT(Init);
    static v8::Local<v8::Value> New(const webrtc::StatsReports& reports);

private:
    ~RTCStatsResponse() final;

    static NAN_METHOD(New);
    static NAN_METHOD(Result);

protected:
    webrtc::StatsReports _reports;
};
};

#endif
