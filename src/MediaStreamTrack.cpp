#include "MediaStreamTrack.h"
#include "MediaVideoRenderer.h"
#include "MediaAudioRenderer.h"

#include "pc/videocapturertracksource.h"
#include "pc/videocapturertracksource.h"

using namespace v8;
using namespace WebRTC;

Nan::Persistent<Function> MediaStreamTrack::constructor;

NAN_MODULE_INIT(MediaStreamTrack::Init)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    Nan::HandleScope scope;

    Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>(New);
    tpl->InstanceTemplate()->SetInternalFieldCount(1);
    tpl->SetClassName(Nan::New("MediaStreamTrack").ToLocalChecked());

    Nan::SetPrototypeMethod(tpl, "getConstraints", GetConstraints);
    Nan::SetPrototypeMethod(tpl, "applyConstraints", ApplyConstraints);
    Nan::SetPrototypeMethod(tpl, "setSettings", GetSettings);
    Nan::SetPrototypeMethod(tpl, "getCapabilities", GetCapabilities);
    Nan::SetPrototypeMethod(tpl, "clone", Clone);
    Nan::SetPrototypeMethod(tpl, "addRenderer", AddRenderer);
    Nan::SetPrototypeMethod(tpl, "removeRenderer", RemoveRenderer);
    Nan::SetPrototypeMethod(tpl, "stop", Stop);

    Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("enabled").ToLocalChecked(), MediaStreamTrack::GetEnabled, SetEnabled);
    Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("id").ToLocalChecked(), GetId);
    Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("kind").ToLocalChecked(), GetKind);
    Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("label").ToLocalChecked(), GetLabel);
    Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("muted").ToLocalChecked(), GetMuted);
    Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("readonly").ToLocalChecked(), GetReadOnly);
    Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("readyState").ToLocalChecked(), GetReadyState);
    Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("remote").ToLocalChecked(), GetRemote);
    Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("onstarted").ToLocalChecked(), GetOnStarted, SetOnStarted);
    Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("onmute").ToLocalChecked(), GetOnMute, SetOnMute);
    Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("onunmute").ToLocalChecked(), GetOnUnMute, SetOnUnMute);
    Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("onoverconstrained").ToLocalChecked(), GetOnOverConstrained, SetOnOverConstrained);
    Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("onended").ToLocalChecked(), GetOnEnded, SetOnEnded);

    constructor.Reset<Function>(tpl->GetFunction());
}

Local<Value> MediaStreamTrack::New(rtc::scoped_refptr<webrtc::MediaStreamTrackInterface> track)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    Nan::EscapableHandleScope scope;

    Local<Value> argv[1];
    Local<Function> instance = Nan::New(MediaStreamTrack::constructor);

    if (instance.IsEmpty() || !track.get()) {
        return scope.Escape(Nan::Null());
    }

    Local<Object> ret = instance->NewInstance(Nan::GetCurrentContext(), 0, argv).ToLocalChecked();
    MediaStreamTrack* self = Nan::ObjectWrap::Unwrap<MediaStreamTrack>(ret);

    self->isVideoTrack = track->kind() == webrtc::MediaStreamTrackInterface::kVideoKind;
    self->isAudioTrack = track->kind() == webrtc::MediaStreamTrackInterface::kAudioKind;
    self->_track = track;
    if (self->isAudioTrack) {
        rtc::scoped_refptr<webrtc::AudioTrackInterface> audio(static_cast<webrtc::AudioTrackInterface*>(track.get()));
        self->_source = audio->GetSource();
    } else if (self->isVideoTrack) {
        rtc::scoped_refptr<webrtc::VideoTrackInterface> video(static_cast<webrtc::VideoTrackInterface*>(track.get()));
        self->_source = video->GetSource();
    }
    self->_track_state = self->_track->state();
    self->_source_state = self->_source->state();
    self->_track->RegisterObserver(self);
    self->_source->RegisterObserver(self);
    self->CheckState();

    return scope.Escape(ret);
}

MediaStreamTrack::MediaStreamTrack()
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;
}

MediaStreamTrack::~MediaStreamTrack()
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    if (_track.get()) {
        _track->UnregisterObserver(this);
    }

    if (_source.get()) {
        _source->UnregisterObserver(this);
    }
}

NAN_METHOD(MediaStreamTrack::New)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    Nan::HandleScope scope;

    if (info.IsConstructCall()) {
        MediaStreamTrack* mediaStreamTrack = new MediaStreamTrack();
        mediaStreamTrack->Wrap(info.This());
        return info.GetReturnValue().Set(info.This());
    }

    Nan::ThrowError("Internal Error");
    info.GetReturnValue().SetUndefined();
}

rtc::scoped_refptr<webrtc::MediaStreamTrackInterface> MediaStreamTrack::Unwrap(Local<Object> value)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    if (!value.IsEmpty()) {
        MediaStreamTrack* self = Nan::ObjectWrap::Unwrap<MediaStreamTrack>(value);

        if (self) {
            return self->_track;
        }
    }

    return 0;
}

rtc::scoped_refptr<webrtc::MediaStreamTrackInterface> MediaStreamTrack::Unwrap(Local<Value> value)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    if (!value.IsEmpty() && value->IsObject()) {
        Local<Object> track = Local<Object>::Cast(value);
        return MediaStreamTrack::Unwrap(track);
    }

    return 0;
}

NAN_METHOD(MediaStreamTrack::GetConstraints)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    info.GetReturnValue().SetUndefined();
}

NAN_METHOD(MediaStreamTrack::ApplyConstraints)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    info.GetReturnValue().SetUndefined();
}

NAN_METHOD(MediaStreamTrack::GetSettings)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    info.GetReturnValue().SetUndefined();
}

NAN_METHOD(MediaStreamTrack::GetCapabilities)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    info.GetReturnValue().SetUndefined();
}

NAN_METHOD(MediaStreamTrack::Clone)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    info.GetReturnValue().SetUndefined();
}

NAN_METHOD(MediaStreamTrack::AddRenderer)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;
    MediaStreamTrack *self = Nan::ObjectWrap::Unwrap<MediaStreamTrack>(info.This());

    if (info.Length() >= 1 && info[0]->IsObject()) {
        Local<Object> rendererObject = Local<Object>::Cast(info[0]);
        RTC_LOG(LS_INFO) << "Track kind: " << self->_track->kind();
        if (self->_track->kind() == webrtc::MediaStreamTrackInterface::kVideoKind)
        {
            // video
            RTC_LOG(LS_INFO) << "Adding video renderer";
            webrtc::VideoTrackInterface *track = reinterpret_cast<webrtc::VideoTrackInterface *>(self->_track.get());
            if (track) {
                MediaVideoRenderer *renderer = Nan::ObjectWrap::Unwrap<MediaVideoRenderer>(rendererObject);
                renderer->SetTrack(track);
                track->AddOrUpdateSink(renderer, rtc::VideoSinkWants());
            }
        } else {
            // audio
            RTC_LOG(LS_INFO) << "Adding audio renderer";
            webrtc::AudioTrackInterface *track = reinterpret_cast<webrtc::AudioTrackInterface *>(self->_track.get());
            if (track) {
                MediaAudioRenderer *renderer = Nan::ObjectWrap::Unwrap<MediaAudioRenderer>(rendererObject);
                renderer->SetTrack(track);
                track->AddSink(renderer);
            }
        }
    }

}

NAN_METHOD(MediaStreamTrack::RemoveRenderer)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    MediaStreamTrack *self = Nan::ObjectWrap::Unwrap<MediaStreamTrack>(info.This());
    if (info.Length() >= 1 && info[0]->IsObject()) {
        Local<Object> rendererObject = Local<Object>::Cast(info[0]);

        if (self->_track->kind() == webrtc::MediaStreamTrackInterface::kVideoKind)
        {
            // audio
            MediaVideoRenderer *renderer = Nan::ObjectWrap::Unwrap<MediaVideoRenderer>(rendererObject);
            webrtc::VideoTrackInterface *track = reinterpret_cast<webrtc::VideoTrackInterface *>(self->_track.get());
            if (track) {
                renderer->SetTrack(nullptr);
                track->RemoveSink(renderer);
            }
        } else {
            // video
            MediaAudioRenderer *renderer = Nan::ObjectWrap::Unwrap<MediaAudioRenderer>(rendererObject);
            webrtc::AudioTrackInterface *track = reinterpret_cast<webrtc::AudioTrackInterface *>(self->_track.get());
            if (track) {
                renderer->SetTrack(nullptr);
                track->RemoveSink(renderer);
            }
        }
    }

}

NAN_METHOD(MediaStreamTrack::Stop)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;
    // TODO: How to stop stream?
 
    MediaStreamTrack *self = Nan::ObjectWrap::Unwrap<MediaStreamTrack>(info.This());
    if (self->_track->kind() == webrtc::MediaStreamTrackInterface::kVideoKind) {
    } else {
    }

}

NAN_GETTER(MediaStreamTrack::GetEnabled)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    MediaStreamTrack* self = Nan::ObjectWrap::Unwrap<MediaStreamTrack>(info.This());
    info.GetReturnValue().Set(Nan::New(self->_track->enabled()));
}

NAN_GETTER(MediaStreamTrack::GetId)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    MediaStreamTrack* self = Nan::ObjectWrap::Unwrap<MediaStreamTrack>(info.Holder());
    info.GetReturnValue().Set(Nan::New(self->_track->id().c_str()).ToLocalChecked());
}

NAN_GETTER(MediaStreamTrack::GetKind)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    MediaStreamTrack* self = Nan::ObjectWrap::Unwrap<MediaStreamTrack>(info.Holder());
    info.GetReturnValue().Set(Nan::New(self->_track->kind().c_str()).ToLocalChecked());
}

NAN_GETTER(MediaStreamTrack::GetLabel)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    // We return the kind for the label so there is something to see
    MediaStreamTrack* self = Nan::ObjectWrap::Unwrap<MediaStreamTrack>(info.Holder());
    info.GetReturnValue().Set(Nan::New(self->_track->kind().c_str()).ToLocalChecked());
}

NAN_GETTER(MediaStreamTrack::GetMuted)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    MediaStreamTrack* self = Nan::ObjectWrap::Unwrap<MediaStreamTrack>(info.Holder());
    info.GetReturnValue().Set(Nan::New((self->_source->state() == webrtc::MediaSourceInterface::kMuted) ? true : false));
}

NAN_GETTER(MediaStreamTrack::GetReadOnly)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    return info.GetReturnValue().Set(Nan::New(true));
}

NAN_GETTER(MediaStreamTrack::GetReadyState)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    MediaStreamTrack* self = Nan::ObjectWrap::Unwrap<MediaStreamTrack>(info.Holder());

    if (self->_track->state() == webrtc::MediaStreamTrackInterface::kLive) {
        return info.GetReturnValue().Set(Nan::New("live").ToLocalChecked());
    }

    info.GetReturnValue().Set(Nan::New("ended").ToLocalChecked());
}

NAN_GETTER(MediaStreamTrack::GetRemote)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    MediaStreamTrack* self = Nan::ObjectWrap::Unwrap<MediaStreamTrack>(info.Holder());
    info.GetReturnValue().Set(Nan::New(self->_source->remote()));
}

NAN_GETTER(MediaStreamTrack::GetOnStarted)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    MediaStreamTrack* self = Nan::ObjectWrap::Unwrap<MediaStreamTrack>(info.Holder());
    return info.GetReturnValue().Set(Nan::New<Function>(self->_onstarted));
}

NAN_GETTER(MediaStreamTrack::GetOnMute)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    MediaStreamTrack* self = Nan::ObjectWrap::Unwrap<MediaStreamTrack>(info.Holder());
    return info.GetReturnValue().Set(Nan::New<Function>(self->_onmute));
}

NAN_GETTER(MediaStreamTrack::GetOnUnMute)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    MediaStreamTrack* self = Nan::ObjectWrap::Unwrap<MediaStreamTrack>(info.Holder());
    return info.GetReturnValue().Set(Nan::New<Function>(self->_onunmute));
}

NAN_GETTER(MediaStreamTrack::GetOnOverConstrained)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    MediaStreamTrack* self = Nan::ObjectWrap::Unwrap<MediaStreamTrack>(info.Holder());
    return info.GetReturnValue().Set(Nan::New<Function>(self->_onoverconstrained));
}

NAN_GETTER(MediaStreamTrack::GetOnEnded)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    MediaStreamTrack* self = Nan::ObjectWrap::Unwrap<MediaStreamTrack>(info.Holder());
    return info.GetReturnValue().Set(Nan::New<Function>(self->_onended));
}

NAN_SETTER(MediaStreamTrack::SetReadOnly)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    // TODO(): Implement This
}

NAN_SETTER(MediaStreamTrack::SetEnabled)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    MediaStreamTrack* self = Nan::ObjectWrap::Unwrap<MediaStreamTrack>(info.Holder());

    if (!value.IsEmpty() && value->IsBoolean()) {
        self->_track->set_enabled(value->IsTrue() ? true : false);
    }
}

NAN_SETTER(MediaStreamTrack::SetOnStarted)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    MediaStreamTrack* self = Nan::ObjectWrap::Unwrap<MediaStreamTrack>(info.Holder());

    if (!value.IsEmpty() && value->IsFunction()) {
        self->_onstarted.Reset<Function>(Local<Function>::Cast(value));
    } else {
        self->_onstarted.Reset();
    }
}

NAN_SETTER(MediaStreamTrack::SetOnMute)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    MediaStreamTrack* self = Nan::ObjectWrap::Unwrap<MediaStreamTrack>(info.Holder());

    if (!value.IsEmpty() && value->IsFunction()) {
        self->_onmute.Reset<Function>(Local<Function>::Cast(value));
    } else {
        self->_onmute.Reset();
    }
}

NAN_SETTER(MediaStreamTrack::SetOnUnMute)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    MediaStreamTrack* self = Nan::ObjectWrap::Unwrap<MediaStreamTrack>(info.Holder());

    if (!value.IsEmpty() && value->IsFunction()) {
        self->_onunmute.Reset<Function>(Local<Function>::Cast(value));
    } else {
        self->_onunmute.Reset();
    }
}

NAN_SETTER(MediaStreamTrack::SetOnOverConstrained)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    MediaStreamTrack* self = Nan::ObjectWrap::Unwrap<MediaStreamTrack>(info.Holder());

    if (!value.IsEmpty() && value->IsFunction()) {
        self->_onoverconstrained.Reset<Function>(Local<Function>::Cast(value));
    } else {
        self->_onoverconstrained.Reset();
    }
}

NAN_SETTER(MediaStreamTrack::SetOnEnded)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    MediaStreamTrack* self = Nan::ObjectWrap::Unwrap<MediaStreamTrack>(info.Holder());

    if (!value.IsEmpty() && value->IsFunction()) {
        self->_onended.Reset<Function>(Local<Function>::Cast(value));
    } else {
        self->_onended.Reset();
    }
}

void MediaStreamTrack::CheckState()
{
    webrtc::MediaStreamTrackInterface::TrackState new_state = _track->state();
    webrtc::MediaSourceInterface::SourceState new_source = _source->state();

    if (_track_state != new_state) {
        if (new_state == webrtc::MediaStreamTrackInterface::kEnded) {
            Local<Function> callback = Nan::New<Function>(_onended);
            Local<Value> argv[1];

            if (!callback.IsEmpty() && callback->IsFunction()) {
                callback->Call(handle(), 0, argv);
            }
        } else if (new_state == webrtc::MediaStreamTrackInterface::kLive) {
            Local<Function> callback = Nan::New<Function>(_onstarted);
            Local<Value> argv[1];

            if (!callback.IsEmpty() && callback->IsFunction()) {
                callback->Call(handle(), 0, argv);
            }
        }
    }

    if (_source_state != new_source) {
        if (new_source == webrtc::MediaSourceInterface::kMuted) {
            Local<Function> callback = Nan::New<Function>(_onmute);
            Local<Value> argv[1];

            if (!callback.IsEmpty() && callback->IsFunction()) {
                callback->Call(handle(), 0, argv);
            }
        } else {
            Local<Function> callback = Nan::New<Function>(_onunmute);
            Local<Value> argv[1];

            if (!callback.IsEmpty() && callback->IsFunction()) {
                callback->Call(handle(), 0, argv);
            }
        }
    }

    _source_state = new_source;
    _track_state = new_state;
}

void MediaStreamTrack::On(Event* event)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    Nan::HandleScope scope;
    MediaStreamTrackEvent type = event->Type<MediaStreamTrackEvent>();

    if (type != kMediaStreamTrackChanged) {
        return;
    }

    MediaStreamTrack::CheckState();
}

void MediaStreamTrack::OnChanged() {
    Emit(kMediaStreamTrackChanged);
}
