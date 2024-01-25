#pragma once
/*
  This code is an adapation from https://codeberg.org/metamuffin/xdg-dp-start/src/branch/master/xdg-dp-start.c
  It consists on an extraction from OBS Source code achieving the call and management of a xdg-portal capture query
  As I spent days to do try to achieve the same result in a less concise way, I feel more confident using this one and give it some personnal changes
*/


#include <future>
#include <string>

#include <gio/gunixfdlist.h>


namespace Huenicorn
{
  namespace XdgDesktopPortal
  {
    // Type definitions
    using StringPair = std::pair<std::string, std::string>;

    // Enums
    enum CaptureType
    {
      Monitor = 1 << 0,
      //Window = 1 << 1,
      //Virtual = 1 << 2,
    };


    enum CursorMode
    {
      Hidden = 1 << 0,
      Embedded = 1 << 1,
      Metadata = 1 << 2,
    };


    enum class CreatePathTokenType
    {
      Request = 0,
      Session = 1
    };


    // Structures
    struct Capture
    {
      CaptureType captureType;
      GCancellable* cancellable;
      char* sessionHandle;
      uint32_t pwNode;
      uint32_t pwFd;
      char cursorVisible;
      std::promise<bool> fdReadyPromise;
      bool updateXdgContext{true};
    };


    struct DbusCallData
    {
      Capture* capture;
      std::string requestPath;
      guint signalId;
      gulong cancelledId;
    };


    // Functions
    void ensureConnection();
    std::string getSenderName();
    GDBusConnection* portalGetDbusConnection();
    StringPair portalCreatePath(CreatePathTokenType createPathTokenType);
    void ensureScreencastPortalProxy();
    GDBusProxy* getScreencastPortalProxy();
    uint32_t getAvailableCursorModes();
    uint32_t getScreencastVersion();
    void onCancelledCallback(GCancellable* cancellable, void* data);
    DbusCallData* subscribeToSignal(Capture* capture, const char* path, GDBusSignalCallback callback);
    void dbusCallDataFree(DbusCallData* call);
    void onPipewireRemoteOpenedCallback(GObject* source, GAsyncResult* res, void* userData);
    void openPipewireRemote(Capture* capture);
    void onStartResponseReceivedCallback(GDBusConnection* connection, const char* sender_name, const char* object_path, const char* interfaceName, const char* signalName, GVariant* parameters, void* userData);
    void onStartedCallback(GObject* source, GAsyncResult* res, void* userData);
    void start(Capture* capture);
    void onSelectSourceResponseReceivedCallback(GDBusConnection* connection, const char* senderName, const char* objectPath, const char* interfaceName, const char* signalName, GVariant* parameters, void* userData);
    void onSourceSelectedCallback(GObject* source, GAsyncResult* res, void* userData);
    void selectSource(Capture* capture);
    void onCreateSessionResponseReceivedCallback(GDBusConnection* connection, const char* senderName, const char* objectPath, const char* interfaceName, const char* signalName, GVariant* parameters, void* userData);
    void onSessionCreatedCallback(GObject* source, GAsyncResult* res, void* userData);
    void createSession(Capture* capture);
    bool initScreencastCapture(Capture* capture);
    void screencastPortalDesktopCaptureCreate(Capture* capture, CaptureType captureType, bool cursorVisible);
    void screencastPortalCaptureDestroy(Capture* capture);
  }
}
