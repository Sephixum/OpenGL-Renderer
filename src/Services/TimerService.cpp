#include "TimerService.hpp"
#include <chrono>

namespace glr
{

  auto TimerService::OnInit() -> void 
  {
    _last_frame = Clock::now();
    _delta      = Duration::zero();
    _elapsed    = Duration::zero();
  } 

  auto TimerService::OnUpdate() -> void 
  {
    auto now     = Clock::now();
    _delta      = now - _last_frame;
    _last_frame = now;

    _elapsed += _delta;
    ++_last_frame;

  } 

  auto TimerService::OnShutdown() -> void {} 

  auto TimerService::GetDeltaTime() const -> Duration { return _delta; }

  auto TimerService::GetDeltaSeconds() const -> double 
  {
    return std::chrono::duration<double>(_delta).count();
  }

  auto TimerService::GetScaledDelta() const -> Duration 
  {
    return std::chrono::duration_cast<Duration>(_delta * _time_scale);
  }

  auto TimerService::GetElapsedTime() const -> Duration 
  {
    return _elapsed;
  }

  auto TimerService::GetFrameCount() const -> std::uint64_t 
  {
    return _fram_count;
  }

  auto TimerService::GetTimeScale() const -> double
  {
    return _time_scale;
  }

}
