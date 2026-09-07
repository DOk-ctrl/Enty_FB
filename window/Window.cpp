#include "stb_image.h"
#include "Window.h"
#include "Logger.h"
#include "tiny_gltf.h"
bool Window::init(unsigned int width, unsigned int height, std::string title) {

  /* set a "hint" for the NEXT window created*/
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  mWindow = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
 
  if (!mWindow) {
    glfwTerminate();
    Logger::log(1, "%s error: Could not create window\n", __FUNCTION__);
    return false;
  }
  // Иконочка
  GLFWimage images[1];
  int channels;

  images[0].pixels = stbi_load(R"(C:\Dev\02_opengl_multiple_models\window\logo.png)", &images[0].width, &images[0].height, &channels, 4);

  if (images[0].pixels) {
      glfwSetWindowIcon(mWindow, 1, images);
      stbi_image_free(images[0].pixels);
      Logger::log(0, "Icon loaded successfully!\n");
  }
  else {
      // Выведет причину ошибки от stb_image
      Logger::log(1, "STB Error: %s\n", stbi_failure_reason());
  }

  glfwMakeContextCurrent(mWindow);

  mRenderer = std::make_unique<OGLRenderer>(mWindow);

  glfwSetWindowUserPointer(mWindow, mRenderer.get());
  glfwSetWindowSizeCallback(mWindow, [](GLFWwindow *win, int width, int height) {
      auto renderer = static_cast<OGLRenderer*>(glfwGetWindowUserPointer(win));
      renderer->setSize(width, height);
    }
  );

  glfwSetKeyCallback(mWindow, [](GLFWwindow* win, int key, int scancode, int action, int mods) {
      auto renderer = static_cast<OGLRenderer*>(glfwGetWindowUserPointer(win));
      renderer->handleKeyEvents(key, scancode, action, mods);
    }
  );

  glfwSetMouseButtonCallback(mWindow, [](GLFWwindow *win, int button, int action, int mods) {
      auto renderer = static_cast<OGLRenderer*>(glfwGetWindowUserPointer(win));
      renderer->handleMouseButtonEvents(button, action, mods);
    }
  );

  glfwSetCursorPosCallback(mWindow, [](GLFWwindow *win, double xpos, double ypos) {
      auto renderer = static_cast<OGLRenderer*>(glfwGetWindowUserPointer(win));
      renderer->handleMousePositionEvents(xpos, ypos);
    }
  );
  /*
  if (!mRenderer->init(width, height)) {
    glfwTerminate();
    Logger::log(1, "%s error: Could not init OpenGL\n", __FUNCTION__);
    return false;
  }
  */
  if (!mRenderer->initEarly(width, height)) {
      glfwTerminate();
      Logger::log(1, "%s error: Could not init OpenGL\n", __FUNCTION__);
      return false;
  }

  Logger::log(1, "%s: Window with OpenGL 4.6 successfully initialized\n", __FUNCTION__);
  return true;
}


void Window::mainLoop() {
  /* force VSYNC */
  glfwSwapInterval(1);
  
 // while (!glfwWindowShouldClose(mWindow)) {
  //  mRenderer->draw();

    /* swap buffers */
 //   glfwSwapBuffers(mWindow);

    /* poll events in a loop */
 //   glfwPollEvents();

 // }
  while (!glfwWindowShouldClose(mWindow)) {
      if (!mRenderer->isReady()) {
          if (!mRenderer->loadFailed()) {
              mRenderer->updateLoading(); // делает ОДИН шаг state machine за кадр
          }
          mRenderer->drawLoadingScreen();
      }
      else {
          mRenderer->draw(); // твой обычный рендер, без изменений
      }

      glfwSwapBuffers(mWindow);
      glfwPollEvents();
  }
}

void Window::cleanup() {
  mRenderer->cleanup();

  glfwDestroyWindow(mWindow);
  glfwTerminate();
  Logger::log(1, "%s: Terminating Window\n", __FUNCTION__);
}
