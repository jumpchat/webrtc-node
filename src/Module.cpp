#include "Common.h"

// #include "BackTrace.h"
// #include "DataChannel.h"
#include "GetSources.h"
#include "MediaDevices.h"
#include "MediaStream.h"
#include "MediaStreamTrack.h"
#include "MediaVideoRenderer.h"
#include "MediaAudioRenderer.h"
// #include "PeerConnection.h"
#include "Platform.h"
// #include "Stats.h"
#include "DesktopCapturer.h"

using namespace v8;

void SetDebug(const Nan::FunctionCallbackInfo<Value>& info)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    if (info.Length() && !info[0].IsEmpty()) {
        if (info[0]->IsTrue()) {
            rtc::LogMessage::LogToDebug(rtc::LS_VERBOSE);
            RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__ << " " << true;
        } else if (info[0]->IsFalse()) {
            rtc::LogMessage::LogToDebug(rtc::LS_NONE);
            RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__ << " " << false;
        } else if (info[0]->IsString()) {
            v8::String::Utf8Value idValue(info[0]->ToString());
            std::string id(*idValue);
            if (id == "verbose") {
                rtc::LogMessage::LogToDebug(rtc::LS_VERBOSE);
            } else if (id == "info") {
                rtc::LogMessage::LogToDebug(rtc::LS_INFO);
            } else if (id == "warning") {
                rtc::LogMessage::LogToDebug(rtc::LS_WARNING);
            } else if (id == "error") {
                rtc::LogMessage::LogToDebug(rtc::LS_ERROR);
            } else {
                rtc::LogMessage::LogToDebug(rtc::LS_NONE);
            }
            RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__ << " " << id;
        }
    }

    info.GetReturnValue().SetUndefined();
}

void RTCGarbageCollect(const Nan::FunctionCallbackInfo<Value>& info)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    Nan::LowMemoryNotification();
    info.GetReturnValue().SetUndefined();
}

void RTCIceCandidate(const Nan::FunctionCallbackInfo<Value>& info)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    if (info.Length() == 1 && info[0]->IsObject() && info.IsConstructCall()) {
        Local<Object> arg = info[0]->ToObject();
        Local<Object> retval = Nan::New<Object>();

        retval->Set(Nan::New("candidate").ToLocalChecked(), arg->Get(Nan::New("candidate").ToLocalChecked()));
        retval->Set(Nan::New("sdpMLineIndex").ToLocalChecked(), arg->Get(Nan::New("sdpMLineIndex").ToLocalChecked()));
        retval->Set(Nan::New("sdpMid").ToLocalChecked(), arg->Get(Nan::New("sdpMid").ToLocalChecked()));

        return info.GetReturnValue().Set(retval);
    } else {
        return info.GetReturnValue().Set(info[0]);
    }
}

void RTCSessionDescription(const Nan::FunctionCallbackInfo<Value>& info)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    if (info.Length() == 1 && info[0]->IsObject() && info.IsConstructCall()) {
        Local<Object> arg = info[0]->ToObject();
        Local<Object> retval = Nan::New<Object>();

        retval->Set(Nan::New("type").ToLocalChecked(), arg->Get(Nan::New("type").ToLocalChecked()));
        retval->Set(Nan::New("sdp").ToLocalChecked(), arg->Get(Nan::New("sdp").ToLocalChecked()));

        return info.GetReturnValue().Set(retval);
    } else {
        return info.GetReturnValue().Set(info[0]);
    }
}

void WebrtcModuleDispose(void* arg)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    WebRTC::Platform::Dispose();
}

void WebrtcModuleInit(Handle<Object> exports)
{
    rtc::LogMessage::LogToDebug(rtc::LS_NONE);

    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    Nan::HandleScope scope;

    WebRTC::Platform::Init();
    // WebRTC::RTCStatsResponse::Init();
    // WebRTC::RTCStatsReport::Init();
    // WebRTC::PeerConnection::Init(exports);
    // WebRTC::DataChannel::Init();
    WebRTC::GetSources::Init(exports);
    WebRTC::MediaDevices::Init(exports);
    WebRTC::MediaStream::Init(exports);
    WebRTC::MediaStreamTrack::Init(exports);
    WebRTC::MediaAudioRenderer::Init(exports);
    WebRTC::MediaVideoRenderer::Init(exports);
    WebRTC::DesktopCapturer::Init(exports);

    exports->Set(Nan::New("RTCGarbageCollect").ToLocalChecked(), Nan::New<FunctionTemplate>(RTCGarbageCollect)->GetFunction());
    exports->Set(Nan::New("RTCIceCandidate").ToLocalChecked(), Nan::New<FunctionTemplate>(RTCIceCandidate)->GetFunction());
    exports->Set(Nan::New("RTCSessionDescription").ToLocalChecked(), Nan::New<FunctionTemplate>(RTCSessionDescription)->GetFunction());
    exports->Set(Nan::New("setDebug").ToLocalChecked(), Nan::New<FunctionTemplate>(SetDebug)->GetFunction());

    node::AtExit(WebrtcModuleDispose);
}

NODE_MODULE(webrtc, WebrtcModuleInit)