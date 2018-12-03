#include "RTCRtpTransceiver.h"



using namespace v8;
using namespace WebRTC;

static Nan::Persistent<Function> constructor;

NAN_MODULE_INIT(RTCRtpTransceiver::Init)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    Nan::HandleScope scope;

    Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>(New);
    tpl->InstanceTemplate()->SetInternalFieldCount(1);
    tpl->SetClassName(Nan::New("RTCRtpTransceiver").ToLocalChecked());

    Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("currentDirection").ToLocalChecked(), GetCurrentDirection);
    Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("direction").ToLocalChecked(), GetDirection, SetDirection);
    Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("mid").ToLocalChecked(), GetMid);
    Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("receiver").ToLocalChecked(), GetReceiver);
    Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("sender").ToLocalChecked(), GetSender);
    Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("stopped").ToLocalChecked(), GetStopped, SetStopped);

    constructor.Reset(tpl->GetFunction());
    Nan::Set(target, Nan::New("RTCRtpTransceiver").ToLocalChecked(), tpl->GetFunction());
}

Local<Value> RTCRtpTransceiver::New(rtc::scoped_refptr<webrtc::RtpTransceiverInterface> transceiver)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    Nan::EscapableHandleScope scope;

    Local<Value> empty;
    Local<Function> instance = Nan::New(constructor);

    if (instance.IsEmpty() || !transceiver.get()) {
        return scope.Escape(Nan::Null());
    }

    Local<Object> ret = instance->NewInstance(Nan::GetCurrentContext()).ToLocalChecked();
    RTCRtpTransceiver* self = Unwrap<RTCRtpTransceiver>(ret);
    if (self) {
        self->_transceiver = transceiver;
        return scope.Escape(ret);
    }

    return scope.Escape(Nan::Null());
}

RTCRtpTransceiver::RTCRtpTransceiver()
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;
}

RTCRtpTransceiver::~RTCRtpTransceiver()
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;
}

NAN_METHOD(RTCRtpTransceiver::New)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    Nan::HandleScope scope;

    if (info.IsConstructCall()) {
        RTC_LOG(LS_INFO) << "RTCRtpTransceiver Constructor Call";
        RTCRtpTransceiver* transceiver = new RTCRtpTransceiver();
        transceiver->Wrap(info.This());

        return info.GetReturnValue().Set(info.This());
    }

    Nan::ThrowError("Internal Error");
    info.GetReturnValue().SetUndefined();
}

NAN_GETTER(RTCRtpTransceiver::GetCurrentDirection)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    RTCRtpTransceiver* self = Unwrap<RTCRtpTransceiver>(info.This());
    std::string direction_str;

    if (self->_transceiver.get()) {
        absl::optional<webrtc::RtpTransceiverDirection> direction_opt = self->_transceiver->current_direction();
        direction_str = TransceiverDirectionToString(*direction_opt);
    }

    info.GetReturnValue().Set(Nan::New(direction_str.c_str()).ToLocalChecked());
}

NAN_GETTER(RTCRtpTransceiver::GetDirection)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    RTCRtpTransceiver* self = Unwrap<RTCRtpTransceiver>(info.This());
    std::string direction_str;

    if (self->_transceiver.get()) {
        absl::optional<webrtc::RtpTransceiverDirection> direction_opt = self->_transceiver->direction();
        direction_str = TransceiverDirectionToString(*direction_opt);
    }

    info.GetReturnValue().Set(Nan::New(direction_str.c_str()).ToLocalChecked());
}

NAN_GETTER(RTCRtpTransceiver::GetMid)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    RTCRtpTransceiver* self = Unwrap<RTCRtpTransceiver>(info.This());
    std::string mid_str;

    if (self->_transceiver.get()) {
        mid_str = *self->_transceiver->mid();
    }

    info.GetReturnValue().Set(Nan::New(mid_str.c_str()).ToLocalChecked());

}

NAN_GETTER(RTCRtpTransceiver::GetReceiver)
{

}

NAN_GETTER(RTCRtpTransceiver::GetSender)
{

}

NAN_GETTER(RTCRtpTransceiver::GetStopped)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    RTCRtpTransceiver* self = Unwrap<RTCRtpTransceiver>(info.This());
    bool stopped = false;

    if (self->_transceiver.get()) {
        stopped = self->_transceiver->stopped();
    }

    info.GetReturnValue().Set(Nan::New(stopped));

}

NAN_SETTER(RTCRtpTransceiver::SetDirection)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    RTCRtpTransceiver* self = Nan::ObjectWrap::Unwrap<RTCRtpTransceiver>(info.Holder());

    if (!value.IsEmpty() && value->IsString()) {
        v8::String::Utf8Value direction(Isolate::GetCurrent(), value->ToString());
        self->_transceiver->SetDirection(RTCRtpTransceiver::TransceiverDirectionFromString(*direction));
    } else {
        RTC_LOG(LS_INFO) << "Setting unknown direction";
    }
}

NAN_SETTER(RTCRtpTransceiver::SetStopped)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    RTCRtpTransceiver* self = Nan::ObjectWrap::Unwrap<RTCRtpTransceiver>(info.Holder());

    if (!value.IsEmpty() && value->IsTrue()) {
        self->_transceiver->Stop();
    } else {
        RTC_LOG(LS_INFO) << "Unable to setup stopped";
    }
}


std::string RTCRtpTransceiver::TransceiverDirectionToString(webrtc::RtpTransceiverDirection direction)
{
    std::string direction_str;
    switch (direction) {
        case webrtc::RtpTransceiverDirection::kSendRecv:
            direction_str = "sendrecv";
            break;
        case webrtc::RtpTransceiverDirection::kSendOnly:
            direction_str = "sendonly";
            break;
        case webrtc::RtpTransceiverDirection::kRecvOnly:
            direction_str = "recvonly";
            break;
        case webrtc::RtpTransceiverDirection::kInactive:
            direction_str = "inactive";
            break;

    }
    return direction_str;
}

webrtc::RtpTransceiverDirection RTCRtpTransceiver::TransceiverDirectionFromString(const std::string &str)
{
    if (str == "sendrecv")
        return webrtc::RtpTransceiverDirection::kSendRecv;
    else if (str == "sendonly")
        return webrtc::RtpTransceiverDirection::kSendOnly;
    else if (str == "recvonly")
        return webrtc::RtpTransceiverDirection::kRecvOnly;
    else if (str == "inactive")
        return webrtc::RtpTransceiverDirection::kInactive;
    else
        return webrtc::RtpTransceiverDirection::kInactive;
}