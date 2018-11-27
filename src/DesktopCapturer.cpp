#include "DesktopCapturer.h"

using namespace v8;
using namespace WebRTC;

Nan::Persistent<Function> DesktopCapturer::constructor;

NAN_MODULE_INIT(DesktopCapturer::Init)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    Nan::HandleScope scope;

    Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>(DesktopCapturer::New);
    tpl->InstanceTemplate()->SetInternalFieldCount(1);
    tpl->SetClassName(Nan::New("DesktopCapturer").ToLocalChecked());

    Nan::SetPrototypeMethod(tpl, "start", DesktopCapturer::Start);
    Nan::SetPrototypeMethod(tpl, "stop", DesktopCapturer::Stop);
    Nan::SetPrototypeMethod(tpl, "getSourceList", DesktopCapturer::GetSourceList);
    Nan::SetPrototypeMethod(tpl, "selectSource", DesktopCapturer::SelectSource);
    Nan::SetPrototypeMethod(tpl, "captureFrame", DesktopCapturer::CaptureFrame);
    Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("onframe").ToLocalChecked(), GetOnFrame, SetOnFrame);

    constructor.Reset<Function>(tpl->GetFunction());
    Nan::Set(target, Nan::New("DesktopCapturer").ToLocalChecked(), tpl->GetFunction());
}

DesktopCapturer::DesktopCapturer()
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;
}

DesktopCapturer::~DesktopCapturer()
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;
}

NAN_METHOD(DesktopCapturer::New)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    Nan::HandleScope scope;

    if (info.IsConstructCall()) {
        DesktopCapturer* capturer = new class DesktopCapturer();
        webrtc::DesktopCaptureOptions options = webrtc::DesktopCaptureOptions::CreateDefault();
        if (info.Length() && info[0]->IsObject()) {
#ifdef WIN32
            Local<Object> localOption = Local<Object>::Cast(info[0]);
            Local<Value> allow_directx_capturer_value = localOption->Get(Nan::New("allow_directx_capturer").ToLocalChecked());
            Local<Int32> allow_directx_capturer(allow_directx_capturer_value->ToInt32(Nan::GetCurrentContext()).ToLocalChecked());
            RTC_LOG(LS_INFO) << "allow_directx_capturer = " << allow_directx_capturer->Value();
            options.set_allow_directx_capturer(allow_directx_capturer->Value());
#endif
        }
        capturer->_capturer = webrtc::DesktopCapturer::CreateScreenCapturer(options);

        capturer->Wrap(info.This());
        return info.GetReturnValue().Set(info.This());
    }

    Nan::ThrowError("Internal Error");
    info.GetReturnValue().SetUndefined();
}

NAN_METHOD(DesktopCapturer::Start)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;
    DesktopCapturer* self = Unwrap<DesktopCapturer>(info.Holder());
    self->_capturer->Start(self);
}

NAN_METHOD(DesktopCapturer::Stop)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;
    DesktopCapturer* self = Unwrap<DesktopCapturer>(info.Holder());
    self->_capturer = NULL;
}

NAN_METHOD(DesktopCapturer::CaptureFrame)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;
    DesktopCapturer* self = Unwrap<DesktopCapturer>(info.Holder());
    self->_capturer->CaptureFrame();
}

NAN_METHOD(DesktopCapturer::GetSourceList)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;
    DesktopCapturer* self = Unwrap<DesktopCapturer>(info.Holder());
    webrtc::DesktopCapturer::SourceList sourceList;
    bool result = self->_capturer->GetSourceList(&sourceList);
    Local<Array> list = Nan::New<Array>();
    if (result) {
        for (size_t i = 0; i < sourceList.size(); i++) {
            webrtc::DesktopCapturer::Source src = sourceList[i];
            Local<Object> source = Nan::New<Object>();
            source->Set(Nan::New("id").ToLocalChecked(), Nan::New((uint32_t)src.id));
            source->Set(Nan::New("title").ToLocalChecked(), Nan::New(src.title).ToLocalChecked());
            list->Set(i, source);
        }
    }

    info.GetReturnValue().Set(list);
}

NAN_METHOD(DesktopCapturer::SelectSource)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;
    DesktopCapturer* self = Unwrap<DesktopCapturer>(info.Holder());

    if (info.Length() == 1 && info[0]->IsInt32()) {
        Local<Int32> id(info[0]->ToInt32(Nan::GetCurrentContext()).ToLocalChecked());
        RTC_LOG(LS_INFO) << "SelectSource " << id->Value();
        self->_capturer->SelectSource(id->Value());
    }
}

NAN_GETTER(DesktopCapturer::GetOnFrame)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    DesktopCapturer* self = Unwrap<DesktopCapturer>(info.Holder());
    return info.GetReturnValue().Set(Nan::New<Function>(self->_onframe));
}

NAN_SETTER(DesktopCapturer::SetOnFrame)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    DesktopCapturer* self = Unwrap<DesktopCapturer>(info.Holder());

    if (!value.IsEmpty() && value->IsFunction()) {
        self->_onframe.Reset<Function>(Local<Function>::Cast(value));
    } else {
        self->_onframe.Reset();
    }
}

void DesktopCapturer::On(Event* event)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;
}

void DesktopCapturer::OnCaptureResult(webrtc::DesktopCapturer::Result result, std::unique_ptr<webrtc::DesktopFrame> frame)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    Local<Function> callback = Nan::New<Function>(_onframe);
    RTC_LOG(LS_INFO) << "callback.IsEmpty() " << callback.IsEmpty();
    if (!callback.IsEmpty() && callback->IsFunction()) {
        Local<Object> container = Nan::New<Object>();
        Local<Value> argv[] = { container };
        int argc = 1;

        size_t frameSize = frame->size().height()*frame->stride();
        RTC_LOG(LS_INFO) << "size = " << frame->size().width()  << "x" << frame->size().height();
        RTC_LOG(LS_INFO) << "stride = " << frame->stride();
        RTC_LOG(LS_INFO) << "frameSize = " << frameSize;

        Local<ArrayBuffer> data = ArrayBuffer::New(v8::Isolate::GetCurrent(), frameSize);
        memcpy(data->GetContents().Data(), frame->data(), frameSize);

        container->Set(Nan::New("data").ToLocalChecked(), data);
        container->Set(Nan::New("stride").ToLocalChecked(), Nan::New(frame->stride()));
        container->Set(Nan::New("width").ToLocalChecked(), Nan::New(frame->size().width()));
        container->Set(Nan::New("height").ToLocalChecked(), Nan::New(frame->size().height()));

        callback->Call(handle(), argc, argv);
    }
}
