#pragma once
#include "spdlog/spdlog.h"
#include "../managers/logmanager.h"

#define ECLIPSE_DEFAULT_LOGGER_NAME "eclipselogger"


#if defined (ECLIPSE_PLATFORM_WINDOWS)
#define ECLIPSE_BREAK __debugbreak();
#elif defined (ECLIPSE_PLATFORM_MAC)
#define ECLIPSE_BREAK __builtin_debugtrap();
#else
#define ECLIPSE_BREAK __builtin_trap();
#endif


#ifndef ECLIPSE_CONFIG_RELEASE
#define ECLIPSE_TRACE(...)                                                     \
  if (spdlog::get(ECLIPSE_DEFAULT_LOGGER_NAME) != nullptr) {                   \
    spdlog::get(ECLIPSE_DEFAULT_LOGGER_NAME)->trace(__VA_ARGS__);               \
  }
#define ECLIPSE_DEBUG(...)                                                     \
  if (spdlog::get(ECLIPSE_DEFAULT_LOGGER_NAME) != nullptr) {                   \
    spdlog::get(ECLIPSE_DEFAULT_LOGGER_NAME)->debug(__VA_ARGS__);               \
  }
#define ECLIPSE_WARN(...)                                                     \
  if (spdlog::get(ECLIPSE_DEFAULT_LOGGER_NAME) != nullptr) {                   \
    spdlog::get(ECLIPSE_DEFAULT_LOGGER_NAME)->warn(__VA_ARGS__);               \
  }
#define ECLIPSE_INFO(...)                                                     \
  if (spdlog::get(ECLIPSE_DEFAULT_LOGGER_NAME) != nullptr) {                   \
    spdlog::get(ECLIPSE_DEFAULT_LOGGER_NAME)->info(__VA_ARGS__);               \
  }
#define ECLIPSE_ERROR(...)                                                     \
  if (spdlog::get(ECLIPSE_DEFAULT_LOGGER_NAME) != nullptr) {                   \
    spdlog::get(ECLIPSE_DEFAULT_LOGGER_NAME)->error(__VA_ARGS__);               \
  }
#define ECLIPSE_FATAL(...)                                                     \
  if (spdlog::get(ECLIPSE_DEFAULT_LOGGER_NAME) != nullptr) {                   \
    spdlog::get(ECLIPSE_DEFAULT_LOGGER_NAME)->critical(__VA_ARGS__);               \
  }
#define ECLIPSE_ASSERT(x, msg) if ((x)) {} else { ECLIPSE_FATAL(" ASSERT - {}\n\t {}\n\t in file: {}\n\t on line {}", #x, msg, __FILE__, __LINE__); }
#else
#define ECLIPSE_TRACE(...) (void)0
#define ECLIPSE_DEBUG(...) (void)0
#define ECLIPSE_INFO(...) (void)0
#define ECLIPSE_WARN(...) (void)0
#define ECLIPSE_ERROR(...) (void)0
#define ECLIPSE_FATAL(...) (void)0
#endif
