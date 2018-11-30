#include "RTCRtpSender.h"
#include "MediaStreamTrack.h"

using namespace v8;
using namespace WebRTC;

static Nan::Persistent<Function> constructor;

NAN_MODULE_INIT(RTCRtpSender::Init)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    Nan::HandleScope scope;

    Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>(New);
    tpl->InstanceTemplate()->SetInternalFieldCount(1);
    tpl->SetClassName(Nan::New("RTCRtpSender").ToLocalChecked());

    Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("track").ToLocalChecked(), GetTrack);

    constructor.Reset(tpl->GetFunction());
    Nan::Set(target, Nan::New("RTCRtpSender").ToLocalChecked(), tpl->GetFunction());
}

Local<Value> RTCRtpSender::New(rtc::scoped_refptr<webrtc::RtpSenderInterface> sender)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    Nan::EscapableHandleScope scope;

    Local<Value> empty;
    Local<Function> instance = Nan::New(constructor);

    if (instance.IsEmpty() || !sender.get()) {
        return scope.Escape(Nan::Null());
    }

    Local<Object> ret = instance->NewInstance(Nan::GetCurrentContext()).ToLocalChecked();
    RTCRtpSender* self = Unwrap<RTCRtpSender>(ret);
    if (self) {
        self->_sender = sender;
        return scope.Escape(ret);
    }

    return scope.Escape(Nan::Null());
}

RTCRtpSender::RTCRtpSender()
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;
}

RTCRtpSender::~RTCRtpSender()
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;
}

NAN_METHOD(RTCRtpSender::New)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    Nan::HandleScope scope;

    if (info.IsConstructCall()) {
        RTC_LOG(LS_INFO) << "RTCRtpSender Constructor Call";
        RTCRtpSender* receiver = new RTCRtpSender();
        receiver->Wrap(info.This());

        return info.GetReturnValue().Set(info.This());
    }

    Nan::ThrowError("Internal Error");
    info.GetReturnValue().SetUndefined();
}

NAN_GETTER(RTCRtpSender::GetTrack)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    RTCRtpSender* self = Unwrap<RTCRtpSender>(info.This());

    if (self->_sender.get()) {
        info.GetReturnValue().Set(MediaStreamTrack::New(self->_sender->track()));
    } else {
        info.GetReturnValue().SetUndefined();
    }
}
