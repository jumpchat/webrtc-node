#ifndef WEBRTC_RTCSTATSREPORT_H
#define WEBRTC_RTCSTATSREPORT_H


#include "Common.h"
#include "EventEmitter.h"

namespace WebRTC {
class RTCStatsReport : public Nan::ObjectWrap {
public:
    static NAN_MODULE_INIT(Init);
    static v8::Local<v8::Value> New(webrtc::StatsReport* report);

private:
    ~RTCStatsReport() final;

    static NAN_METHOD(New);
    static NAN_METHOD(Names);
    static NAN_METHOD(Stat);

    static NAN_GETTER(Id);
    static NAN_GETTER(Type);
    static NAN_GETTER(Timestamp);

protected:
    webrtc::StatsReport* _report;
};
};

#endif
