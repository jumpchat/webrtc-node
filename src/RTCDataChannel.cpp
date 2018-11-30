#include "RTCDataChannel.h"

using namespace v8;
using namespace WebRTC;

static Nan::Persistent<Function> constructor;

NAN_MODULE_INIT(RTCDataChannel::Init)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    Nan::HandleScope scope;

    Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>(New);
    tpl->InstanceTemplate()->SetInternalFieldCount(1);
    tpl->SetClassName(Nan::New("RTCDataChannel").ToLocalChecked());

    Nan::SetPrototypeMethod(tpl, "close", Close);
    Nan::SetPrototypeMethod(tpl, "send", Send);

    Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("id").ToLocalChecked(), GetId);
    Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("label").ToLocalChecked(), GetLabel);
    Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("ordered").ToLocalChecked(), GetOrdered);
    Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("protocol").ToLocalChecked(), GetProtocol);
    Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("readyState").ToLocalChecked(), GetReadyState);
    Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("bufferedAmount").ToLocalChecked(), GetBufferedAmount);
    Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("binaryType").ToLocalChecked(), GetBinaryType, SetBinaryType);
    Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("maxPacketLifeType").ToLocalChecked(), GetMaxPacketLifeType);
    Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("maxRetransmits").ToLocalChecked(), GetMaxRetransmits);
    Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("negotiated").ToLocalChecked(), GetNegotiated);
    Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("reliable").ToLocalChecked(), GetReliable);
    Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("onopen").ToLocalChecked(), GetOnOpen, SetOnOpen);
    Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("onmessage").ToLocalChecked(), GetOnMessage, SetOnMessage);
    Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("onclose").ToLocalChecked(), GetOnClose, SetOnClose);
    Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("onerror").ToLocalChecked(), GetOnError, SetOnError);

    constructor.Reset<Function>(tpl->GetFunction());
}

RTCDataChannel::RTCDataChannel()
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;
}

RTCDataChannel::~RTCDataChannel()
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    if (_socket.get()) {
        _socket->UnregisterObserver();

        webrtc::DataChannelInterface::DataState state(_socket->state());

        if (state != webrtc::DataChannelInterface::kClosing || state != webrtc::DataChannelInterface::kClosed) {
            _socket->Close();
        }
    }
}

NAN_METHOD(RTCDataChannel::New)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    if (info.IsConstructCall()) {
        RTCDataChannel* dataChannel = new RTCDataChannel();
        dataChannel->Wrap(info.This());
        return info.GetReturnValue().Set(info.This());
    }

    Nan::ThrowError("Internal Error");
    info.GetReturnValue().SetUndefined();
}

Local<Value> RTCDataChannel::New(rtc::scoped_refptr<webrtc::DataChannelInterface> dataChannel)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    Nan::EscapableHandleScope scope;
    Local<Function> instance = Nan::New(constructor);

    if (instance.IsEmpty() || !dataChannel.get()) {
        return scope.Escape(Nan::Null());
    }

    Local<Object> ret = instance->NewInstance(Nan::GetCurrentContext()).ToLocalChecked();
    RTCDataChannel* self = Unwrap<RTCDataChannel>(ret);

    self->SetReference(true);
    self->_socket = dataChannel;
    self->_socket->RegisterObserver(self);
    self->Emit(kDataChannelStateChange);

    self->_binaryType.Reset(Nan::New("arraybuffer").ToLocalChecked());

    return scope.Escape(ret);
}

NAN_METHOD(RTCDataChannel::Close)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    RTCDataChannel* self = Unwrap<RTCDataChannel>(info.This());
    webrtc::DataChannelInterface* socket = self->GetSocket();

    if (socket) {
        webrtc::DataChannelInterface::DataState state(socket->state());

        if (state != webrtc::DataChannelInterface::kClosing || state != webrtc::DataChannelInterface::kClosed) {
            socket->Close();
        }
    }

    info.GetReturnValue().SetUndefined();
}

NAN_METHOD(RTCDataChannel::Send)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    RTCDataChannel* self = Unwrap<RTCDataChannel>(info.This());
    webrtc::DataChannelInterface* socket = self->GetSocket();
    bool retval = false;

    if (socket) {
        if (info[0]->IsString()) {
            std::string data(*Nan::Utf8String(info[0]));
            webrtc::DataBuffer buffer(data);
            retval = socket->Send(buffer);
        } else {
            Local<ArrayBuffer> container = Local<ArrayBuffer>::Cast(info[0]);
            //v8::ArrayBuffer::New(info[0]);
            const v8::ArrayBuffer::Contents content = container->Externalize();
            rtc::CopyOnWriteBuffer data(reinterpret_cast<uint8_t*>(content.Data()), content.ByteLength());
            webrtc::DataBuffer buffer(data, true);
            retval = socket->Send(buffer);
        }
    }

    return info.GetReturnValue().Set(Nan::New(retval));
}

NAN_GETTER(RTCDataChannel::GetId)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    RTCDataChannel* self = Unwrap<RTCDataChannel>(info.Holder());
    webrtc::DataChannelInterface* socket = self->GetSocket();

    if (socket) {
        return info.GetReturnValue().Set(Nan::New(socket->id()));
    }

    info.GetReturnValue().SetUndefined();
}

NAN_GETTER(RTCDataChannel::GetLabel)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    RTCDataChannel* self = Unwrap<RTCDataChannel>(info.Holder());
    webrtc::DataChannelInterface* socket = self->GetSocket();

    if (socket) {
        return info.GetReturnValue().Set(Nan::New(socket->label().c_str()).ToLocalChecked());
    }

    info.GetReturnValue().SetUndefined();
}

NAN_GETTER(RTCDataChannel::GetOrdered)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    RTCDataChannel* self = Unwrap<RTCDataChannel>(info.Holder());
    webrtc::DataChannelInterface* socket = self->GetSocket();

    if (socket) {
        return info.GetReturnValue().Set(Nan::New(socket->ordered()));
    }

    info.GetReturnValue().SetUndefined();
}

NAN_GETTER(RTCDataChannel::GetProtocol)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    RTCDataChannel* self = Unwrap<RTCDataChannel>(info.Holder());
    webrtc::DataChannelInterface* socket = self->GetSocket();

    if (socket) {
        return info.GetReturnValue().Set(Nan::New(socket->protocol().c_str()).ToLocalChecked());
    }

    info.GetReturnValue().SetUndefined();
}

NAN_GETTER(RTCDataChannel::GetReadyState)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    RTCDataChannel* self = Unwrap<RTCDataChannel>(info.Holder());
    webrtc::DataChannelInterface* socket = self->GetSocket();

    if (socket) {
        webrtc::DataChannelInterface::DataState state(socket->state());

        switch (state) {
        case webrtc::DataChannelInterface::kConnecting:
            return info.GetReturnValue().Set(Nan::New("connecting").ToLocalChecked());
            break;
        case webrtc::DataChannelInterface::kOpen:
            return info.GetReturnValue().Set(Nan::New("open").ToLocalChecked());
            break;
        case webrtc::DataChannelInterface::kClosing:
            return info.GetReturnValue().Set(Nan::New("closing").ToLocalChecked());
            break;
        case webrtc::DataChannelInterface::kClosed:
            return info.GetReturnValue().Set(Nan::New("closed").ToLocalChecked());
            break;
        }
    }

    info.GetReturnValue().SetUndefined();
}

NAN_GETTER(RTCDataChannel::GetBufferedAmount)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    RTCDataChannel* self = Unwrap<RTCDataChannel>(info.Holder());
    webrtc::DataChannelInterface* socket = self->GetSocket();

    if (socket) {
        return info.GetReturnValue().Set(Nan::New(static_cast<uint32_t>(socket->buffered_amount())));
    }

    info.GetReturnValue().SetUndefined();
}

NAN_GETTER(RTCDataChannel::GetBinaryType)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    RTCDataChannel* self = Unwrap<RTCDataChannel>(info.Holder());
    return info.GetReturnValue().Set(Nan::New(self->_binaryType));
}

NAN_GETTER(RTCDataChannel::GetMaxPacketLifeType)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    RTCDataChannel* self = Unwrap<RTCDataChannel>(info.Holder());
    webrtc::DataChannelInterface* socket = self->GetSocket();

    if (socket) {
        return info.GetReturnValue().Set(Nan::New(static_cast<uint32_t>(socket->maxRetransmitTime())));
    }

    info.GetReturnValue().SetUndefined();
}

NAN_GETTER(RTCDataChannel::GetMaxRetransmits)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    RTCDataChannel* self = Unwrap<RTCDataChannel>(info.Holder());
    webrtc::DataChannelInterface* socket = self->GetSocket();

    if (socket) {
        return info.GetReturnValue().Set(Nan::New(static_cast<uint32_t>(socket->maxRetransmits())));
    }

    info.GetReturnValue().SetUndefined();
}

NAN_GETTER(RTCDataChannel::GetNegotiated)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    RTCDataChannel* self = Unwrap<RTCDataChannel>(info.Holder());
    webrtc::DataChannelInterface* socket = self->GetSocket();

    if (socket) {
        return info.GetReturnValue().Set(Nan::New(socket->negotiated()));
    }

    return info.GetReturnValue().Set(Nan::False());
}

NAN_GETTER(RTCDataChannel::GetReliable)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    RTCDataChannel* self = Unwrap<RTCDataChannel>(info.Holder());
    webrtc::DataChannelInterface* socket = self->GetSocket();

    if (socket) {
        return info.GetReturnValue().Set(Nan::New(socket->reliable()));
    }

    return info.GetReturnValue().Set(Nan::False());
}

NAN_GETTER(RTCDataChannel::GetOnOpen)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    RTCDataChannel* self = Unwrap<RTCDataChannel>(info.Holder());
    return info.GetReturnValue().Set(Nan::New<Function>(self->_onopen));
}

NAN_GETTER(RTCDataChannel::GetOnMessage)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    RTCDataChannel* self = Unwrap<RTCDataChannel>(info.Holder());
    return info.GetReturnValue().Set(Nan::New<Function>(self->_onmessage));
}

NAN_GETTER(RTCDataChannel::GetOnClose)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    RTCDataChannel* self = Unwrap<RTCDataChannel>(info.Holder());
    return info.GetReturnValue().Set(Nan::New<Function>(self->_onclose));
}

NAN_GETTER(RTCDataChannel::GetOnError)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    RTCDataChannel* self = Unwrap<RTCDataChannel>(info.Holder());
    return info.GetReturnValue().Set(Nan::New<Function>(self->_onerror));
}

NAN_SETTER(RTCDataChannel::ReadOnly)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;
}

NAN_SETTER(RTCDataChannel::SetBinaryType)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    RTCDataChannel* self = Unwrap<RTCDataChannel>(info.Holder());

    if (!value.IsEmpty() && value->IsString()) {
        self->_binaryType.Reset(value->ToString());
    } else {
        self->_binaryType.Reset(Nan::New("arraybuffer").ToLocalChecked());
    }
}

NAN_SETTER(RTCDataChannel::SetOnOpen)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    RTCDataChannel* self = Unwrap<RTCDataChannel>(info.Holder());

    if (!value.IsEmpty() && value->IsFunction()) {
        self->_onopen.Reset<Function>(Local<Function>::Cast(value));
    } else {
        self->_onopen.Reset();
    }
}

NAN_SETTER(RTCDataChannel::SetOnMessage)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    RTCDataChannel* self = Unwrap<RTCDataChannel>(info.Holder());

    if (!value.IsEmpty() && value->IsFunction()) {
        self->_onmessage.Reset<Function>(Local<Function>::Cast(value));
    } else {
        self->_onmessage.Reset();
    }
}

NAN_SETTER(RTCDataChannel::SetOnClose)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    RTCDataChannel* self = Unwrap<RTCDataChannel>(info.Holder());

    if (!value.IsEmpty() && value->IsFunction()) {
        self->_onclose.Reset<Function>(Local<Function>::Cast(value));
    } else {
        self->_onclose.Reset();
    }
}

NAN_SETTER(RTCDataChannel::SetOnError)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    RTCDataChannel* self = Unwrap<RTCDataChannel>(info.Holder());

    if (!value.IsEmpty() && value->IsFunction()) {
        self->_onerror.Reset<Function>(Local<Function>::Cast(value));
    } else {
        self->_onclose.Reset();
    }
}

void RTCDataChannel::On(Event* event)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    Nan::HandleScope scope;
    RTCDataChannelEvent type = event->Type<RTCDataChannelEvent>();
    Local<Function> callback;
    Local<Value> argv[1];
    bool isError = false;
    int argc = 0;

    if (type == kDataChannelStateChange) {
        webrtc::DataChannelInterface* socket = RTCDataChannel::GetSocket();

        if (socket) {
            switch (socket->state()) {
            case webrtc::DataChannelInterface::kConnecting:

                break;
            case webrtc::DataChannelInterface::kOpen:
                callback = Nan::New<Function>(_onopen);

                break;
            case webrtc::DataChannelInterface::kClosing:

                break;
            case webrtc::DataChannelInterface::kClosed:
                EventEmitter::SetReference(false);

                callback = Nan::New<Function>(_onclose);
                _onclose.Reset();

                break;
            }
        }
    } else {
        callback = Nan::New<Function>(_onmessage);
        rtc::CopyOnWriteBuffer buffer = event->Unwrap<rtc::CopyOnWriteBuffer>();
        Local<Object> container = Nan::New<Object>();
        argv[0] = container;
        argc = 1;

        if (type == kDataChannelData) {
            container->Set(Nan::New("data").ToLocalChecked(), Nan::New(reinterpret_cast<char*>(buffer.data()), buffer.size()).ToLocalChecked());
        } else {
            Local<ArrayBuffer> arrayBuffer = ArrayBuffer::New(v8::Isolate::GetCurrent(), reinterpret_cast<char*>(buffer.data()), buffer.size());
            container->Set(Nan::New("data").ToLocalChecked(), arrayBuffer);
        }
    }

    if (!callback.IsEmpty() && callback->IsFunction()) {
        callback->Call(handle(), argc, argv);
    } else if (isError) {
        Nan::ThrowError(argv[0]);
    }
}

webrtc::DataChannelInterface* RTCDataChannel::GetSocket() const
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    return _socket.get();
}

void RTCDataChannel::OnStateChange()
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;
    Emit(kDataChannelStateChange);
}

void RTCDataChannel::OnMessage(const webrtc::DataBuffer& buffer)
{
    RTC_LOG(LS_INFO) << __PRETTY_FUNCTION__;

    if (buffer.binary) {
        Emit(kDataChannelBinary, buffer.data);
    } else {
        Emit(kDataChannelData, buffer.data);
    }
}
