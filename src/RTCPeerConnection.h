#ifndef WEBRTC_RTCPEERCONNECTION_H
#define WEBRTC_RTCPEERCONNECTION_H

#include "Common.h"
#include "EventEmitter.h"
#include "MediaConstraints.h"
#include "RTCPeerConnectionObservers.h"

namespace WebRTC {
class RTCPeerConnection : public Nan::ObjectWrap, public EventEmitter {
public:
    enum RTCPeerConnectionEvent {
        kPeerConnectionCreateClosed = 1,
        kPeerConnectionCreateOffer,
        kPeerConnectionCreateOfferError,
        kPeerConnectionCreateAnswer,
        kPeerConnectionCreateAnswerError,
        kPeerConnectionSetLocalDescription,
        kPeerConnectionSetLocalDescriptionError,
        kPeerConnectionSetRemoteDescription,
        kPeerConnectionSetRemoteDescriptionError,
        kPeerConnectionIceCandidate,
        kPeerConnectionSignalChange,
        kPeerConnectionIceChange,
        kPeerConnectionIceGathering,
        kPeerConnectionDataChannel,
        kPeerConnectionAddStream,
        kPeerConnectionRemoveStream,
        kPeerConnectionRenegotiation,
        kPeerConnectionStats
    };

    static NAN_MODULE_INIT(Init);

private:
    RTCPeerConnection(const v8::Local<v8::Object>& configuration);
    ~RTCPeerConnection() final;

    static NAN_METHOD(New);
    static NAN_METHOD(CreateOffer);
    static NAN_METHOD(CreateAnswer);
    static NAN_METHOD(SetLocalDescription);
    static NAN_METHOD(SetRemoteDescription);
    static NAN_METHOD(AddIceCandidate);
    static NAN_METHOD(CreateDataChannel);
    static NAN_METHOD(AddStream);
    static NAN_METHOD(RemoveStream);
    static NAN_METHOD(GetLocalStreams);
    static NAN_METHOD(GetRemoteStreams);
    static NAN_METHOD(GetStreamById);
    static NAN_METHOD(GetStats);
    static NAN_METHOD(GetConfiguration);
    static NAN_METHOD(Close);

    static NAN_GETTER(GetSignalingState);
    static NAN_GETTER(GetIceConnectionState);
    static NAN_GETTER(GetIceGatheringState);
    static NAN_GETTER(GetOnSignalingStateChange);
    static NAN_GETTER(GetOnIceConnectionStateChange);
    static NAN_GETTER(GetOnIceCandidate);
    static NAN_GETTER(GetOnDataChannel);
    static NAN_GETTER(GetOnNegotiationNeeded);
    static NAN_GETTER(GetOnAddStream);
    static NAN_GETTER(GetOnRemoveStream);
    static NAN_GETTER(GetLocalDescription);
    static NAN_GETTER(GetRemoteDescription);

    static NAN_SETTER(ReadOnly);
    static NAN_SETTER(SetOnSignalingStateChange);
    static NAN_SETTER(SetOnIceConnectionStateChange);
    static NAN_SETTER(SetOnIceCandidate);
    static NAN_SETTER(SetOnDataChannel);
    static NAN_SETTER(SetOnNegotiationNeeded);
    static NAN_SETTER(SetOnAddStream);
    static NAN_SETTER(SetOnRemoveStream);

    void On(Event* event) final;

    bool IsStable();

    webrtc::PeerConnectionInterface* GetSocket();

protected:
    Nan::Persistent<v8::Function> _onsignalingstatechange;
    Nan::Persistent<v8::Function> _oniceconnectionstatechange;
    Nan::Persistent<v8::Function> _onicecandidate;
    Nan::Persistent<v8::Function> _ondatachannel;
    Nan::Persistent<v8::Function> _onnegotiationneeded;
    Nan::Persistent<v8::Function> _onaddstream;
    Nan::Persistent<v8::Function> _onremovestream;

    Nan::Persistent<v8::Function> _offerCallback;
    Nan::Persistent<v8::Function> _offerErrorCallback;
    Nan::Persistent<v8::Promise::Resolver> _offerResolver;

    Nan::Persistent<v8::Function> _answerCallback;
    Nan::Persistent<v8::Function> _answerErrorCallback;
    Nan::Persistent<v8::Promise::Resolver> _answerResolver;

    Nan::Persistent<v8::Function> _localCallback;
    Nan::Persistent<v8::Function> _localErrorCallback;
    Nan::Persistent<v8::Promise::Resolver> _localResolver;

    Nan::Persistent<v8::Function> _remoteCallback;
    Nan::Persistent<v8::Function> _remoteErrorCallback;
    Nan::Persistent<v8::Promise::Resolver> _remoteResolver;

    Nan::Persistent<v8::Function> _onstats;

    Nan::Persistent<v8::Object> _localsdp;
    Nan::Persistent<v8::Object> _remotesdp;

    rtc::scoped_refptr<StatsObserver> _stats;
    rtc::scoped_refptr<OfferObserver> _offer;
    rtc::scoped_refptr<AnswerObserver> _answer;
    rtc::scoped_refptr<LocalDescriptionObserver> _local;
    rtc::scoped_refptr<RemoteDescriptionObserver> _remote;
    rtc::scoped_refptr<RTCPeerConnectionObserver> _peer;
    rtc::scoped_refptr<webrtc::PeerConnectionInterface> _socket;
    rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> _factory;

    webrtc::PeerConnectionInterface::RTCConfiguration _config;

    void Resolve(Nan::Persistent<v8::Promise::Resolver> &resolver, v8::Local<v8::Value> value);
    void Reject(Nan::Persistent<v8::Promise::Resolver> &resolver, v8::Local<v8::Value> value);
};
};

#endif