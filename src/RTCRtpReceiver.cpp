#include "RTCRtpReceiver.h"
#include "MediaStreamTrack.h"

using namespace v8;
using namespace WebRTC;

static Nan::Persistent<Function> constructor;

NAN_MODULE_INIT(RTCRtpReceiver::Init)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    Nan::HandleScope scope;

    Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>(New);
    tpl->InstanceTemplate()->SetInternalFieldCount(1);
    tpl->SetClassName(Nan::New("RTCRtpReceiver").ToLocalChecked());

    Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("track").ToLocalChecked(), GetTrack);

    constructor.Reset(tpl->GetFunction());
    Nan::Set(target, Nan::New("RTCRtpReceiver").ToLocalChecked(), tpl->GetFunction());
}

Local<Value> RTCRtpReceiver::New(rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    Nan::EscapableHandleScope scope;

    Local<Value> empty;
    Local<Function> instance = Nan::New(constructor);

    if (instance.IsEmpty() || !receiver.get()) {
        return scope.Escape(Nan::Null());
    }

    Local<Object> ret = instance->NewInstance(Nan::GetCurrentContext()).ToLocalChecked();
    RTCRtpReceiver* self = Unwrap<RTCRtpReceiver>(ret);
    if (self) {
        self->_receiver = receiver;
        return scope.Escape(ret);
    }

    return scope.Escape(Nan::Null());
}

RTCRtpReceiver::RTCRtpReceiver()
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;
}

RTCRtpReceiver::~RTCRtpReceiver()
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;
}

NAN_METHOD(RTCRtpReceiver::New)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    Nan::HandleScope scope;

    if (info.IsConstructCall()) {
        RTC_LOG(LS_INFO) << "RTCRtpReceiver Constructor Call";
        RTCRtpReceiver* receiver = new RTCRtpReceiver();
        receiver->Wrap(info.This());

        return info.GetReturnValue().Set(info.This());
    }

    Nan::ThrowError("Internal Error");
    info.GetReturnValue().SetUndefined();
}

NAN_GETTER(RTCRtpReceiver::GetTrack)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    RTCRtpReceiver* self = Unwrap<RTCRtpReceiver>(info.This());

    if (self->_receiver.get()) {
        info.GetReturnValue().Set(MediaStreamTrack::New(self->_receiver->track()));
    } else {
        info.GetReturnValue().SetUndefined();
    }
}
