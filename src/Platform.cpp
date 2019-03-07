#include "Platform.h"

using namespace WebRTC;

static std::unique_ptr<rtc::Thread> signal_thread;
static std::unique_ptr<rtc::Thread> worker_thread;
static std::unique_ptr<rtc::Thread> network_thread;
static rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> factory;
#if defined(WEBRTC_WIN)
static std::unique_ptr<rtc::WinsockInitializer> winsock_init;
#endif

void Platform::Init()
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

#if defined(WEBRTC_WIN)
    winsock_init = absl::make_unique<rtc::WinsockInitializer>();
#endif

    rtc::InitializeSSL();

    network_thread = rtc::Thread::CreateWithSocketServer();
    network_thread->SetName("network_thread", nullptr);
    RTC_CHECK(network_thread->Start()) << "Failed to start thread";

    worker_thread = rtc::Thread::Create();
    worker_thread->SetName("worker_thread", nullptr);
    RTC_CHECK(worker_thread->Start()) << "Failed to start thread";

    signal_thread = rtc::Thread::Create();
    signal_thread->SetName("signaling_thread", nullptr);
    RTC_CHECK(signal_thread->Start()) << "Failed to start thread";

     std::unique_ptr<cricket::MediaEngineInterface> media_engine =
      cricket::WebRtcMediaEngineFactory::Create(nullptr /* adm */,
                                                webrtc::CreateBuiltinAudioEncoderFactory(),
                                                webrtc::CreateBuiltinAudioDecoderFactory(),
                                                absl::make_unique<webrtc::InternalEncoderFactory>(),
                                                absl::make_unique<webrtc::InternalDecoderFactory>(),
                                                webrtc::CreateBuiltinVideoBitrateAllocatorFactory(),
                                                nullptr /* audio_mixer */,
                                                webrtc::AudioProcessingBuilder().Create());
    RTC_LOG(LS_INFO) << "Media engine created: " << media_engine.get();

    factory = webrtc::CreateModularPeerConnectionFactory(network_thread.get(),
                                                    worker_thread.get(),
                                                    signal_thread.get(),
                                                    std::move(media_engine),
                                                    webrtc::CreateCallFactory(),
                                                    webrtc::CreateRtcEventLogFactory());

//     factory = webrtc::CreatePeerConnectionFactory(
//         network_thread.get(),
//         worker_thread.get(),
//         signal_thread.get(),
//         nullptr,
//         webrtc::CreateBuiltinAudioEncoderFactory(),
//         webrtc::CreateBuiltinAudioDecoderFactory(),
//         nullptr,
//         nullptr);
}

void Platform::Dispose()
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    factory = nullptr;

    signal_thread->Stop();
    worker_thread->Stop();
    network_thread->Stop();

    signal_thread = nullptr;
    worker_thread = nullptr;
    network_thread = nullptr;

    rtc::CleanupSSL();

    #if defined(WEBRTC_WIN)
    winsock_init = nullptr;
    #endif
}

rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> Platform::GetFactory()
{
    return factory;
}
