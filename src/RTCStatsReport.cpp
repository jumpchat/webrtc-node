#include "RTCStatsReport.h"

using namespace v8;
using namespace WebRTC;

static Nan::Persistent<Function> constructor;

RTCStatsReport::~RTCStatsReport()
{
}

NAN_MODULE_INIT(RTCStatsReport::Init)
{
    Nan::HandleScope scope;

    Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>(New);
    tpl->InstanceTemplate()->SetInternalFieldCount(1);
    tpl->SetClassName(Nan::New("RTCStatsReport").ToLocalChecked());

    Nan::SetPrototypeMethod(tpl, "names", Names);
    Nan::SetPrototypeMethod(tpl, "stat", Stat);

    Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("id").ToLocalChecked(), Id);
    Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("type").ToLocalChecked(), Type);
    Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("timestamp").ToLocalChecked(), Timestamp);

    constructor.Reset(tpl->GetFunction());
};

Local<Value> RTCStatsReport::New(webrtc::StatsReport* report)
{
    Nan::EscapableHandleScope scope;
    Local<Function> instance = Nan::New(constructor);

    if (instance.IsEmpty()) {
        return scope.Escape(Nan::Null());
    }

    Local<Object> ret = instance->NewInstance(Nan::GetCurrentContext()).ToLocalChecked();
    RTCStatsReport* stats = Nan::ObjectWrap::Unwrap<RTCStatsReport>(ret);

    if (stats) {
        stats->_report = report;
    }

    return scope.Escape(ret);
}

NAN_METHOD(RTCStatsReport::New)
{
    if (info.IsConstructCall()) {
        RTCStatsReport* report = new RTCStatsReport();
        report->Wrap(info.This());
        return info.GetReturnValue().Set(info.This());
    }

    Nan::ThrowError("Internal Error");
    info.GetReturnValue().SetUndefined();
}

NAN_METHOD(RTCStatsReport::Names)
{
    RTCStatsReport* stats = Nan::ObjectWrap::Unwrap<RTCStatsReport>(info.This());
    Local<Array> list = Nan::New<Array>();
    unsigned int index = 0;

    if (stats && stats->_report) {
        webrtc::StatsReport::Values values = stats->_report->values();
        for (webrtc::StatsReport::Values::iterator it = values.begin(); it != values.end(); it++) {
            webrtc::StatsReport::ValuePtr value = values[it->first];
            list->Set(index, Nan::New(value->display_name()).ToLocalChecked());
            index++;
        }
    }

    return info.GetReturnValue().Set(list);
}

NAN_METHOD(RTCStatsReport::Stat)
{
    RTCStatsReport* stats = Nan::ObjectWrap::Unwrap<RTCStatsReport>(info.This());
    webrtc::StatsReport::Values values = stats->_report->values();

    if (info.Length() >= 1 && info[0]->IsString()) {
        String::Utf8Value entry_value(Isolate::GetCurrent(), info[0]->ToString());
        std::string entry(*entry_value);

        for (webrtc::StatsReport::Values::iterator it = values.begin(); it != values.end(); it++) {
            webrtc::StatsReport::ValuePtr value = values[it->first];

            if (!entry.compare(value->display_name())) {
                switch (value->type()) {
                case webrtc::StatsReport::Value::kInt:
                    return info.GetReturnValue().Set(Nan::New(value->int_val()));

                    break;
                case webrtc::StatsReport::Value::kInt64:
                    return info.GetReturnValue().Set(Nan::New(static_cast<int32_t>(value->int64_val())));

                    break;
                case webrtc::StatsReport::Value::kFloat:
                    return info.GetReturnValue().Set(Nan::New(value->float_val()));

                    break;
                case webrtc::StatsReport::Value::kString:
                    return info.GetReturnValue().Set(Nan::New(value->string_val().c_str()).ToLocalChecked());

                    break;
                case webrtc::StatsReport::Value::kStaticString:
                    return info.GetReturnValue().Set(Nan::New(value->static_string_val()).ToLocalChecked());

                    break;
                case webrtc::StatsReport::Value::kBool:
                    return info.GetReturnValue().Set(Nan::New(value->bool_val()));

                    break;
                case webrtc::StatsReport::Value::kId:
                    return info.GetReturnValue().Set(Nan::New(value->ToString().c_str()).ToLocalChecked());

                    break;
                }
            }
        }
    }

    info.GetReturnValue().SetUndefined();
}

NAN_GETTER(RTCStatsReport::Id)
{
    RTCStatsReport* stats = Nan::ObjectWrap::Unwrap<RTCStatsReport>(info.This());
    std::string id(stats->_report->id()->ToString());
    return info.GetReturnValue().Set(Nan::New(id.c_str()).ToLocalChecked());
}

NAN_GETTER(RTCStatsReport::Type)
{
    RTCStatsReport* stats = Nan::ObjectWrap::Unwrap<RTCStatsReport>(info.This());
    return info.GetReturnValue().Set(Nan::New(stats->_report->TypeToString()).ToLocalChecked());
}

NAN_GETTER(RTCStatsReport::Timestamp)
{
    RTCStatsReport* stats = Nan::ObjectWrap::Unwrap<RTCStatsReport>(info.This());
    return info.GetReturnValue().Set(Nan::New(stats->_report->timestamp()));
}
