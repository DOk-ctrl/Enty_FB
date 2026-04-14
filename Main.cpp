#include <btBulletDynamicsCommon.h>
#include <memory>
#include <string>

#include "Window.h"
#include "Logger.h"

int main(int argc, char *argv[]) {

    if (!glfwInit()) {
        Logger::log(1, "Could not init GLFW\n");
        return -1;
    }

  std::unique_ptr<Window> w = std::make_unique<Window>();
  // полноэкранный режим
  
  
  GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
  const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);
  //
  if (!w->init(mode->width, mode->height, "Tentacle Shippuden: Enty Furry Beast")) {
      Logger::log(1, "%s error: Window init error\n", __FUNCTION__);
      return -1;
  }
  /*
  if (!w->init(960, 720, "Tentacle Shippuden: Enty Furry Beast")) {
    Logger::log(1, "%s error: Window init error\n", __FUNCTION__);
    return -1;
  }
  */
  w->mainLoop();

  w->cleanup();

  return 0;
}
