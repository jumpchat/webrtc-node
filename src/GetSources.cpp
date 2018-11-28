#include "GetSources.h"
#include "Platform.h"
#include "ScreencastCapturer.h"
#include "MediaConstraints.h"

using namespace v8;
using namespace WebRTC;

void GetSources::Init(Handle<Object> exports)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    exports->Set(Nan::New("getSources").ToLocalChecked(), Nan::New<FunctionTemplate>(GetSources::GetDevices)->GetFunction());
}

rtc::scoped_refptr<webrtc::AudioTrackInterface> GetSources::GetAudioSource(const rtc::scoped_refptr<MediaConstraints>& constraints)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    rtc::scoped_refptr<webrtc::AudioTrackInterface> track;
    rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> factory = Platform::GetFactory();

    if (factory.get()) {
        cricket::AudioOptions options;
        CopyConstraintsIntoAudioOptions(constraints->ToConstraints(), &options);
        track = factory->CreateAudioTrack(rtc::CreateRandomUuid(), factory->CreateAudioSource(options));
    }

    return track;
}

rtc::scoped_refptr<webrtc::AudioTrackInterface> GetSources::GetAudioSource(const std::string id, const rtc::scoped_refptr<MediaConstraints>& constraints)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;
    return GetSources::GetAudioSource(constraints);
}

rtc::scoped_refptr<webrtc::VideoTrackInterface> GetSources::GetVideoSource(const rtc::scoped_refptr<MediaConstraints>& constraints)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    std::unique_ptr<cricket::VideoCapturer> capturer;
    rtc::scoped_refptr<webrtc::VideoTrackInterface> track;
    rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> factory = Platform::GetFactory();
    std::unique_ptr<webrtc::VideoCaptureModule::DeviceInfo> video_info(webrtc::VideoCaptureFactory::CreateDeviceInfo());
    cricket::WebRtcVideoDeviceCapturerFactory device_factory;

    if (factory.get()) {
        if (video_info) {
            int num_devices = video_info->NumberOfDevices();

            for (int i = 0; i < num_devices; ++i) {
                const uint32_t kSize = 256;
                char name[kSize] = { 0 };
                char id[kSize] = { 0 };

                if (video_info->GetDeviceName(i, name, kSize, id, kSize) != -1) {
                    capturer = device_factory.Create(cricket::Device(name, 0));

                    if (capturer) {
                        track = factory->CreateVideoTrack(rtc::CreateRandomUuid(), factory->CreateVideoSource(std::move(capturer), constraints->ToConstraints()));
                        return track;
                    }
                }
            }
        }
    }

    return track;
}

rtc::scoped_refptr<webrtc::VideoTrackInterface> GetSources::GetVideoSource(const std::string id_name, const rtc::scoped_refptr<MediaConstraints>& constraints)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    std::unique_ptr<cricket::VideoCapturer> capturer;
    rtc::scoped_refptr<webrtc::VideoTrackInterface> track;
    rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> factory = Platform::GetFactory();
    cricket::WebRtcVideoDeviceCapturerFactory device_factory;

    if (factory.get()) {
        std::string desktop("desktop");
        if (id_name.compare(0, desktop.size(), desktop) == 0) {
            // setup desktop id
            std::unique_ptr<ScreencastCapturer> screencast_capturer(new ScreencastCapturer());
            screencast_capturer->SetDesktopId(constraints->ScreenCastDesktopId());
            screencast_capturer->SetCaptureCursor(constraints->ScreenCastCaptureCursor());
            screencast_capturer->SetFPS(constraints->ScreenCastFPS());
            track = factory->CreateVideoTrack(rtc::CreateRandomUuid(), factory->CreateVideoSource(std::move(screencast_capturer), constraints->ToConstraints()));
            return track;
        } else {
            std::unique_ptr<webrtc::VideoCaptureModule::DeviceInfo> video_info(webrtc::VideoCaptureFactory::CreateDeviceInfo());
            if (video_info) {
                int num_devices = video_info->NumberOfDevices();

                for (int i = 0; i < num_devices; ++i) {
                    const uint32_t kSize = 256;
                    char name[kSize] = { 0 };
                    char id[kSize] = { 0 };

                    if (video_info->GetDeviceName(i, name, kSize, id, kSize) != -1) {
                        if (id_name.empty() || id_name.compare(id) == 0) {
                            capturer = device_factory.Create(cricket::Device(name, 0));

                            if (capturer) {
                                track = factory->CreateVideoTrack(rtc::CreateRandomUuid(), factory->CreateVideoSource(std::move(capturer), constraints->ToConstraints()));
                                return track;
                            }
                        }
                    }
                }
            }
        }
    }

	RTC_LOG(LS_INFO) << "Camera id " << id_name << " not found";

    return track;
}

Local<Value> GetSources::GetDevices()
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    Nan::EscapableHandleScope scope;
    Local<Array> list = Nan::New<Array>();
    uint32_t index = 0;

    std::unique_ptr<webrtc::VideoCaptureModule::DeviceInfo> video_info(webrtc::VideoCaptureFactory::CreateDeviceInfo());

    if (video_info) {
        int num_devices = video_info->NumberOfDevices();

        for (int i = 0; i < num_devices; ++i) {
            const uint32_t kSize = 256;
            char name[kSize] = { 0 };
            char id[kSize] = { 0 };

            if (video_info->GetDeviceName(i, name, kSize, id, kSize) != -1) {
                Local<Object> device = Nan::New<Object>();

                device->Set(Nan::New("kind").ToLocalChecked(), Nan::New("video").ToLocalChecked());
                device->Set(Nan::New("label").ToLocalChecked(), Nan::New(name).ToLocalChecked());
                device->Set(Nan::New("id").ToLocalChecked(), Nan::New(id).ToLocalChecked());

                list->Set(index, device);
                index++;
            }
        }
    }

    return scope.Escape(list);
}

void GetSources::GetDevices(const Nan::FunctionCallbackInfo<Value>& info)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    if (info.Length() == 1 && info[0]->IsFunction()) {
        Local<Function> callback = Local<Function>::Cast(info[0]);

        Local<Value> argv[1] = {
            GetSources::GetDevices()
        };

        if (!callback.IsEmpty()) {
            callback->Call(info.This(), 1, argv);
        }
    }

    info.GetReturnValue().SetUndefined();
}
