#pragma once

#include <string_view>

namespace crystal::lab {

/// A deliberately small contract for a deterministic physics experiment.
///
/// The simulation owns its model state. The core owns neither rendering,
/// input, threads, events, nor global state.
class Simulation {
public:
    virtual ~Simulation() = default;

    [[nodiscard]] virtual std::string_view name() const noexcept = 0;

    /// Restore the simulation to a reproducible initial state.
    virtual void reset() = 0;

    /// Advance the model by one fixed logical time step.
    virtual void step(double dt_seconds) = 0;

    /// Optional finite-run hook for experiments that have a natural endpoint.
    [[nodiscard]] virtual bool finished() const noexcept { return false; }
};

}  // namespace crystal::lab
