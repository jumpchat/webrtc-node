#ifndef WEBRTC_MEDIADEVICES_H
#define WEBRTC_MEDIADEVICES_H

#include "Common.h"

namespace WebRTC {
class MediaDevices : public Nan::ObjectWrap {
public:
    static NAN_MODULE_INIT(Init);

private:
    static NAN_METHOD(New);
    static NAN_METHOD(GetUserMedia);
};
};

#endif
