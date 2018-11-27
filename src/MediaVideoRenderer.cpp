#include "MediaStreamTrack.h"
#include "MediaVideoRenderer.h"

using namespace v8;
using namespace WebRTC;

Nan::Persistent<Function> MediaVideoRenderer::constructor;

MediaVideoRenderer::MediaVideoRenderer() {
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

}
MediaVideoRenderer::~MediaVideoRenderer() {
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;
    if (_track.get()) {
        _track->RemoveSink(this);
    }
}

NAN_MODULE_INIT(MediaVideoRenderer::Init) {
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    Nan::HandleScope scope;

    Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>(MediaVideoRenderer::New);
    tpl->InstanceTemplate()->SetInternalFieldCount(1);
    tpl->SetClassName(Nan::New("MediaVideoRenderer").ToLocalChecked());

    Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("onframe").ToLocalChecked(), MediaVideoRenderer::GetOnFrame, MediaVideoRenderer::SetOnFrame);

    constructor.Reset<Function>(tpl->GetFunction());
    Nan::Set(target, Nan::New("MediaVideoRenderer").ToLocalChecked(), tpl->GetFunction());
}

NAN_METHOD(MediaVideoRenderer::New) {
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    if (info.IsConstructCall()) {
        MediaVideoRenderer* self = new MediaVideoRenderer();
        self->Wrap(info.This());
        return info.GetReturnValue().Set(info.This());
    } else {
        const int argc = 2;
        Local<Value> argv[argc] = {};

        Local<Function> instance = Nan::New(MediaVideoRenderer::constructor);
        return info.GetReturnValue().Set(instance->NewInstance(Nan::GetCurrentContext(), argc, argv).ToLocalChecked());
    }
}

void MediaVideoRenderer::GetOnFrame(Local<String> property, const Nan::PropertyCallbackInfo<Value>& info)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    MediaVideoRenderer* self = Unwrap<MediaVideoRenderer>(info.Holder());
    return info.GetReturnValue().Set(Nan::New<Function>(self->_onframe));
}

void MediaVideoRenderer::SetOnFrame(Local<String> property, Local<Value> value, const Nan::PropertyCallbackInfo<void>& info)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    MediaVideoRenderer* self = Unwrap<MediaVideoRenderer>(info.Holder());

    if (!value.IsEmpty() && value->IsFunction()) {
        self->_onframe.Reset<Function>(Local<Function>::Cast(value));
    } else {
        self->_onframe.Reset();
    }
}

void MediaVideoRenderer::On(Event* event)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    Nan::HandleScope scope;
    MediaVideoRendererEvent type = event->Type<MediaVideoRendererEvent>();

    switch (type) {
        case kMediaVideoRendererOnFrame: {
            Local<Function> callback = Nan::New<Function>(_onframe);
            if (!callback.IsEmpty() && callback->IsFunction()) {
                rtc::scoped_refptr<webrtc::I420Buffer> _frame = event->Unwrap< rtc::scoped_refptr<webrtc::I420Buffer> >();
                Local<Object> container = Nan::New<Object>();
                Local<Value> argv[] = { container };
                int argc = 1;

                int width = _frame->width();
                int height = _frame->height();

                const uint8_t *y = _frame->DataY();
                const size_t y_size = _frame->StrideY()*height;
                const uint8_t *u = _frame->DataU();
                const size_t u_size = _frame->StrideU()*height/2;
                const uint8_t *v = _frame->DataV();
                const size_t v_size = _frame->StrideV()*height/2;

                // RTC_LOG(LS_INFO) << "y = " << y_size;
                // RTC_LOG(LS_INFO) << "u = " << u_size;
                // RTC_LOG(LS_INFO) << "v = " << v_size;

                // Local<ArrayBuffer> y_buffer = ArrayBuffer::New(v8::Isolate::GetCurrent(), reinterpret_cast<void *>(y), y_size);
                // Local<ArrayBuffer> u_buffer = ArrayBuffer::New(v8::Isolate::GetCurrent(), reinterpret_cast<void *>(u), u_size);
                // Local<ArrayBuffer> v_buffer = ArrayBuffer::New(v8::Isolate::GetCurrent(), reinterpret_cast<void *>(v), v_size);

                Local<ArrayBuffer> y_buffer = ArrayBuffer::New(v8::Isolate::GetCurrent(), y_size);
                Local<ArrayBuffer> u_buffer = ArrayBuffer::New(v8::Isolate::GetCurrent(), u_size);
                Local<ArrayBuffer> v_buffer = ArrayBuffer::New(v8::Isolate::GetCurrent(), v_size);
                memcpy(y_buffer->GetContents().Data(), y, y_size);
                memcpy(u_buffer->GetContents().Data(), u, u_size);
                memcpy(v_buffer->GetContents().Data(), v, v_size);

                container->Set(Nan::New("y").ToLocalChecked(), y_buffer);
                container->Set(Nan::New("u").ToLocalChecked(), u_buffer);
                container->Set(Nan::New("v").ToLocalChecked(), v_buffer);
                container->Set(Nan::New("width").ToLocalChecked(), Nan::New(width));
                container->Set(Nan::New("height").ToLocalChecked(), Nan::New(height));

                callback->Call(handle(), argc, argv);
            }
            break;
        }
    }
}

void MediaVideoRenderer::OnFrame(const webrtc::VideoFrame& frame)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    rtc::scoped_refptr<webrtc::I420Buffer> i420frame(static_cast<webrtc::I420Buffer *>(frame.video_frame_buffer().get()));
    Emit(kMediaVideoRendererOnFrame, i420frame);

}

void MediaVideoRenderer::SetTrack(rtc::scoped_refptr<webrtc::VideoTrackInterface> track) {
    _track = track;
}
