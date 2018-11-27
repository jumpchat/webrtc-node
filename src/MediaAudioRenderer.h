
#ifndef WEBRTC_AUDIORENDERER_H
#define WEBRTC_AUDIORENDERER_H

#include "Common.h"
#include "EventEmitter.h"

#include "common_audio/resampler/include/resampler.h"

namespace WebRTC {
    enum MediaAudioRendererEvent {
        kMediaAudioRendererOnData
    };
    class MediaAudioRenderer : public Nan::ObjectWrap, public EventEmitter, public webrtc::AudioTrackSinkInterface {
    public:
        static NAN_MODULE_INIT(Init);
        static NAN_METHOD(New);

        void SetTrack(rtc::scoped_refptr<webrtc::AudioTrackInterface> track);

   private:
        MediaAudioRenderer();
        ~MediaAudioRenderer() final;

        void On(Event *event) final;
        void OnData(const void* audio_data,
            int bits_per_sample,
            int sample_rate,
            size_t number_of_channels,
            size_t number_of_frames) final;

        static void GetOnData(v8::Local<v8::String> property, const Nan::PropertyCallbackInfo<v8::Value> &info);
        static void SetOnData(v8::Local<v8::String> property, v8::Local<v8::Value> value, const Nan::PropertyCallbackInfo<void> &info);

        rtc::scoped_refptr<webrtc::AudioTrackInterface> _track;
        Nan::Persistent<v8::Function> _ondata;

        std::unique_ptr<webrtc::Resampler> _resampler;
    };
};

#endif

