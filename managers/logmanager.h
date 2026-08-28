#pragma once

namespace eclipse::managers{
    class LogManager{
      public:
        LogManager() = default;
        ~LogManager() = default;
        void Initialize();
        void Shutdown();
    };
  
}
