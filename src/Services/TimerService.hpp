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
    float         _time_scale  = 1.0f;

    public:
      virtual auto OnInit()     -> void override;
      virtual auto OnUpdate()   -> void override;
      virtual auto OnShutdown() -> void override;

      [[nodiscard]] auto DeltaTime()    const -> Duration;
      [[nodiscard]] auto DeltaSeconds() const -> double;
      [[nodiscard]] auto ScaledDelta()  const -> Duration;
      [[nodiscard]] auto ElapsedTime()  const -> Duration;
      [[nodiscard]] auto FrameCount()   const -> std::uint64_t;
  };

}
