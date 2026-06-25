#include "string_1d/String1DSimulation.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace crystal::lab::demo {

String1DSimulation::String1DSimulation(String1DConfig config)
    : config_(config),
      previous_(config.grid_points, 0.0),
      current_(config.grid_points, 0.0),
      next_(config.grid_points, 0.0) {
    if (config_.grid_points < 3) {
        throw std::invalid_argument("String1DConfig.grid_points must be at least 3");
    }
    if (config_.length_m <= 0.0 || config_.tension_n <= 0.0 || config_.linear_density_kg_per_m <= 0.0) {
        throw std::invalid_argument("String1D physical parameters must be positive");
    }
    if (config_.pluck_position_fraction <= 0.0 || config_.pluck_position_fraction >= 1.0) {
        throw std::invalid_argument("pluck_position_fraction must lie strictly between 0 and 1");
    }

    dx_m_ = config_.length_m / static_cast<double>(config_.grid_points - 1);
    wave_speed_m_per_s_ = std::sqrt(config_.tension_n / config_.linear_density_kg_per_m);
    reset();
}

std::string_view String1DSimulation::name() const noexcept {
    return "1D taut string";
}

void String1DSimulation::reset() {
    std::fill(previous_.begin(), previous_.end(), 0.0);
    std::fill(current_.begin(), current_.end(), 0.0);
    std::fill(next_.begin(), next_.end(), 0.0);

    // Triangular pluck. previous_ == current_ encodes zero initial velocity.
    for (std::size_t i = 0; i < config_.grid_points; ++i) {
        const double x_fraction = static_cast<double>(i) / static_cast<double>(config_.grid_points - 1);
        const double displacement = x_fraction <= config_.pluck_position_fraction
            ? config_.pluck_amplitude_m * x_fraction / config_.pluck_position_fraction
            : config_.pluck_amplitude_m * (1.0 - x_fraction) / (1.0 - config_.pluck_position_fraction);
        current_[i] = displacement;
        previous_[i] = displacement;
    }

    current_.front() = current_.back() = 0.0;
    previous_.front() = previous_.back() = 0.0;
}

void String1DSimulation::step(double dt_seconds) {
    if (dt_seconds <= 0.0) {
        throw std::invalid_argument("String1DSimulation requires a positive time step");
    }

    const double lambda = cfl_number(dt_seconds);
    if (lambda > 1.0) {
        throw std::runtime_error("CFL violation: c * dt / dx must not exceed 1 for this explicit scheme");
    }

    const double lambda_squared = lambda * lambda;
    next_.front() = 0.0;
    next_.back() = 0.0;

    for (std::size_t i = 1; i + 1 < config_.grid_points; ++i) {
        next_[i] = 2.0 * current_[i] - previous_[i]
            + lambda_squared * (current_[i + 1] - 2.0 * current_[i] + current_[i - 1]);
    }

    previous_.swap(current_);
    current_.swap(next_);
}

double String1DSimulation::wave_speed_m_per_s() const noexcept {
    return wave_speed_m_per_s_;
}

double String1DSimulation::dx_m() const noexcept {
    return dx_m_;
}

double String1DSimulation::cfl_number(double dt_seconds) const noexcept {
    return wave_speed_m_per_s_ * dt_seconds / dx_m_;
}

double String1DSimulation::displacement_m(std::size_t index) const {
    if (index >= current_.size()) {
        throw std::out_of_range("String displacement index is out of range");
    }
    return current_[index];
}

std::size_t String1DSimulation::grid_points() const noexcept {
    return current_.size();
}

double String1DSimulation::total_energy_j(double dt_seconds) const {
    if (dt_seconds <= 0.0) {
        throw std::invalid_argument("Energy requires a positive time step");
    }

    double kinetic_j = 0.0;
    for (std::size_t i = 0; i < current_.size(); ++i) {
        const double velocity = (current_[i] - previous_[i]) / dt_seconds;
        kinetic_j += 0.5 * config_.linear_density_kg_per_m * dx_m_ * velocity * velocity;
    }

    double potential_j = 0.0;
    for (std::size_t i = 0; i + 1 < current_.size(); ++i) {
        const double slope = (current_[i + 1] - current_[i]) / dx_m_;
        potential_j += 0.5 * config_.tension_n * slope * slope * dx_m_;
    }

    return kinetic_j + potential_j;
}

}  // namespace crystal::lab::demo
