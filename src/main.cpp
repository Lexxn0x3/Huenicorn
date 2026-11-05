#include <memory>
#include <string>
#include <thread>

// Huenicorn headers (pulling in Crow) must come before Qt headers because of
// the Qt "signals" macro clashing with Crow's signals() method
#include <Huenicorn/Version.hpp>
#include <Huenicorn/HuenicornCore.hpp>
#include <Huenicorn/Logger.hpp>
#include <Huenicorn/PlatformSelector.hpp>

#include <QCoreApplication>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQmlError>
#include <QWindow>

#include <Huenicorn/Gui/QtController.hpp>


int main(int argc, char *argv[])
{
  Huenicorn::Logger::log("Starting Huenicorn version ", Huenicorn::Version);

  // The legacy web UI backend is kept as an opt-in headless interface
  bool webUIEnabled = false;
  for(int i = 1; i < argc; ++i){
    if(std::string(argv[i]) == "--web"){
      webUIEnabled = true;
    }
  }

  QGuiApplication app(argc, argv);
  QCoreApplication::setApplicationName("huenicorn");
  QCoreApplication::setOrganizationName("huenicorn");

  auto core = std::make_unique<Huenicorn::HuenicornCore>(Huenicorn::Version, Huenicorn::platformAdapter.getConfigFilePath());
  core->setWebUIEnabled(webUIEnabled);

  std::thread coreThread([&core](){ core->start(); });

  Huenicorn::QtController controller(core.get());

  QQmlApplicationEngine engine;
  engine.rootContext()->setContextProperty("huenicorn", &controller);

  // Forward QML warnings/errors to the log so UI issues are visible from a terminal
  QObject::connect(
      &engine,
      &QQmlApplicationEngine::warnings,
      &app,
      [](const QList<QQmlError>& warnings){
        for(const QQmlError& warning : warnings){
          Huenicorn::Logger::error("QML: ", warning.toString().toStdString());
        }
      });

  QObject::connect(
      &engine,
      &QQmlApplicationEngine::objectCreationFailed,
      &app,
      []() {
        Huenicorn::Logger::error("Failed to create the QML interface");
        QCoreApplication::exit(-1);
      },
      Qt::QueuedConnection);

  engine.loadFromModule("huenicorn_UI", "Main");

  const auto rootObjects = engine.rootObjects();
  if(rootObjects.isEmpty()){
    Huenicorn::Logger::error("QML root object was not created, no window will be shown");
  }
  else{
    if(auto* window = qobject_cast<QWindow*>(rootObjects.first())){
      window->showNormal();
      window->requestActivate();
    }
    Huenicorn::Logger::log("Qt GUI window shown");
  }

  QObject::connect(&app, &QGuiApplication::aboutToQuit, [&](){
    core->stop();
    if(coreThread.joinable()){
      coreThread.join();
    }
  });

  int result = app.exec();

  core->stop();
  if(coreThread.joinable()){
    coreThread.join();
  }

  return result;
}
