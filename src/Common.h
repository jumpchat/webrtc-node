#ifndef WEBRTC_COMMON_H
#define WEBRTC_COMMON_H

#ifdef WEBRTC_WIN
#define __PRETTY_FUNCTION__ __FUNCSIG__
#include <Winsock2.h>
#include <rtc_base/win32socketinit.h>
#include <rtc_base/win32socketserver.h>
#endif

#include <nan.h>

#include "rtc_base/atomicops.h"
#include "rtc_base/buffer.h"
#include "rtc_base/helpers.h"
#include "rtc_base/json.h"
#include "rtc_base/logging.h"
#include "rtc_base/refcount.h"
#include "rtc_base/scoped_ref_ptr.h"
#include "rtc_base/scoped_ref_ptr.h"
#include "rtc_base/ssladapter.h"
#include "rtc_base/sslstreamadapter.h"
#include "rtc_base/stringencode.h"
#include "rtc_base/stringutils.h"
#include "rtc_base/thread.h"

#include "api/audio_codecs/builtin_audio_decoder_factory.h"
#include "api/audio_codecs/builtin_audio_encoder_factory.h"
#include "api/datachannelinterface.h"
#include "api/jsep.h"
#include "api/jsepsessiondescription.h"
#include "api/mediaconstraintsinterface.h"
#include "api/mediastreaminterface.h"
#include "api/peerconnectioninterface.h"
#include "api/test/fakeconstraints.h"
#include "api/video/video_frame.h"
#include "api/video/i420_buffer.h"
#include "pc/peerconnectionfactory.h"
#include "pc/mediastream.h"

#include "media/base/videosourcebase.h"
#include "media/engine/webrtcvideocapturerfactory.h"
#include "modules/video_capture/video_capture_factory.h"
#include "modules/desktop_capture/desktop_capturer.h"
#include "modules/desktop_capture/desktop_capture_options.h"
#include "modules/desktop_capture/desktop_and_cursor_composer.h"
#include "modules/desktop_capture/mouse_cursor_monitor.h"

#include <node_object_wrap.h>
#include <queue>
#include <string>
#include <uv.h>

#endif
