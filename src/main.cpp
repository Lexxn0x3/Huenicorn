#include <thread>

#include <Huenicorn/HuenicornCore.hpp>
#include <csignal>

#include <pwd.h>

#define xstr(s) preprocess_str(s)
#define preprocess_str(s) #s
static constexpr std::string Version = xstr(PROJECT_VERSION);


using namespace std;


filesystem::path getConfigRoot()
{
  const char* homeDir;
  if((homeDir = getenv("HOME")) == NULL){
    homeDir = getpwuid(getuid())->pw_dir;
  }

  return std::filesystem::path(homeDir) / ".config/huenicorn";
}


/**
 * @brief Wrapper around threaded application
 * 
 */
class Application
{
public:
  void start()
  {
    m_core = make_unique<Huenicorn::HuenicornCore>(Version, getConfigRoot());
    m_applicationThread.emplace([&](){
      m_core->start();
    });

    m_applicationThread.value().join();
    m_applicationThread.reset();
    m_core.reset();
  }


  void stop()
  {
    if(!m_core){
      return;
    }

    m_core->stop();
  }

private:
  unique_ptr<Huenicorn::HuenicornCore> m_core;
  std::optional<thread> m_applicationThread;
};


Application app;


void signalHandler(int signal)
{
  if(signal == SIGTERM || signal == SIGINT || signal == SIGTSTP){
    cout << "Closing application" << endl;
    app.stop();
  }
}


int main()
{
  cout << "Starting Huenicorn version " << Version << endl;

  signal(SIGTERM, signalHandler);
  signal(SIGINT, signalHandler);
  signal(SIGTSTP, signalHandler);

  app.start();
  cout << "Huenicorn terminated properly" << endl;

  return 0;
}
