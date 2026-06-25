#pragma once

#include <cstddef>
#include <vector>

#include "crystal_lab/core/Simulation.hpp"

namespace crystal::lab::demo {

struct String1DConfig {
    std::size_t grid_points = 201;
    double length_m = 0.65;
    double tension_n = 80.0;
    double linear_density_kg_per_m = 0.006;
    double pluck_position_fraction = 0.22;
    double pluck_amplitude_m = 0.004;
};

/// Finite-difference model of a taut string with fixed endpoints.
///
/// Equation:
///     u_tt = c^2 u_xx,  c = sqrt(T / mu)
///
/// The implementation uses a centered second-order explicit scheme. Its CFL
/// condition in one dimension is c * dt / dx <= 1.
class String1DSimulation final : public Simulation {
public:
    explicit String1DSimulation(String1DConfig config = {});

    [[nodiscard]] std::string_view name() const noexcept override;
    void reset() override;
    void step(double dt_seconds) override;

    [[nodiscard]] bool finished() const noexcept override { return false; }

    [[nodiscard]] double wave_speed_m_per_s() const noexcept;
    [[nodiscard]] double dx_m() const noexcept;
    [[nodiscard]] double cfl_number(double dt_seconds) const noexcept;
    [[nodiscard]] double displacement_m(std::size_t index) const;
    [[nodiscard]] std::size_t grid_points() const noexcept;
    [[nodiscard]] double total_energy_j(double dt_seconds) const;

private:
    String1DConfig config_;
    double dx_m_ = 0.0;
    double wave_speed_m_per_s_ = 0.0;
    std::vector<double> previous_;
    std::vector<double> current_;
    std::vector<double> next_;
};

}  // namespace crystal::lab::demo
