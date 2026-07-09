#pragma once

#include <spdlog/spdlog.h>

#include "Core.h"

namespace Sky::RHI::Log
{
  void init();
  void shutdown() noexcept;

  [[nodiscard]] Ref<spdlog::logger>& get();
}

// ---------- SkyRHI logging macros ----------
// Все макросы префиксом SKY_RHI_ чтобы не конфликтовать с consumer'ом (SlyEngine).
#define SKY_RHI_TRACE(...)    ::Sky::RHI::Log::get()->trace(__VA_ARGS__)
#define SKY_RHI_INFO(...)     ::Sky::RHI::Log::get()->info(__VA_ARGS__)
#define SKY_RHI_WARN(...)     ::Sky::RHI::Log::get()->warn(__VA_ARGS__)
#define SKY_RHI_ERROR(...)    ::Sky::RHI::Log::get()->error(__VA_ARGS__)
#define SKY_RHI_CRITICAL(...) ::Sky::RHI::Log::get()->critical(__VA_ARGS__)

// VkResult проверка с логированием.
#define SKY_RHI_VK_CHECK(expr, msg)                                             \
    do {                                                                        \
        VkResult _res = (expr);                                                 \
        if (_res != VK_SUCCESS) {                                               \
            SKY_RHI_ERROR("{} (VkResult = {})", msg, static_cast<int>(_res));   \
            throw std::runtime_error(msg);                                      \
        }                                                                       \
    } while (0)
