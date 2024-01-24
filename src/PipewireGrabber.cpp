#include <Huenicorn/PipewireGrabber.hpp>

#include <sstream>
#include <future>
#include <fcntl.h>

#include <Huenicorn/XdgDesktopPortal.hpp>
#include <Huenicorn/ImageProcessing.hpp>
#include <Huenicorn/Config.hpp>
#include <Huenicorn/Logger.hpp>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#include <spa/debug/types.h>
#include <spa/param/video/type-info.h>
#pragma GCC diagnostic pop

using namespace std::chrono_literals;

namespace Huenicorn
{
  PipewireGrabber::PipewireGrabber(Config* config):
  IGrabber(config)
  {
    m_pwData.config = config;
    auto readyFuture = m_capture.fdReadyPromise.get_future();

    m_xdgThread.emplace(_initCapture, &m_capture);

    readyFuture.wait();

    if(!readyFuture.get()){
      m_capture.updateXdgContext = false;
      m_xdgThread.value().join();
      m_xdgThread.reset();
      throw std::runtime_error("Failed to select source");
    }

    // TODO : check if a thread for this is really necessary
    m_pipewireThread.emplace(_pipewireThread, &m_capture, &m_pwData);
  }


  PipewireGrabber::~PipewireGrabber()
  {
    _stop();
  }


  IGrabber::Resolution PipewireGrabber::displayResolution() const
  {
    return {m_pwData.format.info.raw.size.width, m_pwData.format.info.raw.size.height};
  }


  IGrabber::RefreshRate PipewireGrabber::displayRefreshRate() const
  {
    return m_pwData.format.info.raw.max_framerate.num;
  }


  void PipewireGrabber::grabFrameSubsample(cv::Mat& cvImage)
  {
    auto lock = std::lock_guard(m_pwData.frameDoubleBuffer.mutex);
    cvImage = std::move(m_pwData.frameDoubleBuffer.frame.at(0));
  }


  void PipewireGrabber::_onCoreInfoCallback(void* userData, const pw_core_info* info)
  {
    (void)userData;
    (void)info;
    //PipewireData* pw = static_cast<PipewireData*>(userData);
    //update_pw_versions(pw, info->version);
    //Logger::debug("Pipewire version : " + std::string(info->version));
  }


  void PipewireGrabber::_onCoreDoneCallback(void* userData, uint32_t id, int seq)
  {
    (void)id;
    (void)seq;

    PipewireData* pw = static_cast<PipewireData*>(userData);

    // TODO : See if extra checks are required
    pw_thread_loop_signal(pw->loop, FALSE);
  }


  void PipewireGrabber::_onCoreErrorCallback(void* userData, uint32_t id, int seq, int res, const char* message)
  {
    PipewireData* pw = static_cast<PipewireData*>(userData);

    std::stringstream ss;
    ss << "[pipewire] Error id: " << id << " seq: " << seq << " res: " << res << "" << g_strerror(res) << " " << message << std::endl;
    Logger::error(ss.str());

    pw_thread_loop_signal(pw->loop, FALSE);
  }


  void PipewireGrabber::_onStreamProcess(void* userdata)
  {
    PipewireData* pw = static_cast<PipewireData*>(userdata);
    struct pw_buffer* pwBuffer;

    if((pwBuffer = pw_stream_dequeue_buffer(pw->stream)) == NULL){
      //pw_log_warn("out of buffers");
      return;
    }

    spa_buffer* spaBuffer = pwBuffer->buffer;
    if(spaBuffer->datas[0].data == NULL){
      return;
    }

    cv::Mat rgbaFrame(pw->format.info.raw.size.width, pw->format.info.raw.size.height, CV_8UC4, spaBuffer->datas[0].data);
    ImageProcessing::rescale(rgbaFrame, pw->config->subsampleWidth());
    cv::cvtColor(rgbaFrame, rgbaFrame, cv::COLOR_RGBA2RGB);

    {
      auto lock = std::lock_guard(pw->frameDoubleBuffer.mutex);
      std::swap(pw->frameDoubleBuffer.frame[0], pw->frameDoubleBuffer.frame[1]);
      pw->frameDoubleBuffer.frame[0] = std::move(rgbaFrame);
    }

    pw_stream_queue_buffer(pw->stream, pwBuffer);
  }


  void PipewireGrabber::_onStreamParamChanged(void* userdata, uint32_t id, const struct spa_pod* param)
  {
    //Logger::log("Params changed !");
    PipewireData* pw = static_cast<PipewireData*>(userdata);

    if(param == NULL || id != SPA_PARAM_Format){
      return;
    }

    if(spa_format_parse(param, &pw->format.media_type, &pw->format.media_subtype) < 0){
      return;
    }

    if(pw->format.media_type != SPA_MEDIA_TYPE_video || pw->format.media_subtype != SPA_MEDIA_SUBTYPE_raw){
      return;
    }

    if(spa_format_video_raw_parse(param, &pw->format.info.raw) < 0){
      return;
    }

    /*
    std::stringstream ss;
    Logger::debug("got video format:");

    ss.str("  format: ");
    ss << pw->format.info.raw.format << spa_debug_type_find_name(spa_type_video_format, pw->format.info.raw.format);
    Logger::debug(ss.str());

    ss.str("  size: ");
    ss << pw->format.info.raw.size.width << "x" << pw->format.info.raw.size.height;
    Logger::debug(ss.str());

    ss.str("  framerate: ");
    ss << pw->format.info.raw.framerate.num << "/" << pw->format.info.raw.framerate.denom;
    Logger::debug(ss.str());
    */

    pw->ready.set_value(true);
  }


  void PipewireGrabber::_initCapture(XdgDesktopPortal::Capture* capture)
  {
    screencastPortalDesktopCaptureCreate(capture, XdgDesktopPortal::CaptureType::Monitor, true);

    GMainLoop* gmain = g_main_loop_new(NULL, FALSE);

    while(capture->updateXdgContext){
      g_main_context_iteration(g_main_loop_get_context(gmain), false);
    }

    g_main_loop_unref(gmain);
  }


  void PipewireGrabber::_pipewireThread(XdgDesktopPortal::Capture* capture, PipewireData* pw)
  {
    pw_init(NULL, NULL);

    pw_core_events coreEvents;
    coreEvents.version = PW_VERSION_CORE_EVENTS;
    coreEvents.info = _onCoreInfoCallback;
    coreEvents.done = _onCoreDoneCallback;
    coreEvents.error = _onCoreErrorCallback;

    pw_stream_events streamEvents;
    streamEvents.version = PW_VERSION_STREAM_EVENTS;
    streamEvents.param_changed = _onStreamParamChanged;
    streamEvents.process = _onStreamProcess;

    pw->loop = pw_thread_loop_new("PipeWire thread loop", NULL);
    pw->context = pw_context_new(pw_thread_loop_get_loop(pw->loop), NULL, 0);

    if(pw_thread_loop_start(pw->loop) < 0){
      throw std::runtime_error("Could not start thread loop");
    }

    pw_thread_loop_lock(pw->loop);

    auto core = pw_context_connect_fd(pw->context, fcntl(capture->pwFd, F_DUPFD_CLOEXEC, 5), NULL, 0);
    if(!core){
      Logger::error("Pipewire core creation error : Could not connect fd");
      pw_thread_loop_unlock(pw->loop);

      return;
    }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
    pw_core_add_listener(core, &pw->coreListener, &coreEvents, pw);

    auto props = pw_properties_new(
      PW_KEY_MEDIA_TYPE, "Video",
      PW_KEY_MEDIA_CATEGORY, "Capture",
      PW_KEY_MEDIA_ROLE, "Screen",
      NULL
    );

    std::string stream_name = "HuenicornStream";

    pw->stream = pw_stream_new(core, stream_name.c_str(), props);

    spa_hook streamListener;
    pw_stream_add_listener(pw->stream, &streamListener, &streamEvents, pw);


    uint8_t buffer[1024];
    spa_pod_builder b = SPA_POD_BUILDER_INIT(buffer, sizeof(buffer));

    auto r1 = spa_rectangle(320, 240);
    auto r2 = spa_rectangle(1, 1);
    auto r3 = spa_rectangle(4096, 4096);

    auto f1 = spa_fraction(25, 1);
    auto f2 = spa_fraction(0, 1);
    auto f3 = spa_fraction(1000, 1);

    const spa_pod* params[1] = {
      static_cast<spa_pod*>(
        spa_pod_builder_add_object(
          &b,
          SPA_TYPE_OBJECT_Format, SPA_PARAM_EnumFormat,
          SPA_FORMAT_mediaType,       SPA_POD_Id(SPA_MEDIA_TYPE_video),
          SPA_FORMAT_mediaSubtype,    SPA_POD_Id(SPA_MEDIA_SUBTYPE_raw),
          SPA_FORMAT_VIDEO_format,    SPA_POD_CHOICE_ENUM_Id(
            7,
            SPA_VIDEO_FORMAT_RGB,
            SPA_VIDEO_FORMAT_RGB,
            SPA_VIDEO_FORMAT_RGBA,
            SPA_VIDEO_FORMAT_RGBx,
            SPA_VIDEO_FORMAT_BGRx,
            SPA_VIDEO_FORMAT_YUY2,
            SPA_VIDEO_FORMAT_I420
          ),
          SPA_FORMAT_VIDEO_size,
          SPA_POD_CHOICE_RANGE_Rectangle(
            &r1,
            &r2,
            &r3
          ),
          SPA_FORMAT_VIDEO_framerate,
          SPA_POD_CHOICE_RANGE_Fraction(
            &f1,
            &f2,
            &f3
          )
        )
      )
    };
#pragma GCC diagnostic pop

    pw_stream_connect(pw->stream, PW_DIRECTION_INPUT, capture->pwNode, static_cast<pw_stream_flags>(PW_STREAM_FLAG_AUTOCONNECT | PW_STREAM_FLAG_MAP_BUFFERS), params, 1);
    pw_thread_loop_unlock(pw->loop);
  }


  void PipewireGrabber::_teardownPipewire()
  {
    pw_thread_loop_lock(m_pwData.loop);
    if (m_pwData.stream){
      pw_stream_disconnect(m_pwData.stream);
    }

    g_clear_pointer(&m_pwData.stream, pw_stream_destroy);
    pw_thread_loop_unlock(m_pwData.loop);

    if(m_pwData.loop){
      pw_thread_loop_stop(m_pwData.loop);
    }

    g_clear_pointer(&m_pwData.context, pw_context_destroy);
    g_clear_pointer(&m_pwData.loop, pw_thread_loop_destroy);

    if(m_capture.pwFd > 0){
      close(m_capture.pwFd);
      m_capture.pwFd = 0;
    }

    pw_deinit();
  }


  void PipewireGrabber::_stop()
  {
    // Stop Pipewire session
    _teardownPipewire();

    // Stop XdgPortal
    XdgDesktopPortal::screencastPortalCaptureDestroy(&m_capture);

    // Waiting for thread to finish
    if(m_pipewireThread.has_value()){
      m_pipewireThread.value().join();
      m_pipewireThread.reset();
    }

    if(m_xdgThread.has_value()){
      m_capture.updateXdgContext = false; // Making sure
      m_xdgThread.value().join();
      m_xdgThread.reset();
    }
  }
}
