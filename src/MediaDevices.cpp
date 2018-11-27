#include "MediaDevices.h"
#include "GetSources.h"
#include "MediaConstraints.h"
#include "MediaStream.h"
#include "Platform.h"

using namespace v8;
using namespace WebRTC;

static Nan::Persistent<Function> constructor;

NAN_MODULE_INIT(MediaDevices::Init)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>(New);
    tpl->InstanceTemplate()->SetInternalFieldCount(1);
    tpl->SetClassName(Nan::New("MediaDevices").ToLocalChecked());

    Nan::SetPrototypeMethod(tpl, "getUserMedia", GetUserMedia);

    constructor.Reset<Function>(tpl->GetFunction());
    Local<Function> instance = Nan::New(constructor);
    Nan::Set(target, Nan::New("mediaDevices").ToLocalChecked(), instance->NewInstance(Nan::GetCurrentContext()).ToLocalChecked());
}

NAN_METHOD(MediaDevices::New)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    if (info.IsConstructCall()) {
        MediaDevices* mediaDevices = new MediaDevices();
        mediaDevices->Wrap(info.This());
        return info.GetReturnValue().Set(info.This());
    } else {
        Local<Function> instance = Nan::New(constructor);
        return info.GetReturnValue().Set(instance->NewInstance(Nan::GetCurrentContext()).ToLocalChecked());
    }
}


NAN_METHOD(MediaDevices::GetUserMedia)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    rtc::scoped_refptr<webrtc::MediaStreamInterface> stream;
    rtc::scoped_refptr<MediaConstraints> constraints = MediaConstraints::New(info[0]);
    const char* error = 0;
    bool have_source = false;

    Local<Promise::Resolver> resolver = Promise::Resolver::New(info.GetIsolate());
    Nan::Persistent<Promise::Resolver> *persistent = new Nan::Persistent<Promise::Resolver>(resolver);
    info.GetReturnValue().Set(resolver->GetPromise());


    std::string audioId = constraints->AudioId();
    std::string videoId = constraints->VideoId();

    if (constraints->UseAudio() || constraints->UseVideo()) {
        rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> factory = Platform::GetFactory();

        if (factory.get()) {
            stream = factory->CreateLocalMediaStream("stream");

            if (stream.get()) {
                if (constraints->UseAudio()) {
                    rtc::scoped_refptr<webrtc::AudioTrackInterface> audio_track;

                    if (audioId.empty()) {
                        audio_track = GetSources::GetAudioSource(constraints);
                    } else {
                        audio_track = GetSources::GetAudioSource(audioId, constraints);
                    }

                    if (audio_track.get()) {
                        if (!stream->AddTrack(audio_track)) {
                            error = "Invalid Audio Input";
                        } else {
                            have_source = true;
                        }
                    } else {
                        if (!audioId.empty()) {
                            error = "Invalid Audio Input";
                        }
                    }
                }

                if (constraints->UseVideo()) {
                    rtc::scoped_refptr<webrtc::VideoTrackInterface> video_track;

                    if (videoId.empty()) {
                        video_track = GetSources::GetVideoSource(constraints);
                    } else {
                        video_track = GetSources::GetVideoSource(videoId, constraints);
                    }

                    if (video_track.get()) {
                        if (!stream->AddTrack(video_track)) {
                            error = "Invalid Video Input - Failed to add track";
                        } else {
                            have_source = true;
                        }
                    } else {
                        if (!videoId.empty()) {
                            error = "Invalid Video Input - Failed to create video track";
                        }
                    }
                }
            } else {
                error = "Internal Error";
            }
        }
    }

    if (!have_source) {
        error = "No available inputs";
    }

    Handle<Value> argv[1];

    if (!error) {
        if (stream.get()) {
            argv[0] = MediaStream::New(stream);
        } else {
            error = "Invalid MediaStream";
        }
    }

    if (!stream.get()) {
        error = "Invalid MediaStream";
    }

    if (error) {
        if (!info[2].IsEmpty() && info[2]->IsFunction()) {
            Local<Function> onerror = Local<Function>::Cast(info[2]);
            argv[0] = Nan::Error(error);

            onerror->Call(info.This(), 1, argv);
        } else {
            Nan::ThrowError(error);
        }

        auto resolver = Nan::New(*persistent);
        auto result = resolver->Reject(Nan::GetCurrentContext(), Nan::New(error).ToLocalChecked());
        RTC_LOG(LS_INFO) << "GetUserMedia() failed: reject=" << result.FromMaybe(false) << " error=" << error;
    } else {
        if (!info[1].IsEmpty() && info[1]->IsFunction()) {
            Local<Function> onsuccess = Local<Function>::Cast(info[1]);
            onsuccess->Call(info.This(), 1, argv);

        }

        auto resolver = Nan::New(*persistent);
        auto result = resolver->Resolve(Nan::GetCurrentContext(), argv[0]);
        RTC_LOG(LS_INFO) << "GetUserMedia() promise result: " << result.FromMaybe(false);
    }

    // info.GetReturnValue().SetUndefined();
}
