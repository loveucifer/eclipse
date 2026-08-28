#include "logmanager.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"
#include "../src/log.h"
#include <memory>

namespace eclipse::managers{
  void LogManager::Initialize(){
    auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    // [year/month/day  hour::minute::second::millilsecond  logmessage]
    consoleSink -> set_pattern("%^[%Y-%m-%d  %H:%M:%S.%e] %v%$"); // v is the actual msg , dollar sign marks the end

    std::vector<spdlog::sink_ptr> sinks {consoleSink};
    auto Logger = std::make_shared<spdlog::logger>(ECLIPSE_DEFAULT_LOGGER_NAME,sinks.begin(),sinks.end());
    Logger -> set_level(spdlog::level::trace);

    Logger -> flush_on(spdlog::level::trace);

    spdlog::register_logger(Logger);

  }
  void LogManager::Shutdown(){

    spdlog::shutdown();
        
  }
}
