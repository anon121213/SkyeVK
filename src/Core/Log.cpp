#include "skypch.h"

#include <spdlog/sinks/stdout_color_sinks.h>

namespace Sky::RHI::Log
{
  static Ref<spdlog::logger> s_Logger;

  void init()
  {
    spdlog::set_pattern("%^[%Y-%m-%d %T.%e] [%n] [%l]%$ %v");

    s_Logger = spdlog::stdout_color_mt("SkyRHI");
    s_Logger->set_level(spdlog::level::trace);
  }

  void shutdown() noexcept
  {
    spdlog::shutdown();
  }

  Ref<spdlog::logger>& get() { return s_Logger; }
} // namespace Sky::RHI::Log
