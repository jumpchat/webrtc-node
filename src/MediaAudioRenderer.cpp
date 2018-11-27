#include "MediaStreamTrack.h"
#include "MediaAudioRenderer.h"

using namespace v8;
using namespace WebRTC;

static Nan::Persistent<v8::Function> constructor;

class AudioData : public rtc::RefCountInterface {
public:
    webrtc::DataBuffer audio_data;
    int bits_per_sample;
    int sample_rate;
    size_t number_of_channels;
    size_t number_of_frames;

    AudioData(const void *_audio_data,
        int _bits_per_sample,
        int _sample_rate,
        size_t _number_of_channels,
        size_t _number_of_frames) :
        audio_data(rtc::CopyOnWriteBuffer((uint8_t *)_audio_data, _bits_per_sample*_number_of_channels*_number_of_frames/8), true),
        bits_per_sample(_bits_per_sample),
        sample_rate(_sample_rate),
        number_of_channels(_number_of_channels),
        number_of_frames(_number_of_frames)
    {

    }

};

MediaAudioRenderer::MediaAudioRenderer() :
    _resampler(new webrtc::Resampler())
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

}
MediaAudioRenderer::~MediaAudioRenderer() {
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;
    if (_track.get()) {
        _track->RemoveSink(this);
    }
}

NAN_MODULE_INIT(MediaAudioRenderer::Init) {
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    Nan::HandleScope scope;

    Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>(MediaAudioRenderer::New);
    tpl->InstanceTemplate()->SetInternalFieldCount(1);
    tpl->SetClassName(Nan::New("MediaAudioRenderer").ToLocalChecked());

    Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("ondata").ToLocalChecked(), GetOnData, SetOnData);

    constructor.Reset<Function>(tpl->GetFunction());
    Nan::Set(target, Nan::New("MediaAudioRenderer").ToLocalChecked(), tpl->GetFunction());
}

NAN_METHOD(MediaAudioRenderer::New) {
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    if (info.IsConstructCall()) {
        MediaAudioRenderer* self = new MediaAudioRenderer();
        self->Wrap(info.This());
        return info.GetReturnValue().Set(info.This());
    } else {
        const int argc = 2;
        Local<Value> argv[argc] = {};

        Local<Function> instance = Nan::New(constructor);
        return info.GetReturnValue().Set(instance->NewInstance(Nan::GetCurrentContext(), argc, argv).ToLocalChecked());
    }
}

void MediaAudioRenderer::GetOnData(Local<String> property, const Nan::PropertyCallbackInfo<Value>& info)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    MediaAudioRenderer* self = Unwrap<MediaAudioRenderer>(info.Holder());
    return info.GetReturnValue().Set(Nan::New<Function>(self->_ondata));
}

void MediaAudioRenderer::SetOnData(Local<String> property, Local<Value> value, const Nan::PropertyCallbackInfo<void>& info)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    MediaAudioRenderer* self = Unwrap<MediaAudioRenderer>(info.Holder());

    if (!value.IsEmpty() && value->IsFunction()) {
        self->_ondata.Reset<Function>(Local<Function>::Cast(value));
    } else {
        self->_ondata.Reset();
    }
}

void MediaAudioRenderer::On(Event* event)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    Nan::HandleScope scope;
    MediaAudioRendererEvent type = event->Type<MediaAudioRendererEvent>();

    switch (type) {
        case kMediaAudioRendererOnData: {
            Local<Function> callback = Nan::New<Function>(_ondata);
            if (!callback.IsEmpty() && callback->IsFunction()) {
                rtc::scoped_refptr<AudioData> _data = event->Unwrap< rtc::scoped_refptr<AudioData> >();
                Local<Object> container = Nan::New<Object>();
                Local<Value> argv[] = { container };
                int argc = 1;

                // convert to 48000khz
                switch (_data->sample_rate) {
                    case 48000: {
                        // divide by 8 to change bits to bytes
                        size_t data_size = (_data->bits_per_sample*_data->number_of_frames*_data->number_of_channels) >> 3;
                        Local<ArrayBuffer> data = ArrayBuffer::New(v8::Isolate::GetCurrent(), data_size);
                        memcpy(data->GetContents().Data(), _data->audio_data.data.data(), data_size);

                        container->Set(Nan::New("data").ToLocalChecked(), data);
                        container->Set(Nan::New("bits_per_sample").ToLocalChecked(), Nan::New(_data->bits_per_sample));
                        container->Set(Nan::New("sample_rate").ToLocalChecked(), Nan::New(_data->sample_rate));
                        container->Set(Nan::New("number_of_channels").ToLocalChecked(), Nan::New((int)_data->number_of_channels));
                        container->Set(Nan::New("number_of_frames").ToLocalChecked(), Nan::New((int)_data->number_of_frames));
                        break;
                    }
                    default: {
                        size_t out_len = 0;
                        size_t data_size = (_data->bits_per_sample*_data->number_of_frames*_data->number_of_channels) >> 3;
                        Local<ArrayBuffer> data = ArrayBuffer::New(v8::Isolate::GetCurrent(), data_size);
                        _resampler->ResetIfNeeded(_data->sample_rate, 48000, _data->number_of_channels);
                        _resampler->Push((int16_t *)_data->audio_data.data.data(), _data->audio_data.data.size(),
                            (int16_t *)data->GetContents().Data(), data_size, out_len);

                        container->Set(Nan::New("data").ToLocalChecked(), data);
                        container->Set(Nan::New("bits_per_sample").ToLocalChecked(), Nan::New(_data->bits_per_sample));
                        container->Set(Nan::New("sample_rate").ToLocalChecked(), Nan::New(48000));
                        container->Set(Nan::New("number_of_channels").ToLocalChecked(), Nan::New((int)_data->number_of_channels));
                        container->Set(Nan::New("number_of_frames").ToLocalChecked(), Nan::New((int)_data->number_of_frames));
                        break;
                    }
                }
                callback->Call(handle(), argc, argv);
            }
            break;
        }
    }
}

void MediaAudioRenderer::OnData(const void* audio_data,
            int bits_per_sample,
            int sample_rate,
            size_t number_of_channels,
            size_t number_of_frames)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    rtc::scoped_refptr<AudioData> audioData(new rtc::RefCountedObject<AudioData>(audio_data, bits_per_sample, sample_rate,
        number_of_channels, number_of_frames));
    Emit(kMediaAudioRendererOnData, audioData);

}

void MediaAudioRenderer::SetTrack(rtc::scoped_refptr<webrtc::AudioTrackInterface> track) {
    _track = track;
}
