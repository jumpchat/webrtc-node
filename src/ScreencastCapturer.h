#ifndef WEBRTC_SCREENCASTCAPTURER_H
#define WEBRTC_SCREENCASTCAPTURER_H

#include "Common.h"
#include "EventEmitter.h"

namespace WebRTC {

class ScreencastCapturer : public cricket::VideoCapturer,
                            public webrtc::DesktopCapturer::Callback, public rtc::Thread {
    public:
        enum ScreencastCapturerEvent {
            kScreencastCapturerCaptureFrame,
            kScreencastCapturerStart
        };
        ScreencastCapturer();
        virtual ~ScreencastCapturer();

        // cricket::VideoCapturer
        virtual cricket::CaptureState Start(const cricket::VideoFormat& capture_format);
        virtual void Stop();
        virtual bool IsRunning();
        virtual bool IsScreencast() const;
        virtual bool GetPreferredFourccs(std::vector<uint32_t>* fourccs);

        // webrtc::DesktopCapturer::Callback
        virtual void OnCaptureResult(webrtc::DesktopCapturer::Result result, std::unique_ptr<webrtc::DesktopFrame> frame);

        // rtc::Thread
        virtual void Run();

        // setters/getters
        void SetDesktopId(int desktopId);
        void SetCaptureCursor(bool captureCursor);
        void SetFPS(int fps);

    private:
        std::unique_ptr<webrtc::DesktopCapturer> _capturer;
        int _desktopId;
        bool _captureCursor;
        int _fps;
};
}

#endif // WEBRTC_SCREENCASTCAPTURER_H
