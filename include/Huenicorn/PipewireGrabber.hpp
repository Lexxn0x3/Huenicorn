#pragma once

#include <Huenicorn/IGrabber.hpp>

#include <mutex>
#include <optional>
#include <thread>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#include <pipewire/pipewire.h>
#include <spa/param/video/format-utils.h>
#pragma GCC diagnostic pop

#include <Huenicorn/XdgDesktopPortal.hpp>


namespace Huenicorn
{
  /**
   * Pipewire implementation of screen grabber instanciated for Wayland support
  */
  class PipewireGrabber : public IGrabber
  {
  private:
    /**
     * Thread-safe double buffer for grabbed frames
    */
    struct SafeDoubleBuffer
    {
      std::array<cv::Mat, 2> frame;
      std::mutex mutex;
    };


    /**
     * Pipewire-specific data shared between processes
    */
    struct PipewireData
    {
      spa_hook coreListener;
      pw_thread_loop* loop;
      pw_context* context;
      pw_stream* stream;
      spa_video_info format;
      SafeDoubleBuffer frameDoubleBuffer;
      std::promise<bool> ready;
      Config* config;
    };


  public:
    // Constructor / destructor
    /**
     * PipewireGrabber constructor
    */
    PipewireGrabber(Config* config);


    /**
     * PipewireGrabber destructor
    */
    virtual ~PipewireGrabber();


    /**
     * @brief Returns the resolution of the selected display
     * 
     * @return Resolution Resolution of the selected display
     */
    virtual Resolution displayResolution() const override;


    /**
     * @brief Returns the resolution of the selected display
     * 
     * @return Resolution Resolution of the selected display
     */
    virtual RefreshRate displayRefreshRate() const override;


    // Methods
    /**
     * @brief Returns a subsample of the last captured frame
     * 
     * @param imageData Subsample of the last captured frame
     */
    virtual void grabFrameSubsample(cv::Mat& cvImage) override;


  private:
    // Methods
    static void _onCoreInfoCallback(void* userData, const pw_core_info* info);

    static void _onCoreDoneCallback(void* userData, uint32_t id, int seq);

    static void _onCoreErrorCallback(void* userData, uint32_t id, int seq, int res, const char* message);

    static void _onStreamProcess(void* userdata);

    static void _onStreamParamChanged(void* userdata, uint32_t id, const struct spa_pod* param);

    /**
     * Screencast portal thread
    */
    static void _initCapture(XdgDesktopPortal::Capture* capture);


    /**
     * Pipewire thread
    */
    static void _pipewireThread(XdgDesktopPortal::Capture* capture, PipewireData* pw);


    /**
     * Proper shutdown for Pipewire session
    */
    void _teardownPipewire();


    /**
     * Terminates all inner executions
    */
    void _stop();


    // Attributes
    std::optional<std::thread> m_xdgThread;
    std::optional<std::thread> m_pipewireThread;
    XdgDesktopPortal::Capture m_capture;
    PipewireData m_pwData;
  };
}
