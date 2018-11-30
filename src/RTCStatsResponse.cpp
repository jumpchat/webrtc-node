#include "RTCStatsResponse.h"
#include "RTCStatsReport.h"

using namespace v8;
using namespace WebRTC;

static Nan::Persistent<Function> constructor;

RTCStatsResponse::~RTCStatsResponse()
{
}

NAN_MODULE_INIT(RTCStatsResponse::Init)
{
    Nan::HandleScope scope;

    Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>(RTCStatsResponse::New);
    tpl->InstanceTemplate()->SetInternalFieldCount(1);
    tpl->SetClassName(Nan::New("RTCStatsResponse").ToLocalChecked());

    Nan::SetPrototypeMethod(tpl, "result", Result);

    constructor.Reset(tpl->GetFunction());
}

NAN_METHOD(RTCStatsResponse::New)
{
    if (info.IsConstructCall()) {
        RTCStatsResponse* response = new RTCStatsResponse();
        response->Wrap(info.This());
        return info.GetReturnValue().Set(info.This());
    }

    Nan::ThrowError("Internal Error");
    info.GetReturnValue().SetUndefined();
}

Local<Value> RTCStatsResponse::New(const webrtc::StatsReports& reports)
{
    Nan::EscapableHandleScope scope;
    Local<Function> instance = Nan::New(constructor);

    if (instance.IsEmpty()) {
        return scope.Escape(Nan::Null());
    }

    Local<Object> ret = instance->NewInstance(Nan::GetCurrentContext()).ToLocalChecked();
    RTCStatsResponse* response = Nan::ObjectWrap::Unwrap<RTCStatsResponse>(ret);

    response->_reports = reports;

    return scope.Escape(ret);
}

NAN_METHOD(RTCStatsResponse::Result)
{
    RTCStatsResponse* response = Nan::ObjectWrap::Unwrap<RTCStatsResponse>(info.This());
    Local<Array> list = Nan::New<Array>(response->_reports.size());

    for (unsigned int index = 0; index < response->_reports.size(); index++) {
        list->Set(index, RTCStatsReport::New(const_cast<webrtc::StatsReport*>(response->_reports.at(index))));
    }

    return info.GetReturnValue().Set(list);
}
