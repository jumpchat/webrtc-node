#ifndef WEBRTC_MEDIACONSTRAINTS_H
#define WEBRTC_MEDIACONSTRAINTS_H

#include "Common.h"

namespace WebRTC {
class MediaConstraints : public webrtc::MediaConstraintsInterface, public rtc::RefCountInterface {
    friend class rtc::RefCountedObject<MediaConstraints>;

public:
    static rtc::scoped_refptr<MediaConstraints> New();
    static rtc::scoped_refptr<MediaConstraints> New(const v8::Local<v8::Object>& constraints);
    static rtc::scoped_refptr<MediaConstraints> New(const v8::Local<v8::Value>& constraints);

    bool UseAudio() const;
    bool UseVideo() const;

    std::string AudioId() const;
    std::string VideoId() const;

    int ScreenCastDesktopId() const;
    bool ScreenCastCaptureCursor() const;
    int ScreenCastFPS() const;

    bool IsMandatory(const std::string& key);
    bool GetMandatory(const std::string& key);
    void RemoveMandatory(const std::string& key);
    void AddMandatory(const std::string& key, const std::string& value);
    void SetMandatory(const std::string& key, const std::string& value);

    template <class T>
    void SetMandatory(const std::string& key, const T& value)
    {
        SetMandatory(key, rtc::ToString(value));
    }

    bool IsOptional(const std::string& key);
    bool GetOptional(const std::string& key);
    void RemoveOptional(const std::string& key);
    void AddOptional(const std::string& key, const std::string& value);
    void SetOptional(const std::string& key, const std::string& value);

    template <class T>
    void SetOptional(const std::string& key, const T& value)
    {
        SetOptional(key, rtc::ToString(value));
    }

    const webrtc::MediaConstraintsInterface* ToConstraints() const;
    const webrtc::MediaConstraintsInterface::Constraints& GetMandatory() const final;
    const webrtc::MediaConstraintsInterface::Constraints& GetOptional() const final;

private:
    explicit MediaConstraints();
    ~MediaConstraints() override;

    void SetOptional(std::string key, v8::Local<v8::Value> value);
    void SetMandatory(std::string key, v8::Local<v8::Value> value);

protected:
    bool _audio;
    bool _video;

    std::string _audioId;
    std::string _videoId;

    // screencast
    int _screenCastDesktopId;
    bool _screenCastCaptureCursor;
    int _screenCastFPS;

    webrtc::MediaConstraintsInterface::Constraints _mandatory;
    webrtc::MediaConstraintsInterface::Constraints _optional;
};
};

#endif
