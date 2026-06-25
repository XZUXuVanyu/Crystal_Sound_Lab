#pragma once

#include <cstddef>
#include <functional>
#include <stdexcept>

#include "crystal_lab/core/Simulation.hpp"

namespace crystal::lab {

struct RunConfig {
    double dt_seconds = 0.0;
    std::size_t max_steps = 0;
    std::size_t observe_every = 1;
};

struct RunState {
    std::size_t step_index = 0;
    double time_seconds = 0.0;
};

using Observer = std::function<void(const RunState&, const Simulation&)>;

/// Run a simulation deterministically on the calling thread.
///
/// This is intentionally headless and synchronous. A window, audio device, or
/// input source may call this runner in chunks later, but physics correctness
/// must not depend on a GUI event loop.
inline RunState run_fixed_steps(
    Simulation& simulation,
    const RunConfig& config,
    const Observer& observer = {}) {
    if (config.dt_seconds <= 0.0) {
        throw std::invalid_argument("RunConfig.dt_seconds must be positive");
    }
    if (config.max_steps == 0) {
        throw std::invalid_argument("RunConfig.max_steps must be non-zero");
    }
    if (config.observe_every == 0) {
        throw std::invalid_argument("RunConfig.observe_every must be non-zero");
    }

    simulation.reset();
    RunState state{};

    if (observer) {
        observer(state, simulation);
    }

    while (state.step_index < config.max_steps && !simulation.finished()) {
        simulation.step(config.dt_seconds);
        ++state.step_index;
        state.time_seconds += config.dt_seconds;

        if (observer && (state.step_index % config.observe_every == 0 || simulation.finished())) {
            observer(state, simulation);
        }
    }

    return state;
}

}  // namespace crystal::lab
