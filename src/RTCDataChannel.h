#ifndef WEBRTC_RTCDATACHANNEL_H
#define WEBRTC_RTCDATACHANNEL_H

#include "Common.h"
#include "EventEmitter.h"

namespace WebRTC {

class RTCDataChannel : public Nan::ObjectWrap, public EventEmitter, public webrtc::DataChannelObserver {
public:
    enum RTCDataChannelEvent {
        kDataChannelStateChange,
        kDataChannelBinary,
        kDataChannelData,
    };
    static NAN_MODULE_INIT(Init);
    static v8::Local<v8::Value> New(rtc::scoped_refptr<webrtc::DataChannelInterface> dataChannel);

private:
    RTCDataChannel();
    ~RTCDataChannel() final;

    static NAN_METHOD(New);
    static NAN_METHOD(Close);
    static NAN_METHOD(Send);

    static NAN_GETTER(GetId);
    static NAN_GETTER(GetLabel);
    static NAN_GETTER(GetOrdered);
    static NAN_GETTER(GetProtocol);
    static NAN_GETTER(GetReadyState);
    static NAN_GETTER(GetBufferedAmount);
    static NAN_GETTER(GetBinaryType);
    static NAN_GETTER(GetMaxPacketLifeType);
    static NAN_GETTER(GetMaxRetransmits);
    static NAN_GETTER(GetNegotiated);
    static NAN_GETTER(GetReliable);
    static NAN_GETTER(GetOnOpen);
    static NAN_GETTER(GetOnMessage);
    static NAN_GETTER(GetOnClose);
    static NAN_GETTER(GetOnError);

    static NAN_SETTER(ReadOnly);
    static NAN_SETTER(SetBinaryType);
    static NAN_SETTER(SetOnOpen);
    static NAN_SETTER(SetOnMessage);
    static NAN_SETTER(SetOnClose);
    static NAN_SETTER(SetOnError);

    void On(Event* event) final;

    // webrtc::DataChannelObserver
    void OnStateChange() final;
    void OnMessage(const webrtc::DataBuffer& buffer) final;

    webrtc::DataChannelInterface* GetSocket() const;

protected:
    rtc::scoped_refptr<webrtc::DataChannelInterface> _socket;

    Nan::Persistent<v8::String> _binaryType;

    Nan::Persistent<v8::Function> _onopen;
    Nan::Persistent<v8::Function> _onmessage;
    Nan::Persistent<v8::Function> _onclose;
    Nan::Persistent<v8::Function> _onerror;
};
};

#endif
