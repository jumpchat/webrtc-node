#ifndef WEBRTC_RTCPEERCONNECTION_OBSERVERS_H
#define WEBRTC_RTCPEERCONNECTION_OBSERVERS_H

#include "EventEmitter.h"

namespace WebRTC {
class OfferObserver : public webrtc::CreateSessionDescriptionObserver, public NotifyEmitter {
public:
    OfferObserver(EventEmitter* listener = 0);

    void OnSuccess(webrtc::SessionDescriptionInterface* sdp) final;
    void OnFailure(const std::string& error) final;
};

class AnswerObserver : public webrtc::CreateSessionDescriptionObserver, public NotifyEmitter {
public:
    AnswerObserver(EventEmitter* listener = 0);

    void OnSuccess(webrtc::SessionDescriptionInterface* sdp) final;
    void OnFailure(const std::string& error) final;
};

class LocalDescriptionObserver : public webrtc::SetSessionDescriptionObserver, public NotifyEmitter {
public:
    LocalDescriptionObserver(EventEmitter* listener = 0);

    void OnSuccess() final;
    void OnFailure(const std::string& error) final;
};

class RemoteDescriptionObserver : public webrtc::SetSessionDescriptionObserver, public NotifyEmitter {
public:
    RemoteDescriptionObserver(EventEmitter* listener = 0);

    void OnSuccess() final;
    void OnFailure(const std::string& error) final;
};

class RTCPeerConnectionObserver : public webrtc::PeerConnectionObserver,
                               public rtc::RefCountInterface,
                               public NotifyEmitter {
public:
    RTCPeerConnectionObserver(EventEmitter* listener = 0);

    void OnSignalingChange(webrtc::PeerConnectionInterface::SignalingState state) final;
    void OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState state) final;
    void OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState state) final;
    void OnIceCandidate(const webrtc::IceCandidateInterface* candidate) final;
    void OnDataChannel(rtc::scoped_refptr<webrtc::DataChannelInterface> channel) final;
    void OnRenegotiationNeeded() final;

    void OnAddStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) final;
    void OnRemoveStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) final;
    void OnTrack(rtc::scoped_refptr<webrtc::RtpTransceiverInterface> receiver) final;
};



class StatsObserver : public webrtc::StatsObserver, public NotifyEmitter {
public:
    StatsObserver(EventEmitter* listener = 0);

    void OnComplete(const webrtc::StatsReports& reports) final;
};
};

#endif
