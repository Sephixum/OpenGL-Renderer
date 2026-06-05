#pragma once

#include "Services/ServiceLocator.hpp"
#include <chrono>

namespace glr
{

  class TimerService final : public IService
  {
    using Clock     = std::chrono::steady_clock;
    using TimePoint = Clock::time_point;
    using Duration  = Clock::duration;

    TimePoint     _last_frame;
    Duration      _delta;
    Duration      _elapsed;
    std::uint64_t _fram_count;
    double        _time_scale  = 1.0;

    public:
      virtual auto OnInit()     -> void override;
      virtual auto OnUpdate()   -> void override;
      virtual auto OnShutdown() -> void override;

      [[nodiscard]] auto GetDeltaTime()    const -> Duration;
      [[nodiscard]] auto GetDeltaSeconds() const -> double;
      [[nodiscard]] auto GetScaledDelta()  const -> Duration;
      [[nodiscard]] auto GetElapsedTime()  const -> Duration;
      [[nodiscard]] auto GetFrameCount()   const -> std::uint64_t;
      [[nodiscard]] auto GetTimeScale()    const -> double;
  };

}
