#include "skypch.h"

#include <spdlog/sinks/stdout_color_sinks.h>

namespace Sky::RHI::Log
{
  static std::shared_ptr<spdlog::logger> s_Logger;

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

  std::shared_ptr<spdlog::logger>& get() { return s_Logger; }
} // namespace Sky::RHI::Log
