#include "RTCPeerConnection.h"
#include "RTCDataChannel.h"
#include "MediaStream.h"
#include "MediaStreamTrack.h"
#include "Platform.h"
#include "RTCStatsReport.h"
#include "RTCStatsResponse.h"
#include "RTCRtpTransceiver.h"
#include "RTCRtpReceiver.h"

using namespace v8;
using namespace WebRTC;
using namespace rtc;

static Nan::Persistent<v8::Function> constructor;

NAN_MODULE_INIT(RTCPeerConnection::Init)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    Nan::HandleScope scope;

    Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>(New);
    tpl->InstanceTemplate()->SetInternalFieldCount(1);
    tpl->SetClassName(Nan::New("RTCPeerConnection").ToLocalChecked());

    Nan::SetPrototypeMethod(tpl, "createOffer", CreateOffer);
    Nan::SetPrototypeMethod(tpl, "createAnswer", CreateAnswer);
    Nan::SetPrototypeMethod(tpl, "setLocalDescription", SetLocalDescription);
    Nan::SetPrototypeMethod(tpl, "setRemoteDescription", SetRemoteDescription);
    Nan::SetPrototypeMethod(tpl, "addIceCandidate", AddIceCandidate);
    Nan::SetPrototypeMethod(tpl, "createDataChannel", CreateDataChannel);
    Nan::SetPrototypeMethod(tpl, "addStream", AddStream);
    Nan::SetPrototypeMethod(tpl, "removeStream", RemoveStream);
    Nan::SetPrototypeMethod(tpl, "getLocalStreams", GetLocalStreams);
    Nan::SetPrototypeMethod(tpl, "getRemoteStreams", GetRemoteStreams);
    Nan::SetPrototypeMethod(tpl, "getStreamById", GetStreamById);
    Nan::SetPrototypeMethod(tpl, "getStats", GetStats);
    Nan::SetPrototypeMethod(tpl, "getConfiguration", GetConfiguration);
    Nan::SetPrototypeMethod(tpl, "close", Close);

    Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("signalingState").ToLocalChecked(), GetSignalingState);
    Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("iceConnectionState").ToLocalChecked(), GetIceConnectionState);
    Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("iceGatheringState").ToLocalChecked(), GetIceGatheringState);
    Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("localDescription").ToLocalChecked(), GetLocalDescription);
    Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("remoteDescription").ToLocalChecked(), GetRemoteDescription);
    Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("onsignalingstatechange").ToLocalChecked(), GetOnSignalingStateChange, SetOnSignalingStateChange);
    Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("oniceconnectionstatechange").ToLocalChecked(), GetOnIceConnectionStateChange, SetOnIceConnectionStateChange);
    Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("onicecandidate").ToLocalChecked(), GetOnIceCandidate, SetOnIceCandidate);
    Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("ondatachannel").ToLocalChecked(), GetOnDataChannel, SetOnDataChannel);
    Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("onnegotiationneeded").ToLocalChecked(), GetOnNegotiationNeeded, SetOnNegotiationNeeded);
    Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("onaddstream").ToLocalChecked(), GetOnAddStream, SetOnAddStream);
    Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("ontrack").ToLocalChecked(), GetOnTrack, SetOnTrack);
    Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("onremovestream").ToLocalChecked(), GetOnRemoveStream, SetOnRemoveStream);

    constructor.Reset<Function>(tpl->GetFunction());
    Nan::Set(target, Nan::New("RTCPeerConnection").ToLocalChecked(), tpl->GetFunction());
}

RTCPeerConnection::RTCPeerConnection(const Local<Object>& configuration) :
    _config(webrtc::PeerConnectionInterface::RTCConfigurationType::kSafe)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    if (!configuration.IsEmpty()) {
        Local<Value> iceservers_value = configuration->Get(Nan::New("iceServers").ToLocalChecked());

        if (!iceservers_value.IsEmpty() && iceservers_value->IsArray()) {
            Local<Array> list = Local<Array>::Cast(iceservers_value);

            for (unsigned int index = 0; index < list->Length(); index++) {
                Local<Value> server_value = list->Get(index);

                if (!server_value.IsEmpty() && server_value->IsObject()) {
                    Local<Object> server = Local<Object>::Cast(server_value);
                    Local<Value> url_value = server->Get(Nan::New("url").ToLocalChecked());
                    Local<Value> urls_value = server->Get(Nan::New("urls").ToLocalChecked());
                    Local<Value> username_value = server->Get(Nan::New("username").ToLocalChecked());
                    Local<Value> credential_value = server->Get(Nan::New("credential").ToLocalChecked());
                    webrtc::PeerConnectionInterface::IceServer entry;

                    if (!url_value.IsEmpty() && url_value->IsString()) {
                        v8::String::Utf8Value url(Isolate::GetCurrent(), url_value->ToString());

                        entry.urls.push_back(*url);

                        if (!username_value.IsEmpty() && username_value->IsString()) {
                            String::Utf8Value username(Isolate::GetCurrent(), username_value->ToString());
                            entry.username = *username;
                        }

                        if (!credential_value.IsEmpty() && credential_value->IsString()) {
                            String::Utf8Value credential(Isolate::GetCurrent(), credential_value->ToString());
                            entry.password = *credential;
                        }
                    }

                    if (!urls_value.IsEmpty()) {
                        if (urls_value->IsString()) {
                            String::Utf8Value url(Isolate::GetCurrent(), urls_value->ToString());
                            entry.urls.push_back(*url);
                        }  if (urls_value->IsArray()) {
                            Local<Array> urls = Local<Array>::Cast(urls_value);
                            for (uint32_t i = 0; i < urls->Length(); i++) {
                                String::Utf8Value url(Isolate::GetCurrent(), urls->Get(i));
                                entry.urls.push_back(*url);
                            }
                        }

                        if (!username_value.IsEmpty() && username_value->IsString()) {
                            String::Utf8Value username(Isolate::GetCurrent(), username_value->ToString());
                            entry.username = *username;
                        }

                        if (!credential_value.IsEmpty() && credential_value->IsString()) {
                            String::Utf8Value credential(Isolate::GetCurrent(), credential_value->ToString());
                            entry.password = *credential;
                        }
                    }

                    if (!entry.urls.empty()) {
                        _config.servers.push_back(entry);
                    }
                }
            }
        }

        // explicitly use unified plan for now to allow OnTrack to be called
        // _config.sdp_semantics = webrtc::SdpSemantics::kUnifiedPlan;

        Local<Value> sdpsemantics_value = configuration->Get(Nan::New("sdpSemantics").ToLocalChecked());
        if (!sdpsemantics_value.IsEmpty() && sdpsemantics_value->IsString()) {
            v8::String::Utf8Value sdpsemantics(Isolate::GetCurrent(), sdpsemantics_value->ToString());
            std::string sdpsemantics_str = *sdpsemantics;
            if (sdpsemantics_str == "plan-b") {
                _config.sdp_semantics = webrtc::SdpSemantics::kPlanB;
            } else if (sdpsemantics_str == "unified-plan") {
                _config.sdp_semantics = webrtc::SdpSemantics::kUnifiedPlan;
            }
        }
    }

    _stats = new rtc::RefCountedObject<StatsObserver>(this);
    _offer = new rtc::RefCountedObject<OfferObserver>(this);
    _answer = new rtc::RefCountedObject<AnswerObserver>(this);
    _local = new rtc::RefCountedObject<LocalDescriptionObserver>(this);
    _remote = new rtc::RefCountedObject<RemoteDescriptionObserver>(this);
    _peer = new rtc::RefCountedObject<RTCPeerConnectionObserver>(this);
    _factory = Platform::GetFactory();
}

RTCPeerConnection::~RTCPeerConnection()
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    if (_socket.get()) {
        webrtc::PeerConnectionInterface::SignalingState state(_socket->signaling_state());

        if (state != webrtc::PeerConnectionInterface::kClosed) {
            _socket->Close();
        }
    }

    _stats->RemoveListener(this);
    _offer->RemoveListener(this);
    _answer->RemoveListener(this);
    _local->RemoveListener(this);
    _remote->RemoveListener(this);
    _peer->RemoveListener(this);
}

webrtc::PeerConnectionInterface* RTCPeerConnection::GetSocket()
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    if (!_socket.get()) {
        if (_factory.get()) {
            EventEmitter::SetReference(true);
            webrtc::PeerConnectionDependencies dependencies(_peer.get());
            _socket = _factory->CreatePeerConnection(_config, std::move(dependencies));

            if (!_socket.get()) {
                Nan::ThrowError("Internal Socket Error");
            }
        } else {
            Nan::ThrowError("Internal Factory Error");
        }
    }

    return _socket.get();
}

NAN_METHOD(RTCPeerConnection::New)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    Local<Object> configuration;

    if (info.Length() >= 1 && info[0]->IsObject()) {
        configuration = Local<Object>::Cast(info[0]);
    }

    // setup default configuration
    if (configuration.IsEmpty()) {
        RTC_LOG(LS_INFO) << "Using default config";
        static const char *json_str = "{ \"iceServers\": [{ \"url\": \"stun:stun.l.google.com:19302\" }] }";
        configuration = Local<Object>::Cast(JSON::Parse(Nan::GetCurrentContext(), Nan::New(json_str).ToLocalChecked()).ToLocalChecked());
    }

    if (info.IsConstructCall()) {
        RTCPeerConnection* peer = new RTCPeerConnection(configuration);
        peer->Wrap(info.This());
        return info.GetReturnValue().Set(info.This());
    } else {
        const int argc = 1;
        Local<Value> argv[argc] = {
            configuration,
        };

        Local<Function> instance = Nan::New(constructor);
        return info.GetReturnValue().Set(instance->NewInstance(Nan::GetCurrentContext(), argc, argv).ToLocalChecked());
    }
}

NAN_METHOD(RTCPeerConnection::CreateOffer)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    RTCPeerConnection* self = Unwrap<RTCPeerConnection>(info.This());
    webrtc::PeerConnectionInterface* socket = self->GetSocket();

    if (!info[0].IsEmpty() && info[0]->IsFunction()) {
        self->_offerCallback.Reset<Function>(Local<Function>::Cast(info[0]));
    } else {
        self->_offerCallback.Reset();
    }

    if (!info[1].IsEmpty() && info[1]->IsFunction()) {
        self->_offerErrorCallback.Reset<Function>(Local<Function>::Cast(info[1]));
    } else {
        self->_offerErrorCallback.Reset();
    }

    Local<Value> options_value;
    if (!info[0].IsEmpty() && info[0]->IsObject()) {
        options_value = info[0];
    } else if (!info[2].IsEmpty() && info[2]->IsObject()) {
        options_value = info[2];
    }

    if (socket) {
        webrtc::PeerConnectionInterface::RTCOfferAnswerOptions options;
        rtc::scoped_refptr<MediaConstraints> constraints = MediaConstraints::New(options_value);
        CopyConstraintsIntoOfferAnswerOptions(constraints->ToConstraints(), &options);
        socket->CreateOffer(self->_offer.get(), options);
    } else {
        Nan::ThrowError("Internal Error");
    }

    // return a promise
    Local<Promise::Resolver> resolver = Promise::Resolver::New(Nan::GetCurrentContext()).ToLocalChecked();
    self->_offerResolver.Reset(resolver);
    info.GetReturnValue().Set(resolver->GetPromise());
}

NAN_METHOD(RTCPeerConnection::CreateAnswer)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    RTCPeerConnection* self = Unwrap<RTCPeerConnection>(info.This());
    webrtc::PeerConnectionInterface* socket = self->GetSocket();

    if (!info[0].IsEmpty() && info[0]->IsFunction()) {
        self->_answerCallback.Reset<Function>(Local<Function>::Cast(info[0]));
    } else {
        self->_answerCallback.Reset();
    }

    if (!info[1].IsEmpty() && info[1]->IsFunction()) {
        self->_answerErrorCallback.Reset<Function>(Local<Function>::Cast(info[1]));
    } else {
        self->_answerErrorCallback.Reset();
    }

    Local<Value> options_value;
    if (!info[0].IsEmpty() && info[0]->IsObject()) {
        options_value = info[0];
    } else if (!info[2].IsEmpty() && info[2]->IsObject()) {
        options_value = info[2];
    }

    if (socket) {
        webrtc::PeerConnectionInterface::RTCOfferAnswerOptions options;
        rtc::scoped_refptr<MediaConstraints> constraints = MediaConstraints::New(options_value);
        CopyConstraintsIntoOfferAnswerOptions(constraints->ToConstraints(), &options);
        socket->CreateAnswer(self->_answer.get(), options);
    } else {
        Nan::ThrowError("Internal Error");
    }

    // return a promise
    Local<Promise::Resolver> resolver = Promise::Resolver::New(Nan::GetCurrentContext()).ToLocalChecked();
    self->_answerResolver.Reset(resolver);
    info.GetReturnValue().Set(resolver->GetPromise());
}

NAN_METHOD(RTCPeerConnection::SetLocalDescription)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    RTCPeerConnection* self = Unwrap<RTCPeerConnection>(info.This());
    webrtc::PeerConnectionInterface* socket = self->GetSocket();
    const char* error = "Invalid SessionDescription";

    if (!info[0].IsEmpty() && info[0]->IsObject()) {
        Local<Object> desc_obj = Local<Object>::Cast(info[0]);
        Local<Value> type_value = desc_obj->Get(Nan::New("type").ToLocalChecked());
        Local<Value> sdp_value = desc_obj->Get(Nan::New("sdp").ToLocalChecked());

        if (!type_value.IsEmpty() && type_value->IsString()) {
            if (!sdp_value.IsEmpty() && sdp_value->IsString()) {
                if (!info[1].IsEmpty() && info[1]->IsFunction()) {
                    self->_localCallback.Reset<Function>(Local<Function>::Cast(info[1]));
                } else {
                    self->_localCallback.Reset();
                }

                if (!info[2].IsEmpty() && info[2]->IsFunction()) {
                    self->_localErrorCallback.Reset<Function>(Local<Function>::Cast(info[2]));
                } else {
                    self->_localErrorCallback.Reset();
                }

                String::Utf8Value type(Isolate::GetCurrent(), type_value->ToString());
                String::Utf8Value sdp(Isolate::GetCurrent(), sdp_value->ToString());

                webrtc::SessionDescriptionInterface* desc(webrtc::CreateSessionDescription(*type, *sdp, 0));

                if (desc) {
                    if (socket) {
                        self->_localsdp.Reset<Object>(desc_obj);
                        socket->SetLocalDescription(self->_local.get(), desc);
                        error = 0;
                    } else {
                        error = "Internal Error";
                    }
                }
            }
        }
    }

    if (error) {
        if (!info[2].IsEmpty() && info[2]->IsFunction()) {
            Local<Value> argv[1] = {
                Nan::Error(error)
            };

            Local<Function> onerror = Local<Function>::Cast(info[2]);
            onerror->Call(info.This(), 1, argv);
        } else {
            Nan::ThrowError(error);
        }
    }

    Local<Promise::Resolver> resolver = Promise::Resolver::New(Nan::GetCurrentContext()).ToLocalChecked();
    self->_localResolver.Reset(resolver);
    info.GetReturnValue().Set(resolver->GetPromise());
}

NAN_METHOD(RTCPeerConnection::SetRemoteDescription)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    RTCPeerConnection* self = Unwrap<RTCPeerConnection>(info.This());
    webrtc::PeerConnectionInterface* socket = self->GetSocket();
    const char* error = "Invalid SessionDescription";

    if (!info[0].IsEmpty() && info[0]->IsObject()) {
        Local<Object> desc_obj = Local<Object>::Cast(info[0]);
        Local<Value> type_value = desc_obj->Get(Nan::New("type").ToLocalChecked());
        Local<Value> sdp_value = desc_obj->Get(Nan::New("sdp").ToLocalChecked());

        if (!type_value.IsEmpty() && type_value->IsString()) {
            if (!sdp_value.IsEmpty() && sdp_value->IsString()) {
                if (!info[1].IsEmpty() && info[1]->IsFunction()) {
                    self->_remoteCallback.Reset<Function>(Local<Function>::Cast(info[1]));
                } else {
                    self->_remoteCallback.Reset();
                }

                if (!info[2].IsEmpty() && info[2]->IsFunction()) {
                    self->_remoteErrorCallback.Reset<Function>(Local<Function>::Cast(info[2]));
                } else {
                    self->_remoteErrorCallback.Reset();
                }

                String::Utf8Value type(Isolate::GetCurrent(), type_value->ToString());
                String::Utf8Value sdp(Isolate::GetCurrent(), sdp_value->ToString());

                webrtc::SessionDescriptionInterface* desc(webrtc::CreateSessionDescription(*type, *sdp, 0));

                if (desc) {
                    if (socket) {
                        self->_remotesdp.Reset<Object>(desc_obj);
                        socket->SetRemoteDescription(self->_remote.get(), desc);
                        error = 0;
                    } else {
                        error = "Internal Error";
                    }
                }
            }
        }
    }

    if (error) {
        if (!info[2].IsEmpty() && info[2]->IsFunction()) {
            Local<Value> argv[1] = {
                Nan::Error(error)
            };

            Local<Function> onerror = Local<Function>::Cast(info[2]);
            onerror->Call(info.This(), 1, argv);
        } else {
            Nan::ThrowError(error);
        }
    }

    Local<Promise::Resolver> resolver = Promise::Resolver::New(Nan::GetCurrentContext()).ToLocalChecked();
    self->_remoteResolver.Reset(resolver);
    info.GetReturnValue().Set(resolver->GetPromise());
}

NAN_METHOD(RTCPeerConnection::AddIceCandidate)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    RTCPeerConnection* self = Unwrap<RTCPeerConnection>(info.This());
    webrtc::PeerConnectionInterface* socket = self->GetSocket();

    const char* error = 0;
    Local<Value> argv[1];

    if (!info[0].IsEmpty() && info[0]->IsObject()) {
        Local<Object> desc = Local<Object>::Cast(info[0]);
        Local<Value> sdpMid_value = desc->Get(Nan::New("sdpMid").ToLocalChecked());
        Local<Value> sdpMLineIndex_value = desc->Get(Nan::New("sdpMLineIndex").ToLocalChecked());
        Local<Value> sdp_value = desc->Get(Nan::New("candidate").ToLocalChecked());

        if (!sdpMid_value.IsEmpty() && sdpMid_value->IsString()) {
            if (!sdpMLineIndex_value.IsEmpty() && sdpMLineIndex_value->IsInt32()) {
                if (!sdp_value.IsEmpty() && sdp_value->IsString()) {
                    Local<Int32> sdpMLineIndex(sdpMLineIndex_value->ToInt32(Nan::GetCurrentContext()).ToLocalChecked());
                    String::Utf8Value sdpMid(Isolate::GetCurrent(), sdpMid_value->ToString());
                    String::Utf8Value sdp(Isolate::GetCurrent(), sdp_value->ToString());

                    std::unique_ptr<webrtc::IceCandidateInterface> candidate(webrtc::CreateIceCandidate(*sdpMid, sdpMLineIndex->Value(), *sdp, 0));

                    if (candidate) {
                        if (socket) {
                            if (socket->AddIceCandidate(candidate.get())) {
                                if (!info[1].IsEmpty() && info[1]->IsFunction()) {
                                    Local<Function> success = Local<Function>::Cast(info[1]);
                                    success->Call(info.This(), 0, argv);
                                }
                            } else {
                                error = "Failed to add ICECandidate";
                            }
                        } else {
                            error = "Internal Error";
                        }
                    } else {
                        error = "Invalid ICECandidate";
                    }
                } else {
                    error = "Invalid candidate";
                }
            } else {
                error = "Invalid sdpMLineIndex";
            }
        } else {
            error = "Invalid sdpMid";
        }
    }

    if (error) {
        if (!info[2].IsEmpty() && info[2]->IsFunction()) {
            argv[0] = Nan::Error(error);

            Local<Function> onerror = Local<Function>::Cast(info[2]);
            onerror->Call(info.This(), 1, argv);
        } else {
            Nan::ThrowError(error);
        }
    }

    info.GetReturnValue().SetUndefined();
}

NAN_METHOD(RTCPeerConnection::CreateDataChannel)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    RTCPeerConnection* self = Unwrap<RTCPeerConnection>(info.This());
    webrtc::PeerConnectionInterface* socket = self->GetSocket();

    std::string label;
    webrtc::DataChannelInit config;

    if (!info[0].IsEmpty() && info[0]->IsString()) {
        String::Utf8Value label_utf8(Isolate::GetCurrent(), info[0]->ToString());
        label = *label_utf8;
    }

    if (!info[1].IsEmpty() && info[1]->IsObject()) {
        Local<Object> config_obj = Local<Object>::Cast(info[0]);

        Local<Value> reliable_value = config_obj->Get(Nan::New("reliable").ToLocalChecked());
        Local<Value> ordered_value = config_obj->Get(Nan::New("ordered").ToLocalChecked());
        Local<Value> maxRetransmitTime_value = config_obj->Get(Nan::New("maxRetransmitTime").ToLocalChecked());
        Local<Value> maxRetransmits_value = config_obj->Get(Nan::New("maxRetransmits").ToLocalChecked());
        Local<Value> protocol_value = config_obj->Get(Nan::New("protocol").ToLocalChecked());
        Local<Value> id_value = config_obj->Get(Nan::New("id").ToLocalChecked());

        if (!reliable_value.IsEmpty()) {
            if (reliable_value->IsTrue()) {
                config.reliable = true;
            } else {
                config.reliable = false;
            }
        }

        if (!ordered_value.IsEmpty()) {
            if (ordered_value->IsTrue()) {
                config.ordered = true;
            } else {
                config.ordered = false;
            }
        }

        if (!maxRetransmitTime_value.IsEmpty() && maxRetransmitTime_value->IsInt32()) {
            Local<Int32> maxRetransmitTime(maxRetransmitTime_value->ToInt32(Nan::GetCurrentContext()).ToLocalChecked());
            config.maxRetransmitTime = maxRetransmitTime->Value();
        }

        if (!maxRetransmits_value.IsEmpty() && maxRetransmits_value->IsInt32()) {
            Local<Int32> maxRetransmits(maxRetransmits_value->ToInt32(Nan::GetCurrentContext()).ToLocalChecked());
            config.maxRetransmits = maxRetransmits->Value();
        }

        if (!protocol_value.IsEmpty() && protocol_value->IsString()) {
            String::Utf8Value protocol(Isolate::GetCurrent(), protocol_value->ToString());
            config.protocol = *protocol;
        }

        if (!id_value.IsEmpty() && id_value->IsInt32()) {
            Local<Int32> id(id_value->ToInt32(Nan::GetCurrentContext()).ToLocalChecked());
            config.id = id->Value();
        }
    }

    if (socket) {
        rtc::scoped_refptr<webrtc::DataChannelInterface> dataChannel = socket->CreateDataChannel(label, &config);

        if (dataChannel.get()) {
            return info.GetReturnValue().Set(RTCDataChannel::New(dataChannel));
        }
    }

    Nan::ThrowError("Internal Error");
    info.GetReturnValue().SetUndefined();
}

NAN_METHOD(RTCPeerConnection::AddStream)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    RTCPeerConnection* self = Unwrap<RTCPeerConnection>(info.This());
    rtc::scoped_refptr<webrtc::MediaStreamInterface> mediaStream = MediaStream::Unwrap(info[0]);

    if (mediaStream.get()) {
        webrtc::PeerConnectionInterface* socket = self->GetSocket();

        if (socket) {
            auto config = socket->GetConfiguration();
            if (config.sdp_semantics == webrtc::SdpSemantics::kUnifiedPlan)
            {
                auto audio = mediaStream->GetAudioTracks();
                std::vector<std::string> stream_ids;
                stream_ids.push_back(mediaStream->id());
                for (size_t i = 0; i < audio.size(); i++) {
                    socket->AddTrack(audio[i], stream_ids);
                }

                auto video = mediaStream->GetVideoTracks();
                for (size_t i = 0; i < video.size(); i++) {
                    socket->AddTrack(video[i], stream_ids);
                }
            } else {
                if (!socket->AddStream(mediaStream)) {
                    Nan::ThrowError("AddStream Failed");
                }
            }
        } else {
            Nan::ThrowError("Internal Error");
        }
    } else {
        Nan::ThrowError("Invalid MediaStream Object");
    }

    info.GetReturnValue().SetUndefined();
}

NAN_METHOD(RTCPeerConnection::RemoveStream)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    RTCPeerConnection* self = Unwrap<RTCPeerConnection>(info.This());
    scoped_refptr<webrtc::MediaStreamInterface> mediaStream = MediaStream::Unwrap(info[0]);

    if (mediaStream.get()) {
        webrtc::PeerConnectionInterface* socket = self->GetSocket();

        if (socket) {
            socket->RemoveStream(mediaStream);
        } else {
            Nan::ThrowError("Internal Error");
        }
    } else {
        Nan::ThrowError("Invalid MediaStream Object");
    }

    info.GetReturnValue().SetUndefined();
}

NAN_METHOD(RTCPeerConnection::GetLocalStreams)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    RTCPeerConnection* self = Unwrap<RTCPeerConnection>(info.This());
    webrtc::PeerConnectionInterface* socket = self->GetSocket();

    if (socket) {
        rtc::scoped_refptr<webrtc::StreamCollectionInterface> local = socket->local_streams();

        if (local.get()) {
            Local<Array> list = Nan::New<Array>();
            uint32_t index = 0;
            size_t count;

            for (count = 0; count < local->count(); count++) {
                list->Set(index, MediaStream::New(local->at(count)));
            }

            return info.GetReturnValue().Set(list);
        }
    }

    Nan::ThrowError("Internal Error");
    info.GetReturnValue().SetUndefined();
}

NAN_METHOD(RTCPeerConnection::GetRemoteStreams)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    RTCPeerConnection* self = Unwrap<RTCPeerConnection>(info.This());
    webrtc::PeerConnectionInterface* socket = self->GetSocket();

    if (socket) {
        rtc::scoped_refptr<webrtc::StreamCollectionInterface> remote = socket->remote_streams();

        if (remote.get()) {
            Local<Array> list = Nan::New<Array>();
            uint32_t index = 0;
            size_t count;

            for (count = 0; count < remote->count(); count++) {
                list->Set(index, MediaStream::New(remote->at(count)));
            }

            return info.GetReturnValue().Set(list);
        }
    }

    Nan::ThrowError("Internal Error");
    info.GetReturnValue().SetUndefined();
}

NAN_METHOD(RTCPeerConnection::GetStreamById)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    RTCPeerConnection* self = Unwrap<RTCPeerConnection>(info.This());
    webrtc::PeerConnectionInterface* socket = self->GetSocket();

    if (socket) {
        if (info.Length() >= 1 && info[0]->IsString()) {
            v8::String::Utf8Value idValue(Isolate::GetCurrent(), info[0]->ToString());
            std::string id(*idValue);

            rtc::scoped_refptr<webrtc::StreamCollectionInterface> local = socket->local_streams();
            rtc::scoped_refptr<webrtc::StreamCollectionInterface> remote = socket->remote_streams();
            rtc::scoped_refptr<webrtc::MediaStreamInterface> stream;

            if (local.get()) {
                stream = local->find(id);
            }

            if (remote.get() && !stream.get()) {
                stream = remote->find(id);
            }

            if (stream.get()) {
                return info.GetReturnValue().Set(MediaStream::New(stream));
            } else {
                return info.GetReturnValue().Set(Nan::Null());
            }
        } else {
            Nan::ThrowError("Invalid Argument");
        }
    }

    Nan::ThrowError("Internal Error");
    info.GetReturnValue().SetUndefined();
}

NAN_METHOD(RTCPeerConnection::GetStats)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    RTCPeerConnection* self = Unwrap<RTCPeerConnection>(info.This());
    webrtc::PeerConnectionInterface* socket = self->GetSocket();

    if (!info[0].IsEmpty() && info[0]->IsFunction()) {
        self->_onstats.Reset<Function>(Local<Function>::Cast(info[0]));

        if (socket) {
            if (!socket->GetStats(self->_stats.get(), 0, webrtc::PeerConnectionInterface::kStatsOutputLevelStandard)) {
                Local<Function> callback = Nan::New<Function>(self->_onstats);
                Local<Value> argv[1] = { Nan::Null() };

                callback->Call(info.This(), 1, argv);
                self->_onstats.Reset();
            }
        } else {
            Nan::ThrowError("Internal Error");
        }
    } else {
        Nan::ThrowError("Missing Callback");
    }

    info.GetReturnValue().SetUndefined();
}

NAN_METHOD(RTCPeerConnection::GetConfiguration)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    RTCPeerConnection* self = Unwrap<RTCPeerConnection>(info.This());
    webrtc::PeerConnectionInterface* socket = self->GetSocket();

    if (socket) {
        webrtc::PeerConnectionInterface::RTCConfiguration config = socket->GetConfiguration();
        Local<Object> object = Nan::New<Object>();

        std::string bundlePolicy;
        switch (config.bundle_policy) {
            case webrtc::PeerConnectionInterface::kBundlePolicyBalanced:
                bundlePolicy = "balanced";
                break;
            case webrtc::PeerConnectionInterface::kBundlePolicyMaxBundle:
                bundlePolicy = "max-bundle";
                break;
            case webrtc::PeerConnectionInterface::kBundlePolicyMaxCompat:
                bundlePolicy = "max-compat";
                break;
        }

        std::string iceTransportPolicy;
        switch (config.type) {
            case webrtc::PeerConnectionInterface::kNone:
                iceTransportPolicy = "none";
                break;
            case webrtc::PeerConnectionInterface::kRelay:
                iceTransportPolicy = "relay";
                break;
            case webrtc::PeerConnectionInterface::kNoHost:
                iceTransportPolicy = "no-host";
                break;
            case webrtc::PeerConnectionInterface::kAll:
                iceTransportPolicy = "all";
                break;
        }

        std::string rtcpMuxPolicy;
        switch (config.rtcp_mux_policy) {
            case webrtc::PeerConnectionInterface::kRtcpMuxPolicyNegotiate:
                rtcpMuxPolicy = "negotiate";
                break;
            case webrtc::PeerConnectionInterface::kRtcpMuxPolicyRequire:
                rtcpMuxPolicy = "require";
                break;
        }

        std::string sdpSemantics;
        switch (config.sdp_semantics) {
            case webrtc::SdpSemantics::kPlanB:
                sdpSemantics = "plan-b";
                break;
            case webrtc::SdpSemantics::kUnifiedPlan:
                sdpSemantics = "unified-plan";
                break;
        }

        Nan::Set(object, Nan::New("bundlePolicy").ToLocalChecked(), Nan::New(bundlePolicy.c_str()).ToLocalChecked());
        Nan::Set(object, Nan::New("iceCandidatePoolSize").ToLocalChecked(), Nan::New(config.ice_candidate_pool_size));
        Nan::Set(object, Nan::New("iceTransportPolicy").ToLocalChecked(), Nan::New(iceTransportPolicy.c_str()).ToLocalChecked());
        Nan::Set(object, Nan::New("rtcpMuxPolicy").ToLocalChecked(), Nan::New(rtcpMuxPolicy.c_str()).ToLocalChecked());
        Nan::Set(object, Nan::New("sdpSemantics").ToLocalChecked(), Nan::New(sdpSemantics.c_str()).ToLocalChecked());

        Local<Array> iceServers = Nan::New<Array>();
        for (size_t i = 0; i < config.servers.size(); i++) {
            Local<Object> iceServer = Nan::New<Object>();
            const webrtc::PeerConnectionInterface::IceServer &server = config.servers[i];
            if (!server.username.empty()) {
                Nan::Set(iceServer, Nan::New("username").ToLocalChecked(), Nan::New(server.username.c_str()).ToLocalChecked());
            }
            if (!server.password.empty()) {
                Nan::Set(iceServer, Nan::New("credentials").ToLocalChecked(), Nan::New(server.username.c_str()).ToLocalChecked());
            }

            Local<Array> urls_array = Nan::New<Array>();
            Nan::Set(iceServer, Nan::New("urls").ToLocalChecked(), urls_array);
            for (size_t i = 0; i < server.urls.size(); i++) {
                urls_array->Set(i, Nan::New(server.urls[i].c_str()).ToLocalChecked());
            }

            iceServers->Set(i, iceServer);
        }
        Nan::Set(object, Nan::New("iceServers").ToLocalChecked(), iceServers);

        info.GetReturnValue().Set(object);
    } else {
        Nan::ThrowError("Internal Error");
        info.GetReturnValue().SetUndefined();
    }
}

NAN_METHOD(RTCPeerConnection::Close)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    RTCPeerConnection* self = Unwrap<RTCPeerConnection>(info.This());
    webrtc::PeerConnectionInterface* socket = self->GetSocket();

    if (socket) {
        socket->Close();
    } else {
        Nan::ThrowError("Internal Error");
    }

    info.GetReturnValue().SetUndefined();
}

NAN_GETTER(RTCPeerConnection::GetSignalingState)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    RTCPeerConnection* self = Unwrap<RTCPeerConnection>(info.Holder());
    webrtc::PeerConnectionInterface* socket = self->GetSocket();

    if (socket) {
        webrtc::PeerConnectionInterface::SignalingState state(socket->signaling_state());

        switch (state) {
        case webrtc::PeerConnectionInterface::kStable:
            return info.GetReturnValue().Set(Nan::New("stable").ToLocalChecked());
            break;
        case webrtc::PeerConnectionInterface::kHaveLocalOffer:
            return info.GetReturnValue().Set(Nan::New("have-local-offer").ToLocalChecked());
            break;
        case webrtc::PeerConnectionInterface::kHaveLocalPrAnswer:
            return info.GetReturnValue().Set(Nan::New("have-local-pranswer").ToLocalChecked());
            break;
        case webrtc::PeerConnectionInterface::kHaveRemoteOffer:
            return info.GetReturnValue().Set(Nan::New("have-remote-offer").ToLocalChecked());
            break;
        case webrtc::PeerConnectionInterface::kHaveRemotePrAnswer:
            return info.GetReturnValue().Set(Nan::New("have-remote-pranswer").ToLocalChecked());
            break;
        default:
            return info.GetReturnValue().Set(Nan::New("closed").ToLocalChecked());
            break;
        }
    } else {
        Nan::ThrowError("Internal Error");
    }

    info.GetReturnValue().SetUndefined();
}

NAN_GETTER(RTCPeerConnection::GetIceConnectionState)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    RTCPeerConnection* self = Unwrap<RTCPeerConnection>(info.Holder());
    webrtc::PeerConnectionInterface* socket = self->GetSocket();

    if (socket) {
        webrtc::PeerConnectionInterface::IceConnectionState state(socket->ice_connection_state());

        switch (state) {
        case webrtc::PeerConnectionInterface::kIceConnectionNew:
            return info.GetReturnValue().Set(Nan::New("new").ToLocalChecked());
            break;
        case webrtc::PeerConnectionInterface::kIceConnectionChecking:
            return info.GetReturnValue().Set(Nan::New("checking").ToLocalChecked());
            break;
        case webrtc::PeerConnectionInterface::kIceConnectionConnected:
            return info.GetReturnValue().Set(Nan::New("connected").ToLocalChecked());
            break;
        case webrtc::PeerConnectionInterface::kIceConnectionCompleted:
            return info.GetReturnValue().Set(Nan::New("completed").ToLocalChecked());
            break;
        case webrtc::PeerConnectionInterface::kIceConnectionFailed:
            return info.GetReturnValue().Set(Nan::New("failed").ToLocalChecked());
            break;
        case webrtc::PeerConnectionInterface::kIceConnectionDisconnected:
            return info.GetReturnValue().Set(Nan::New("disconnected").ToLocalChecked());
            break;
        default:
            return info.GetReturnValue().Set(Nan::New("closed").ToLocalChecked());
            break;
        }
    } else {
        Nan::ThrowError("Internal Error");
    }

    info.GetReturnValue().SetUndefined();
}

NAN_GETTER(RTCPeerConnection::GetIceGatheringState)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    RTCPeerConnection* self = Unwrap<RTCPeerConnection>(info.Holder());
    webrtc::PeerConnectionInterface* socket = self->GetSocket();

    if (socket) {
        webrtc::PeerConnectionInterface::IceGatheringState state(socket->ice_gathering_state());

        switch (state) {
        case webrtc::PeerConnectionInterface::kIceGatheringNew:
            return info.GetReturnValue().Set(Nan::New("new").ToLocalChecked());
            break;
        case webrtc::PeerConnectionInterface::kIceGatheringGathering:
            return info.GetReturnValue().Set(Nan::New("gathering").ToLocalChecked());
            break;
        default:
            return info.GetReturnValue().Set(Nan::New("complete").ToLocalChecked());
            break;
        }
    } else {
        Nan::ThrowError("Internal Error");
    }

    info.GetReturnValue().SetUndefined();
}

NAN_GETTER(RTCPeerConnection::GetOnSignalingStateChange)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    RTCPeerConnection* self = Unwrap<RTCPeerConnection>(info.Holder());
    return info.GetReturnValue().Set(Nan::New<Function>(self->_onsignalingstatechange));
}

NAN_GETTER(RTCPeerConnection::GetOnIceConnectionStateChange)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    RTCPeerConnection* self = Unwrap<RTCPeerConnection>(info.Holder());
    return info.GetReturnValue().Set(Nan::New<Function>(self->_oniceconnectionstatechange));
}

NAN_GETTER(RTCPeerConnection::GetOnIceCandidate)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    RTCPeerConnection* self = Unwrap<RTCPeerConnection>(info.Holder());
    return info.GetReturnValue().Set(Nan::New<Function>(self->_onicecandidate));
}

NAN_GETTER(RTCPeerConnection::GetLocalDescription)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    RTCPeerConnection* self = Unwrap<RTCPeerConnection>(info.Holder());
    return info.GetReturnValue().Set(Nan::New<Object>(self->_localsdp));
}

NAN_GETTER(RTCPeerConnection::GetRemoteDescription)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    RTCPeerConnection* self = Unwrap<RTCPeerConnection>(info.Holder());
    return info.GetReturnValue().Set(Nan::New<Object>(self->_remotesdp));
}

NAN_GETTER(RTCPeerConnection::GetOnDataChannel)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    RTCPeerConnection* self = Unwrap<RTCPeerConnection>(info.Holder());
    return info.GetReturnValue().Set(Nan::New<Function>(self->_ondatachannel));
}

NAN_GETTER(RTCPeerConnection::GetOnNegotiationNeeded)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    RTCPeerConnection* self = Unwrap<RTCPeerConnection>(info.Holder());
    return info.GetReturnValue().Set(Nan::New<Function>(self->_onnegotiationneeded));
}

NAN_GETTER(RTCPeerConnection::GetOnAddStream)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    RTCPeerConnection* self = Unwrap<RTCPeerConnection>(info.Holder());
    return info.GetReturnValue().Set(Nan::New<Function>(self->_onaddstream));
}

NAN_GETTER(RTCPeerConnection::GetOnTrack)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    RTCPeerConnection* self = Unwrap<RTCPeerConnection>(info.Holder());
    return info.GetReturnValue().Set(Nan::New<Function>(self->_ontrack));
}

NAN_GETTER(RTCPeerConnection::GetOnRemoveStream)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    RTCPeerConnection* self = Unwrap<RTCPeerConnection>(info.Holder());
    return info.GetReturnValue().Set(Nan::New<Function>(self->_onremovestream));
}

NAN_SETTER(RTCPeerConnection::ReadOnly)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;
}

NAN_SETTER(RTCPeerConnection::SetOnSignalingStateChange)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    RTCPeerConnection* self = Unwrap<RTCPeerConnection>(info.Holder());

    if (!value.IsEmpty() && value->IsFunction()) {
        self->_onsignalingstatechange.Reset<Function>(Local<Function>::Cast(value));
    } else {
        self->_onsignalingstatechange.Reset();
    }
}

NAN_SETTER(RTCPeerConnection::SetOnIceConnectionStateChange)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    RTCPeerConnection* self = Unwrap<RTCPeerConnection>(info.Holder());

    if (!value.IsEmpty() && value->IsFunction()) {
        self->_oniceconnectionstatechange.Reset<Function>(Local<Function>::Cast(value));
    } else {
        self->_oniceconnectionstatechange.Reset();
    }
}

NAN_SETTER(RTCPeerConnection::SetOnIceCandidate)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    RTCPeerConnection* self = Unwrap<RTCPeerConnection>(info.Holder());

    if (!value.IsEmpty() && value->IsFunction()) {
        self->_onicecandidate.Reset<Function>(Local<Function>::Cast(value));
    } else {
        self->_onicecandidate.Reset();
    }
}

NAN_SETTER(RTCPeerConnection::SetOnDataChannel)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    RTCPeerConnection* self = Unwrap<RTCPeerConnection>(info.Holder());

    if (!value.IsEmpty() && value->IsFunction()) {
        self->_ondatachannel.Reset<Function>(Local<Function>::Cast(value));
    } else {
        self->_ondatachannel.Reset();
    }
}

NAN_SETTER(RTCPeerConnection::SetOnNegotiationNeeded)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    RTCPeerConnection* self = Unwrap<RTCPeerConnection>(info.Holder());

    if (!value.IsEmpty() && value->IsFunction()) {
        self->_onnegotiationneeded.Reset<Function>(Local<Function>::Cast(value));
    } else {
        self->_onnegotiationneeded.Reset();
    }
}

NAN_SETTER(RTCPeerConnection::SetOnAddStream)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    RTCPeerConnection* self = Unwrap<RTCPeerConnection>(info.Holder());

    if (!value.IsEmpty() && value->IsFunction()) {
        self->_onaddstream.Reset<Function>(Local<Function>::Cast(value));
    } else {
        self->_onaddstream.Reset();
    }
}

NAN_SETTER(RTCPeerConnection::SetOnTrack)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    RTCPeerConnection* self = Unwrap<RTCPeerConnection>(info.Holder());

    if (!value.IsEmpty() && value->IsFunction()) {
        self->_ontrack.Reset<Function>(Local<Function>::Cast(value));
    } else {
        self->_ontrack.Reset();
    }
}

NAN_SETTER(RTCPeerConnection::SetOnRemoveStream)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    RTCPeerConnection* self = Unwrap<RTCPeerConnection>(info.Holder());

    if (!value.IsEmpty() && value->IsFunction()) {
        self->_onremovestream.Reset<Function>(Local<Function>::Cast(value));
    } else {
        self->_onremovestream.Reset();
    }
}

void RTCPeerConnection::On(WebRTC::Event* event)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    Nan::HandleScope scope;
    RTCPeerConnectionEvent type = event->Type<RTCPeerConnectionEvent>();
    Local<Function> callback;
    Local<Object> container;
    Local<Value> argv[1];
    bool isError = false;
    std::string data;
    int argc = 0;

    switch (type) {
    case kPeerConnectionCreateClosed:
        EventEmitter::SetReference(false);

        break;
    case kPeerConnectionCreateOffer:
        RTC_LOG(LS_INFO) << "kPeerConnectionCreateOffer";
        callback = Nan::New<Function>(_offerCallback);

        _offerCallback.Reset();
        _offerErrorCallback.Reset();

        data = event->Unwrap<std::string>();
        argv[0] = JSON::Parse(Nan::GetCurrentContext(), Nan::New(data.c_str()).ToLocalChecked()).ToLocalChecked();
        argc = 1;

        // call the promise
        Resolve(_offerResolver, argv[0]);

        break;
    case kPeerConnectionCreateOfferError:
        RTC_LOG(LS_INFO) << "kPeerConnectionCreateOfferError";
        callback = Nan::New<Function>(_offerErrorCallback);

        _offerCallback.Reset();
        _offerErrorCallback.Reset();

        isError = true;
        data = event->Unwrap<std::string>();
        argv[0] = Nan::Error(data.c_str());
        argc = 1;

        // call the promise
        Reject(_offerResolver, argv[0]);

        break;
    case kPeerConnectionCreateAnswer:
        RTC_LOG(LS_INFO) << "kPeerConnectionCreateAnswer";
        callback = Nan::New<Function>(_answerCallback);

        _answerCallback.Reset();
        _answerErrorCallback.Reset();

        data = event->Unwrap<std::string>();
        argv[0] = JSON::Parse(Nan::GetCurrentContext(), Nan::New(data.c_str()).ToLocalChecked()).ToLocalChecked();
        argc = 1;

        // call the promise
        Resolve(_answerResolver, argv[0]);

        break;
    case kPeerConnectionCreateAnswerError:
        RTC_LOG(LS_INFO) << "kPeerConnectionCreateAnswerError";
        callback = Nan::New<Function>(_answerErrorCallback);

        _answerCallback.Reset();
        _answerErrorCallback.Reset();

        isError = true;
        data = event->Unwrap<std::string>();
        argv[0] = Nan::Error(data.c_str());
        argc = 1;

        // call the promise
        Reject(_answerResolver, argv[0]);

        break;
    case kPeerConnectionSetLocalDescription:
        RTC_LOG(LS_INFO) << "kPeerConnectionSetLocalDescription";
        callback = Nan::New<Function>(_localCallback);

        _localCallback.Reset();
        _localErrorCallback.Reset();

        // call the promise
        Resolve(_localResolver, Nan::Null());

        break;
    case kPeerConnectionSetLocalDescriptionError:
        RTC_LOG(LS_INFO) << "kPeerConnectionSetLocalDescriptionError";
        callback = Nan::New<Function>(_localErrorCallback);

        _localCallback.Reset();
        _localErrorCallback.Reset();
        _localsdp.Reset();

        isError = true;
        data = event->Unwrap<std::string>();
        argv[0] = Nan::Error(data.c_str());
        argc = 1;

        // call the promise
        Reject(_localResolver, argv[0]);

        break;
    case kPeerConnectionSetRemoteDescription:
        callback = Nan::New<Function>(_remoteCallback);

        _remoteCallback.Reset();
        _remoteErrorCallback.Reset();

        Resolve(_remoteResolver, Nan::Null());

        break;
    case kPeerConnectionSetRemoteDescriptionError:
        callback = Nan::New<Function>(_remoteErrorCallback);

        _remoteCallback.Reset();
        _remoteErrorCallback.Reset();
        _remotesdp.Reset();

        isError = true;
        data = event->Unwrap<std::string>();
        argv[0] = Nan::Error(data.c_str());
        argc = 1;

        Reject(_remoteResolver, argv[0]);

        break;
    case kPeerConnectionIceCandidate:
        callback = Nan::New<Function>(_onicecandidate);
        container = Nan::New<Object>();

        data = event->Unwrap<std::string>();

        if (data.empty()) {
            container->Set(Nan::New("candidate").ToLocalChecked(), Nan::Null());
        } else {
            container->Set(Nan::New("candidate").ToLocalChecked(),
                JSON::Parse(Nan::GetCurrentContext(), Nan::New(data.c_str()).ToLocalChecked()).ToLocalChecked());
        }

        argv[0] = container;
        argc = 1;

        break;
    case kPeerConnectionSignalChange:
        callback = Nan::New<Function>(_onsignalingstatechange);

        break;
    case kPeerConnectionIceChange:
        callback = Nan::New<Function>(_oniceconnectionstatechange);

        break;
    case kPeerConnectionIceGathering:

        break;
    case kPeerConnectionDataChannel:
        callback = Nan::New<Function>(_ondatachannel);

        container = Nan::New<Object>();
        container->Set(Nan::New("channel").ToLocalChecked(), RTCDataChannel::New(event->Unwrap<rtc::scoped_refptr<webrtc::DataChannelInterface> >()));

        argv[0] = container;
        argc = 1;

        break;
    case kPeerConnectionAddStream:
        callback = Nan::New<Function>(_onaddstream);

        container = Nan::New<Object>();
        container->Set(Nan::New("stream").ToLocalChecked(), MediaStream::New(event->Unwrap<rtc::scoped_refptr<webrtc::MediaStreamInterface> >()));

        argv[0] = container;
        argc = 1;

        break;
    case kPeerConnectionTrack: {
        RTC_LOG(LS_INFO) << "kPeerConnectionTrack";
        callback = Nan::New<Function>(_ontrack);

        rtc::scoped_refptr<webrtc::RtpTransceiverInterface> transceiver = event->Unwrap<rtc::scoped_refptr<webrtc::RtpTransceiverInterface> >();

        // set the new object
        container = Nan::New<Object>();
        argv[0] = container;
        argc = 1;

        Local<Array> streams = Nan::New<Array>();
        const std::vector<rtc::scoped_refptr<webrtc::MediaStreamInterface>> &streams_array = transceiver->receiver()->streams();
        for (size_t i = 0; i < streams_array.size(); i++) {
            streams->Set(i, MediaStream::New(streams_array[i]));
        }
        container->Set(Nan::New("streams").ToLocalChecked(), streams);

        rtc::scoped_refptr<webrtc::MediaStreamTrackInterface> track = transceiver->receiver()->track();
        container->Set(Nan::New("track").ToLocalChecked(), MediaStreamTrack::New(track));
        container->Set(Nan::New("receiver").ToLocalChecked(), RTCRtpReceiver::New(transceiver->receiver()));
        container->Set(Nan::New("transceiver").ToLocalChecked(), RTCRtpTransceiver::New(transceiver));

        break;
    }
    case kPeerConnectionRemoveStream:
        callback = Nan::New<Function>(_onremovestream);

        container = Nan::New<Object>();
        container->Set(Nan::New("stream").ToLocalChecked(), MediaStream::New(event->Unwrap<rtc::scoped_refptr<webrtc::MediaStreamInterface> >()));

        argv[0] = container;
        argc = 1;

        break;
    case kPeerConnectionRenegotiation:
        callback = Nan::New<Function>(_onnegotiationneeded);

        break;
    case kPeerConnectionStats:
        callback = Nan::New<Function>(_onstats);

        argv[0] = RTCStatsResponse::New(event->Unwrap<webrtc::StatsReports>());
        argc = 1;

        break;
    }

    if (!callback.IsEmpty() && callback->IsFunction()) {
        callback->Call(handle(), argc, argv);
    } else if (isError) {
        Nan::ThrowError(argv[0]);
    }
}

bool RTCPeerConnection::IsStable()
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    webrtc::PeerConnectionInterface* socket = GetSocket();

    if (socket) {
        webrtc::PeerConnectionInterface::SignalingState state(socket->signaling_state());

        if (state == webrtc::PeerConnectionInterface::kStable) {
            return true;
        }
    }

    return false;
}

void RTCPeerConnection::Resolve(Nan::Persistent<v8::Promise::Resolver> &resolver, v8::Local<v8::Value> value)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    auto localResolver = Nan::New(resolver);
    auto result = localResolver->Resolve(Nan::GetCurrentContext(), value);

    // https://github.com/nodejs/nan/issues/541#issuecomment-191567765
    // Fix issue w/ stalls after resolve/reject is called
    Isolate::GetCurrent()->RunMicrotasks();

    if (result.IsNothing()) {
        RTC_LOG(LS_ERROR) << "Promise resolve failed";
    }
}

void RTCPeerConnection::Reject(Nan::Persistent<v8::Promise::Resolver> &resolver, v8::Local<v8::Value> value)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    auto localResolver = Nan::New(resolver);
    auto result = localResolver->Reject(Nan::GetCurrentContext(), value);

    // https://github.com/nodejs/nan/issues/541#issuecomment-191567765
    // Fix issue w/ stalls after resolve/reject is called
    Isolate::GetCurrent()->RunMicrotasks();

    if (result.IsNothing()) {
        RTC_LOG(LS_ERROR) << "Promise resolve failed";
    }
}
