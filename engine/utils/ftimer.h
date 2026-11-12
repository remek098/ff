#pragma once

#include "common/primitive_types.h"
#include <chrono>

namespace ff::utils {

class ftimer {
  public:
    using clock      = std::chrono::steady_clock;
    using time_stamp = clock::time_point;

    f32           dt() const { return _dt; }
    constexpr f32 dt_avg() const { return _dt_avg; } // seconds
    constexpr f32 fps_avg() const { return _fps_avg; }

    void          begin() { _start = clock::now(); }

    // Returns true when statistics have been refreshed.
    bool end() {
        const auto now = clock::now();
        const auto dt  = now - _start;

        _dt            = std::chrono::duration<float>(dt).count();

        _total_frame_time += dt;
        ++_counter;

        const auto elapsed = now - _seconds;

        if (elapsed >= std::chrono::seconds{ 1 }) {
            _dt_avg           = std::chrono::duration<f32>(_total_frame_time).count() / static_cast<f32>(_counter);

            _fps_avg          = static_cast<f32>(_counter) / std::chrono::duration<f32>(elapsed).count();

            _total_frame_time = clock::duration::zero();
            _counter          = 0;
            _seconds          = now;

            return true;
        }

        return false;
    }

  private:
    f32             _dt{ 0.f };
    f32             _dt_avg{ 0.0167f };
    f32             _fps_avg{ 60.f };

    u32             _counter{ 0 };

    clock::duration _total_frame_time{};

    time_stamp      _start{};
    time_stamp      _seconds{ clock::now() };
};
} // namespace ff::utils