#include "ScreencastCapturer.h"
#include "libyuv/convert.h"
#include "libyuv/video_common.h"

using namespace WebRTC;

ScreencastCapturer::ScreencastCapturer() :
    _desktopId(0),
    _captureCursor(true),
    _fps(30)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;
}

ScreencastCapturer::~ScreencastCapturer()
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;
}

cricket::CaptureState ScreencastCapturer::Start(const cricket::VideoFormat& capture_format)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

	rtc::Thread::Start();

    SetCaptureState(cricket::CS_RUNNING);
    return cricket::CS_RUNNING;
}

void ScreencastCapturer::Stop()
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;
    SetCaptureState(cricket::CS_STOPPED);
	rtc::Thread::Stop();
}

bool ScreencastCapturer::IsRunning()
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    return capture_state() == cricket::CS_RUNNING;
}

bool ScreencastCapturer::IsScreencast() const
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    return true;
}

bool ScreencastCapturer::GetPreferredFourccs(std::vector<uint32_t>* fourccs)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    fourccs->push_back(cricket::FOURCC_I420);
    return true;
}

void ScreencastCapturer::OnCaptureResult(webrtc::DesktopCapturer::Result result, std::unique_ptr<webrtc::DesktopFrame> frame)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    if (!IsRunning()) {
        return;
    }
    if(result != webrtc::DesktopCapturer::Result::SUCCESS) {
        return;
    }

    int32_t width = frame->size().width();
    int32_t height = frame->size().height();

    int adapted_width;
    int adapted_height;
    int crop_width;
    int crop_height;
    int crop_x;
    int crop_y;
    int64_t timestamp_us = rtc::TimeMicros();

    if (!cricket::VideoCapturer::AdaptFrame(width, height, timestamp_us, timestamp_us, &adapted_width,
                    &adapted_height, &crop_width, &crop_height, &crop_x,
                    &crop_y, &timestamp_us)) {
        RTC_LOG(LS_ERROR) << "Can't adapt frame " << width << "x" << height;
        return;
    }

    rtc::scoped_refptr<webrtc::I420Buffer> buffer(webrtc::I420Buffer::Create(width, height));
    size_t frameSize = frame->size().height()*frame->stride();
    const int conversionResult = libyuv::ConvertToI420((const uint8_t*)frame->data(), frameSize,
							buffer->MutableDataY(), buffer->StrideY(),
							buffer->MutableDataU(), buffer->StrideU(),
							buffer->MutableDataV(), buffer->StrideV(),
							0, 0,
							width, height,
							width, height,
							libyuv::kRotate0, libyuv::FOURCC_ARGB);
    if (conversionResult >= 0) {
        if (adapted_width != width || adapted_height != height) {
            rtc::scoped_refptr<webrtc::I420Buffer> i420_buffer(webrtc::I420Buffer::Create(adapted_width, adapted_height));
            i420_buffer->CropAndScaleFrom(*buffer, crop_x, crop_y, crop_width, crop_height);
            buffer = i420_buffer;
        }

        webrtc::VideoFrame frame(buffer, 0, timestamp_us, webrtc::kVideoRotation_0);
        OnFrame(frame, width, height);
    } else {
        RTC_LOG(LS_ERROR) << "ScreencastCapturer::OnCaptureResult conversion error:" << conversionResult;
    }
}

void ScreencastCapturer::Run()
{
    webrtc::DesktopCaptureOptions options = webrtc::DesktopCaptureOptions::CreateDefault();
#ifdef WIN32
    options.set_allow_directx_capturer(true);
#endif
    _capturer = webrtc::DesktopCapturer::CreateScreenCapturer(options);
    _capturer->SelectSource(_desktopId);

    if (_captureCursor) {
        _capturer = std::unique_ptr<webrtc::DesktopAndCursorComposer>(new webrtc::DesktopAndCursorComposer(std::move(_capturer), options));
    }

    _capturer->Start(this);

    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;
	while (IsRunning()) {
        _capturer->CaptureFrame();
        int64_t timestamp_us = rtc::TimeMicros();
        int64_t diff_us = rtc::TimeMicros() - timestamp_us;

        int sleepMs = (1000/_fps) - ((int)diff_us/1000);
        if (sleepMs < 0) {
            sleepMs = 16;
        }
        // RTC_LOG(LS_INFO) << "Capture Frame " << sleepMs << "ms";
        rtc::Thread::SleepMs(sleepMs);
    }

    _capturer = NULL;
}

void ScreencastCapturer::SetDesktopId(int desktopId)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__ <<  " : " << desktopId;
    _desktopId = desktopId;
}

void ScreencastCapturer::SetCaptureCursor(bool captureCursor)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__ <<  " : " << captureCursor;
    _captureCursor = captureCursor;
}

void ScreencastCapturer::SetFPS(int fps)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__ <<  " : " << fps;
    _fps = fps;
}
