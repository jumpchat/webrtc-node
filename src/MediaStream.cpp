#include "MediaStream.h"
#include "MediaStreamTrack.h"
#include "Platform.h"

using namespace v8;
using namespace WebRTC;

Nan::Persistent<Function> MediaStream::constructor;

NAN_MODULE_INIT(MediaStream::Init)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    Nan::HandleScope scope;

    Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>(MediaStream::New);
    tpl->InstanceTemplate()->SetInternalFieldCount(1);
    tpl->SetClassName(Nan::New("MediaStream").ToLocalChecked());

    Nan::SetPrototypeMethod(tpl, "addTrack", MediaStream::AddTrack);
    Nan::SetPrototypeMethod(tpl, "removeTrack", MediaStream::RemoveTrack);
    Nan::SetPrototypeMethod(tpl, "clone", MediaStream::Clone);
    Nan::SetPrototypeMethod(tpl, "getAudioTracks", MediaStream::GetAudioTracks);
    Nan::SetPrototypeMethod(tpl, "getTrackById", MediaStream::GetTrackById);
    Nan::SetPrototypeMethod(tpl, "getVideoTracks", MediaStream::GetVideoTracks);
    Nan::SetPrototypeMethod(tpl, "getTracks", MediaStream::GetTracks);

    Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("active").ToLocalChecked(), MediaStream::GetActive);
    Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("ended").ToLocalChecked(), MediaStream::GetEnded);
    Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("id").ToLocalChecked(), MediaStream::GetId);
    Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("onaddtrack").ToLocalChecked(), MediaStream::GetOnAddTrack, MediaStream::SetOnAddTrack);
    Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("onremovetrack").ToLocalChecked(), MediaStream::GetOnRemoveTrack, MediaStream::SetOnRemoveTrack);

    constructor.Reset(tpl->GetFunction());
    Nan::Set(target, Nan::New("MediaStream").ToLocalChecked(), tpl->GetFunction());
}

Local<Value> MediaStream::New(rtc::scoped_refptr<webrtc::MediaStreamInterface> mediaStream)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    Nan::EscapableHandleScope scope;

    Local<Value> empty;
    Local<Function> instance = Nan::New(MediaStream::constructor);

    if (instance.IsEmpty() || !mediaStream.get()) {
        return scope.Escape(Nan::Null());
    }

    Local<Object> ret = instance->NewInstance(Nan::GetCurrentContext()).ToLocalChecked();
    MediaStream* self = Nan::ObjectWrap::Unwrap<MediaStream>(ret);
    if (self) {
        self->_stream = mediaStream;
        self->_audio_tracks = self->_stream->GetAudioTracks();
        self->_video_tracks = self->_stream->GetVideoTracks();
        // self->_stream->RegisterObserver(self->_observer.get());
        self->CheckState();

        return scope.Escape(ret);
    }

    return scope.Escape(Nan::Null());
}

MediaStream::MediaStream()
    : _active(false)
    , _ended(true)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    // _observer = new rtc::RefCountedObject<MediaStreamObserver>(this);
}

MediaStream::~MediaStream()
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    if (_stream.get()) {
        // _stream->UnregisterObserver(_observer.get());
    }

    // _observer->RemoveListener(this);
}

NAN_METHOD(MediaStream::New)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    Nan::HandleScope scope;

    if (info.IsConstructCall()) {
        RTC_LOG(LS_INFO) << "MediaStream Constructor Call";
        MediaStream* mediaStream = new MediaStream();
        mediaStream->Wrap(info.This());

        mediaStream->_stream = webrtc::MediaStream::Create(rtc::CreateRandomUuid());
        mediaStream->_audio_tracks = mediaStream->_stream->GetAudioTracks();
        mediaStream->_video_tracks = mediaStream->_stream->GetVideoTracks();
        // mediaStream->_stream->RegisterObserver(mediaStream->_observer.get());
        mediaStream->CheckState();

        return info.GetReturnValue().Set(info.This());
    }

    Nan::ThrowError("Internal Error");
    info.GetReturnValue().SetUndefined();
}

rtc::scoped_refptr<webrtc::MediaStreamInterface> MediaStream::Unwrap(Local<Object> value)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    if (!value.IsEmpty()) {
        MediaStream* self = Nan::ObjectWrap::Unwrap<MediaStream>(value);

        if (self) {
            return self->_stream;
        }
    }

    return 0;
}

rtc::scoped_refptr<webrtc::MediaStreamInterface> MediaStream::Unwrap(Local<Value> value)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    if (!value.IsEmpty() && value->IsObject()) {
        Local<Object> stream = Local<Object>::Cast(value);
        return MediaStream::Unwrap(stream);
    }

    return 0;
}

NAN_METHOD(MediaStream::AddTrack)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    rtc::scoped_refptr<webrtc::MediaStreamInterface> stream = MediaStream::Unwrap(info.This());
    bool retval = false;

    if (stream.get()) {
        if (info.Length() >= 1 && info[0]->IsObject()) {
            rtc::scoped_refptr<webrtc::MediaStreamTrackInterface> track = MediaStreamTrack::Unwrap(info[0]);

            if (track.get()) {
                std::string kind = track->kind();

                if (kind.compare("audio") == 0) {
                    rtc::scoped_refptr<webrtc::AudioTrackInterface> audio(static_cast<webrtc::AudioTrackInterface*>(track.get()));
                    retval = stream->AddTrack(audio);
                } else {
                    rtc::scoped_refptr<webrtc::VideoTrackInterface> video(static_cast<webrtc::VideoTrackInterface*>(track.get()));
                    retval = stream->AddTrack(video);
                }
            }
        }
    } else {
        Nan::ThrowError("Internal Error");
    }

    return info.GetReturnValue().Set(Nan::New(retval));
}

NAN_METHOD(MediaStream::RemoveTrack)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    rtc::scoped_refptr<webrtc::MediaStreamInterface> stream = MediaStream::Unwrap(info.This());
    bool retval = false;

    if (stream.get()) {
        if (info.Length() >= 1 && info[0]->IsObject()) {
            rtc::scoped_refptr<webrtc::MediaStreamTrackInterface> track = MediaStreamTrack::Unwrap(info[0]);

            if (track.get()) {
                std::string kind = track->kind();

                if (kind.compare("audio") == 0) {
                    rtc::scoped_refptr<webrtc::AudioTrackInterface> audio(static_cast<webrtc::AudioTrackInterface*>(track.get()));
                    retval = stream->RemoveTrack(audio);
                } else {
                    rtc::scoped_refptr<webrtc::VideoTrackInterface> video(static_cast<webrtc::VideoTrackInterface*>(track.get()));
                    retval = stream->RemoveTrack(video);
                }
            }
        }
    } else {
        Nan::ThrowError("Internal Error");
    }

    return info.GetReturnValue().Set(Nan::New(retval));
}

NAN_METHOD(MediaStream::Clone)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    rtc::scoped_refptr<webrtc::MediaStreamInterface> self = MediaStream::Unwrap(info.This());
    rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> factory = WebRTC::Platform::GetFactory();
    rtc::scoped_refptr<webrtc::MediaStreamInterface> stream;

    if (self.get() && factory.get()) {
        stream = factory->CreateLocalMediaStream("stream");

        if (stream.get()) {
            webrtc::AudioTrackVector audio_list = self->GetAudioTracks();
            std::vector<rtc::scoped_refptr<webrtc::AudioTrackInterface> >::iterator audio_it;

            for (audio_it = audio_list.begin(); audio_it != audio_list.end(); audio_it++) {
                rtc::scoped_refptr<webrtc::AudioTrackInterface> track(*audio_it);

                if (track.get()) {
                    stream->AddTrack(track.get());
                }
            }

            webrtc::VideoTrackVector video_list = self->GetVideoTracks();
            std::vector<rtc::scoped_refptr<webrtc::VideoTrackInterface> >::iterator video_it;

            for (video_it = video_list.begin(); video_it != video_list.end(); video_it++) {
                rtc::scoped_refptr<webrtc::VideoTrackInterface> track(*video_it);

                if (track.get()) {
                    stream->AddTrack(track.get());
                }
            }

            return info.GetReturnValue().Set(MediaStream::New(stream));
        }
    }

    Nan::ThrowError("Internal Error");
    info.GetReturnValue().SetUndefined();
}

NAN_METHOD(MediaStream::GetTrackById)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    rtc::scoped_refptr<webrtc::MediaStreamInterface> stream = MediaStream::Unwrap(info.This());

    if (stream.get()) {
        if (info.Length() >= 1 && info[0]->IsString()) {
            v8::String::Utf8Value idValue(info[0]->ToString());
            std::string id(*idValue);

            rtc::scoped_refptr<webrtc::AudioTrackInterface> audio = stream->FindAudioTrack(id);

            if (audio.get()) {
                return info.GetReturnValue().Set(MediaStreamTrack::New(audio.get()));
            }

            rtc::scoped_refptr<webrtc::VideoTrackInterface> video = stream->FindVideoTrack(id);

            if (video.get()) {
                return info.GetReturnValue().Set(MediaStreamTrack::New(video.get()));
            }
        }

        return info.GetReturnValue().Set(Nan::Null());
    } else {
        Nan::ThrowError("Internal Error");
    }

    info.GetReturnValue().SetUndefined();
}

NAN_METHOD(MediaStream::GetAudioTracks)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    rtc::scoped_refptr<webrtc::MediaStreamInterface> self = MediaStream::Unwrap(info.This());

    if (self.get()) {
        webrtc::AudioTrackVector audio_list = self->GetAudioTracks();
        std::vector<rtc::scoped_refptr<webrtc::AudioTrackInterface> >::iterator audio_it;
        Local<Array> list = Nan::New<Array>();
        uint32_t index = 0;

        for (audio_it = audio_list.begin(); audio_it != audio_list.end(); audio_it++) {
            rtc::scoped_refptr<webrtc::AudioTrackInterface> track(*audio_it);

            if (track.get()) {
                list->Set(index, MediaStreamTrack::New(track.get()));
                index++;
            }
        }

        return info.GetReturnValue().Set(list);
    } else {
        Nan::ThrowError("Internal Error");
    }

    info.GetReturnValue().SetUndefined();
}

NAN_METHOD(MediaStream::GetVideoTracks)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    rtc::scoped_refptr<webrtc::MediaStreamInterface> self = MediaStream::Unwrap(info.This());

    if (self.get()) {
        webrtc::VideoTrackVector video_list = self->GetVideoTracks();
        std::vector<rtc::scoped_refptr<webrtc::VideoTrackInterface> >::iterator video_it;
        Local<Array> list = Nan::New<Array>();
        uint32_t index = 0;

        for (video_it = video_list.begin(); video_it != video_list.end(); video_it++) {
            rtc::scoped_refptr<webrtc::VideoTrackInterface> track(*video_it);

            if (track.get()) {
                list->Set(index, MediaStreamTrack::New(track.get()));
                index++;
            }
        }

        return info.GetReturnValue().Set(list);
    } else {
        Nan::ThrowError("Internal Error");
    }

    info.GetReturnValue().SetUndefined();
}

NAN_METHOD(MediaStream::GetTracks)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    rtc::scoped_refptr<webrtc::MediaStreamInterface> self = MediaStream::Unwrap(info.This());

    if (self.get()) {
        webrtc::AudioTrackVector audio_list = self->GetAudioTracks();
        webrtc::VideoTrackVector video_list = self->GetVideoTracks();

        std::vector<rtc::scoped_refptr<webrtc::AudioTrackInterface> >::iterator audio_it;
        std::vector<rtc::scoped_refptr<webrtc::VideoTrackInterface> >::iterator video_it;

        Local<Array> list = Nan::New<Array>();
        uint32_t index = 0;

        for (audio_it = audio_list.begin(); audio_it != audio_list.end(); audio_it++) {
            rtc::scoped_refptr<webrtc::AudioTrackInterface> track(*audio_it);

            if (track.get()) {
                list->Set(index, MediaStreamTrack::New(track.get()));
                index++;
            }
        }

        for (video_it = video_list.begin(); video_it != video_list.end(); video_it++) {
            rtc::scoped_refptr<webrtc::VideoTrackInterface> track(*video_it);

            if (track.get()) {
                list->Set(index, MediaStreamTrack::New(track.get()));
                index++;
            }
        }

        return info.GetReturnValue().Set(list);
    } else {
        Nan::ThrowError("Internal Error");
    }

    info.GetReturnValue().SetUndefined();
}

NAN_GETTER(MediaStream::GetActive)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    MediaStream* self = Nan::ObjectWrap::Unwrap<MediaStream>(info.Holder());
    return info.GetReturnValue().Set(Nan::New(self->_active));
}

NAN_GETTER(MediaStream::GetEnded)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    MediaStream* self = Nan::ObjectWrap::Unwrap<MediaStream>(info.Holder());
    return info.GetReturnValue().Set(Nan::New(self->_ended));
}

NAN_GETTER(MediaStream::GetId)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    rtc::scoped_refptr<webrtc::MediaStreamInterface> stream = MediaStream::Unwrap(info.Holder());

    if (stream.get()) {
        return info.GetReturnValue().Set(Nan::New(stream->id().c_str()).ToLocalChecked());
    } else {
        Nan::ThrowError("Internal Error");
    }

    info.GetReturnValue().SetUndefined();
}

NAN_GETTER(MediaStream::GetOnAddTrack)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    MediaStream* self = Nan::ObjectWrap::Unwrap<MediaStream>(info.Holder());
    return info.GetReturnValue().Set(Nan::New<Function>(self->_onaddtrack));
}

NAN_GETTER(MediaStream::GetOnRemoveTrack)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    MediaStream* self = Nan::ObjectWrap::Unwrap<MediaStream>(info.Holder());
    return info.GetReturnValue().Set(Nan::New<Function>(self->_onremovetrack));
}

NAN_SETTER(MediaStream::ReadOnly)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;
}

NAN_SETTER(MediaStream::SetOnAddTrack)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    MediaStream* self = Nan::ObjectWrap::Unwrap<MediaStream>(info.Holder());

    if (!value.IsEmpty() && value->IsFunction()) {
        self->_onaddtrack.Reset<Function>(Local<Function>::Cast(value));
    } else {
        self->_onaddtrack.Reset();
    }
}

NAN_SETTER(MediaStream::SetOnRemoveTrack)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    MediaStream* self = Nan::ObjectWrap::Unwrap<MediaStream>(info.Holder());

    if (!value.IsEmpty() && value->IsFunction()) {
        self->_onremovetrack.Reset<Function>(Local<Function>::Cast(value));
    } else {
        self->_onremovetrack.Reset();
    }
}

void MediaStream::CheckState()
{
    _active = false;
    _ended = true;

    webrtc::AudioTrackVector new_audio_tracks = _stream->GetAudioTracks();
    webrtc::VideoTrackVector new_video_tracks = _stream->GetVideoTracks();

    for (const auto& cached_track : _audio_tracks) {
        auto it = std::find_if(
            new_audio_tracks.begin(), new_audio_tracks.end(),
            [cached_track](const webrtc::AudioTrackVector::value_type& new_track) {
                return new_track->id().compare(cached_track->id()) == 0;
            });
        if (it == new_audio_tracks.end()) {
            Local<Function> callback = Nan::New<Function>(_onremovetrack);
            Local<Value> argv[] = {
                MediaStreamTrack::New(cached_track.get())
            };

            if (!callback.IsEmpty() && callback->IsFunction()) {
                callback->Call(handle(), 1, argv);
            }
        }
    }

    for (const auto& new_track : new_audio_tracks) {
        if (new_track->state() == webrtc::MediaStreamTrackInterface::kLive) {
            _active = true;
            _ended = false;
        }

        auto it = std::find_if(
            _audio_tracks.begin(), _audio_tracks.end(),
            [new_track](const webrtc::AudioTrackVector::value_type& cached_track) {
                return new_track->id().compare(cached_track->id()) == 0;
            });
        if (it == _audio_tracks.end()) {
            Local<Function> callback = Nan::New<Function>(_onaddtrack);
            Local<Value> argv[] = {
                MediaStreamTrack::New(new_track.get())
            };

            if (!callback.IsEmpty() && callback->IsFunction()) {
                callback->Call(handle(), 1, argv);
            }
        }
    }

    for (const auto& cached_track : _video_tracks) {
        auto it = std::find_if(
            new_video_tracks.begin(), new_video_tracks.end(),
            [cached_track](const webrtc::VideoTrackVector::value_type& new_track) {
                return new_track->id().compare(cached_track->id()) == 0;
            });
        if (it == new_video_tracks.end()) {
            Local<Function> callback = Nan::New<Function>(_onremovetrack);
            Local<Value> argv[] = {
                MediaStreamTrack::New(cached_track.get())
            };

            if (!callback.IsEmpty() && callback->IsFunction()) {
                callback->Call(handle(), 1, argv);
            }
        }
    }

    for (const auto& new_track : new_video_tracks) {
        if (new_track->state() == webrtc::MediaStreamTrackInterface::kLive) {
            _active = true;
            _ended = false;
        }

        auto it = std::find_if(
            _video_tracks.begin(), _video_tracks.end(),
            [new_track](const webrtc::VideoTrackVector::value_type& cached_track) {
                return new_track->id().compare(cached_track->id()) == 0;
            });
        if (it == _video_tracks.end()) {
            Local<Function> callback = Nan::New<Function>(_onaddtrack);
            Local<Value> argv[] = {
                MediaStreamTrack::New(new_track.get())
            };

            if (!callback.IsEmpty() && callback->IsFunction()) {
                callback->Call(handle(), 1, argv);
            }
        }
    }

    _audio_tracks = new_audio_tracks;
    _video_tracks = new_video_tracks;
}

void MediaStream::On(Event* event)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    Nan::HandleScope scope;
    MediaStreamEvent type = event->Type<MediaStreamEvent>();

    if (type != kMediaStreamChanged) {
        Nan::ThrowError("Internal Error");
        return;
    }

    MediaStream::CheckState();
}
