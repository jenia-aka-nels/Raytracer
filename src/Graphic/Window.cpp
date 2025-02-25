#include "Window.h"

Window::Window(std::tuple<int, int> resolution) {
  _resolution = resolution;
  glfwInit();

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

  _window = glfwCreateWindow(std::get<0>(resolution), std::get<1>(resolution), "Vulkan", nullptr, nullptr);
  glfwSetWindowUserPointer(_window, this);
  // glfwSetFramebufferSizeCallback(_window, framebufferResizeCallback);
}

std::vector<const char*> Window::getExtensions() {
  uint32_t glfwExtensionCount = 0;
  const char** glfwExtensions;
  glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

  return std::vector<const char*>(glfwExtensions, glfwExtensions + glfwExtensionCount);
}

GLFWwindow* Window::getWindow() { return _window; }

Window::~Window() {
  glfwDestroyWindow(_window);
  glfwTerminate();
}