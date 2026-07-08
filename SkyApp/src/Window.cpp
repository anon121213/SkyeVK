#include "Window.h"

#include <stdexcept>

Window::Window(const uint32_t width, const uint32_t height, const std::string& title)
{
  if (!glfwInit())
  {
    throw std::runtime_error("GLFW initialization failed");
  }

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  m_Window = glfwCreateWindow(static_cast<int>(width), static_cast<int>(height), title.c_str(), nullptr, nullptr);

  if (m_Window == nullptr)
  {
    glfwTerminate();
    throw std::runtime_error("Failed to create GLFW window");
  }
}

Window::~Window()
{
  glfwDestroyWindow(m_Window);
  glfwTerminate();
}

bool Window::shouldClose() const
{
  return glfwWindowShouldClose(m_Window);
}

void Window::pollEvents()
{
  glfwPollEvents();
}

GLFWwindow* Window::handle() const
{
  return m_Window;
}

VkSurfaceKHR Window::createSurface(VkInstance instance) const
{
  VkSurfaceKHR surface;

  if (glfwCreateWindowSurface(instance, m_Window, nullptr, &surface) != VK_SUCCESS)
    throw std::runtime_error("Failed to create window surface");

  return surface;
}

std::vector<const char*> Window::getRequiredInstanceExtensions()
{
  uint32_t count = 0;
  const auto glfwExtensions = glfwGetRequiredInstanceExtensions(&count);

  if (!glfwExtensions)
    throw std::runtime_error("GLFW extensions not available");

  const std::vector extensions(glfwExtensions, glfwExtensions + count);
  return extensions;
}