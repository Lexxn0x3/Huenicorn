#include <Huenicorn/X11Grabber.hpp>

#include <err.h>

#include <cstring>

#include <X11/Xutil.h>
#include <X11/extensions/Xrandr.h>
#include <sys/shm.h>

#include <Huenicorn/Config.hpp>
#include <Huenicorn/ImageProcessing.hpp>


namespace Huenicorn
{
  X11Grabber::X11Grabber(Config* config):
  IGrabber(config)
  {
    if(!(m_display = XOpenDisplay(0))){
      errx(1, "cannot open display '%s'", XDisplayName(0));
    }

    int screenId = XDefaultScreen(m_display);

    m_shmInfo = std::make_unique<XShmSegmentInfo>();

    Screen* screen = ScreenOfDisplay(m_display, 0);

    m_ximage = XShmCreateImage(m_display,
      DefaultVisual(m_display, screenId),
      DefaultDepth(m_display, screenId),
      ZPixmap,
      NULL,
      m_shmInfo.get(),
      screen->width,
      screen->height
    );

    m_shmInfo->shmid = shmget(IPC_PRIVATE, m_ximage->bytes_per_line * m_ximage->height, IPC_CREAT | 0777);

    m_shmInfo->readOnly = False;
    m_shmInfo->shmaddr = m_ximage->data = reinterpret_cast<char*>(shmat(m_shmInfo->shmid, 0, 0));

    XShmAttach(m_display, m_shmInfo.get());
  }


  X11Grabber::~X11Grabber()
  {
    if(m_shmInfo) {
      shmdt(m_shmInfo->shmaddr);
      shmctl(m_shmInfo->shmid, IPC_RMID, 0);
      XShmDetach(m_display, m_shmInfo.get());
      m_shmInfo.reset();
    }

    if(m_ximage){
      XDestroyImage(m_ximage);
      m_ximage = nullptr;
    }

    if(m_display){
      XCloseDisplay(m_display);
      m_display = nullptr;
    }
  }


  glm::ivec2 X11Grabber::displayResolution() const
  {
    int width = 0;
    int height = 0;
    int screenId = 0;

    screenId = DefaultScreen(m_display);
    width = DisplayWidth(m_display, screenId);
    height = DisplayHeight(m_display, screenId);

    return {width, height};
  }


  void X11Grabber::grabFrameSubsample(cv::Mat& cvImage)
  {
    Window root = DefaultRootWindow(m_display);
    int screenId = XDefaultScreen(m_display);

    if(!m_imageData.has_value()){
      XWindowAttributes attributes = XWindowAttributes();
      XGetWindowAttributes(m_display, root, &attributes);
      m_imageData.emplace();
      m_imageData->width = attributes.width;
      m_imageData->height = attributes.height;
      m_imageData->pixels.resize(m_imageData->width * m_imageData->height * 4);
    }

    XShmGetImage(m_display, RootWindow(m_display, screenId), m_ximage, 0, 0, AllPlanes);

    m_imageData->bitsPerPixel = m_ximage->bits_per_pixel;

    memcpy(m_imageData->pixels.data(), m_ximage->data, m_imageData->pixels.size());

    cv::Mat image;
    if(m_imageData->bitsPerPixel > 24){
      image = cv::Mat(m_imageData->height, m_imageData->width, CV_8UC4, m_imageData->pixels.data());
    }
    else{
      image = cv::Mat(m_imageData->height, m_imageData->width, CV_8UC3, m_imageData->pixels.data());
    }

    ImageProcessing::rescale(image, m_config->subsampleWidth(), m_config->interpolation());

    if(image.channels() == 4){
      cv::cvtColor(image, image, cv::COLOR_RGBA2RGB);
    }

    cvImage = std::move(image);
  }


  IGrabber::RefreshRate X11Grabber::displayRefreshRate() const
  {
    int screenId = XDefaultScreen(m_display);
    Window root = RootWindow(m_display, screenId);
    XRRScreenConfiguration* displayConfig = XRRGetScreenInfo(m_display, root);
    RefreshRate currentRate = XRRConfigCurrentRate(displayConfig);

    return currentRate;
  }
}
