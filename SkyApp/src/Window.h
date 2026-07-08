#pragma once

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <string>
#include <vector>

class Window
{
public:
  Window(uint32_t width, uint32_t height, const std::string& title);
  ~Window();

  Window(const Window&) = delete;
  Window& operator=(const Window&) = delete;

  static void pollEvents();

  [[nodiscard]] bool shouldClose() const;
  [[nodiscard]] GLFWwindow* handle() const;
  [[nodiscard]] VkSurfaceKHR createSurface(VkInstance instance) const;
  [[nodiscard]] static std::vector<const char*> getRequiredInstanceExtensions();

private:
  GLFWwindow* m_Window = nullptr;

};