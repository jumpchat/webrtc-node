#include "RTCPeerConnectionObservers.h"
#include "RTCDataChannel.h"
#include "MediaStream.h"
#include "MediaStreamTrack.h"
#include "RTCPeerConnection.h"

using namespace WebRTC;

OfferObserver::OfferObserver(EventEmitter* listener)
    : NotifyEmitter(listener)
{
}

void OfferObserver::OnSuccess(webrtc::SessionDescriptionInterface* desc)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    Json::StyledWriter writer;
    Json::Value msg;
    std::string sdp;

    if (desc->ToString(&sdp)) {
        msg["type"] = desc->type();
        msg["sdp"] = sdp;

        Emit(RTCPeerConnection::kPeerConnectionCreateOffer, writer.write(msg));
    }
}

void OfferObserver::OnFailure(const std::string& error)
{
    RTC_LOG(LS_ERROR) << __PRETTY_FUNCTION__;

    Emit(RTCPeerConnection::kPeerConnectionCreateOfferError, error);
}

AnswerObserver::AnswerObserver(EventEmitter* listener)
    : NotifyEmitter(listener)
{
}

void AnswerObserver::OnSuccess(webrtc::SessionDescriptionInterface* desc)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    Json::StyledWriter writer;
    Json::Value msg;
    std::string sdp;

    if (desc->ToString(&sdp)) {
        msg["type"] = desc->type();
        msg["sdp"] = sdp;

        Emit(RTCPeerConnection::kPeerConnectionCreateAnswer, writer.write(msg));
    }
}

void AnswerObserver::OnFailure(const std::string& error)
{
    RTC_LOG(LS_ERROR) << __PRETTY_FUNCTION__;

    Emit(RTCPeerConnection::kPeerConnectionCreateAnswerError, error);
}

LocalDescriptionObserver::LocalDescriptionObserver(EventEmitter* listener)
    : NotifyEmitter(listener)
{
}

void LocalDescriptionObserver::OnSuccess()
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    Emit(RTCPeerConnection::kPeerConnectionSetLocalDescription);
}

void LocalDescriptionObserver::OnFailure(const std::string& error)
{
    RTC_LOG(LS_ERROR) << __PRETTY_FUNCTION__;

    Emit(RTCPeerConnection::kPeerConnectionSetLocalDescriptionError, error);
}

RemoteDescriptionObserver::RemoteDescriptionObserver(EventEmitter* listener)
    : NotifyEmitter(listener)
{
}

void RemoteDescriptionObserver::OnSuccess()
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    Emit(RTCPeerConnection::kPeerConnectionSetRemoteDescription);
}

void RemoteDescriptionObserver::OnFailure(const std::string& error)
{
    RTC_LOG(LS_ERROR) << __PRETTY_FUNCTION__;

    Emit(RTCPeerConnection::kPeerConnectionSetRemoteDescriptionError, error);
}

RTCPeerConnectionObserver::RTCPeerConnectionObserver(EventEmitter* listener)
    : NotifyEmitter(listener)
{
}

void RTCPeerConnectionObserver::OnSignalingChange(webrtc::PeerConnectionInterface::SignalingState state)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    Emit(RTCPeerConnection::kPeerConnectionSignalChange);

    if (state == webrtc::PeerConnectionInterface::kClosed) {
        Emit(RTCPeerConnection::kPeerConnectionCreateClosed);
    }
}

void RTCPeerConnectionObserver::OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState state)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    Emit(RTCPeerConnection::kPeerConnectionIceChange);
}

void RTCPeerConnectionObserver::OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState state)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    Emit(RTCPeerConnection::kPeerConnectionIceGathering);

    if (state == webrtc::PeerConnectionInterface::kIceGatheringComplete) {
        Emit(RTCPeerConnection::kPeerConnectionIceCandidate, std::string());
    }
}

void RTCPeerConnectionObserver::OnDataChannel(rtc::scoped_refptr<webrtc::DataChannelInterface> channel)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    rtc::scoped_refptr<webrtc::DataChannelInterface> dataChannel = channel;

    if (dataChannel.get()) {
        Emit(RTCPeerConnection::kPeerConnectionDataChannel, dataChannel);
    }
}

void RTCPeerConnectionObserver::OnAddStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    rtc::scoped_refptr<webrtc::MediaStreamInterface> mediaStream = stream;

    if (mediaStream.get()) {
        Emit(RTCPeerConnection::kPeerConnectionAddStream, mediaStream);
    }
}

void RTCPeerConnectionObserver::OnRemoveStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    rtc::scoped_refptr<webrtc::MediaStreamInterface> mediaStream = stream;

    if (mediaStream.get()) {
        Emit(RTCPeerConnection::kPeerConnectionRemoveStream, mediaStream);
    }
}

void RTCPeerConnectionObserver::OnTrack(rtc::scoped_refptr<webrtc::RtpTransceiverInterface> transceiver)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    if (transceiver.get()) {
        Emit(RTCPeerConnection::kPeerConnectionTrack, transceiver);
    }
}

void RTCPeerConnectionObserver::OnRenegotiationNeeded()
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    Emit(RTCPeerConnection::kPeerConnectionRenegotiation);
}

void RTCPeerConnectionObserver::OnIceCandidate(const webrtc::IceCandidateInterface* candidate)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    Json::StyledWriter writer;
    Json::Value msg;
    std::string sdp;

    if (candidate->ToString(&sdp)) {
        msg["sdpMid"] = candidate->sdp_mid();
        msg["sdpMLineIndex"] = candidate->sdp_mline_index();
        msg["candidate"] = sdp;

        Emit(RTCPeerConnection::kPeerConnectionIceCandidate, writer.write(msg));
    }
}

StatsObserver::StatsObserver(EventEmitter* listener)
    : NotifyEmitter(listener)
{
}

void StatsObserver::OnComplete(const webrtc::StatsReports& reports)
{
    RTC_LOG(LS_INFO) << "StatsObserver::OnComplete()";

    Emit(RTCPeerConnection::kPeerConnectionStats, reports);
}
