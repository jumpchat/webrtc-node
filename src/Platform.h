#ifndef WEBRTC_PLATFORM_H
#define WEBRTC_PLATFORM_H

#include "Common.h"

namespace WebRTC {
  class Platform {
	  public:
	    static void Init();
	    static void Dispose();
      static rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> GetFactory();
  };
};

#endif