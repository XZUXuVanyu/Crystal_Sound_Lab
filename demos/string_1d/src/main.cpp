#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string_view>

#include "crystal_lab/core/CsvWriter.hpp"
#include "crystal_lab/core/FixedStepRunner.hpp"
#include "string_1d/String1DSimulation.hpp"

namespace {

struct Arguments {
    std::size_t steps = 120000;
    std::size_t observe_every = 100;
    std::filesystem::path output = "string_1d_trace.csv";
    std::filesystem::path profile_output = "string_1d_final_profile.csv";
};

[[nodiscard]] std::size_t parse_size(std::string_view value, std::string_view option) {
    try {
        const auto parsed = std::stoull(std::string(value));
        if (parsed == 0) {
            throw std::invalid_argument("zero");
        }
        return static_cast<std::size_t>(parsed);
    } catch (...) {
        throw std::invalid_argument("Invalid positive integer for " + std::string(option));
    }
}

[[nodiscard]] Arguments parse_arguments(int argc, char** argv) {
    Arguments result;

    for (int i = 1; i < argc; ++i) {
        const std::string_view arg = argv[i];
        if (arg == "--help") {
            std::cout
                << "Usage: string_1d [--steps N] [--observe-every N] [--output FILE] [--profile-output FILE]\n";
            std::exit(EXIT_SUCCESS);
        }
        if (i + 1 >= argc) {
            throw std::invalid_argument("Missing value after " + std::string(arg));
        }

        const std::string_view value = argv[++i];
        if (arg == "--steps") {
            result.steps = parse_size(value, arg);
        } else if (arg == "--observe-every") {
            result.observe_every = parse_size(value, arg);
        } else if (arg == "--output") {
            result.output = value;
        } else if (arg == "--profile-output") {
            result.profile_output = value;
        } else {
            throw std::invalid_argument("Unknown option: " + std::string(arg));
        }
    }

    return result;
}

}  // namespace

int main(int argc, char** argv) {
    try {
        const Arguments args = parse_arguments(argc, argv);

        crystal::lab::demo::String1DSimulation simulation;
        const double dt_seconds = 0.90 * simulation.dx_m() / simulation.wave_speed_m_per_s();

        crystal::lab::CsvWriter trace(args.output);
        trace.header("time_s,midpoint_displacement_m,total_energy_j,cfl");

        const auto state = crystal::lab::run_fixed_steps(
            simulation,
            {
                .dt_seconds = dt_seconds,
                .max_steps = args.steps,
                .observe_every = args.observe_every,
            },
            [&trace, dt_seconds](const crystal::lab::RunState& run_state, const crystal::lab::Simulation& base) {
                const auto& string = static_cast<const crystal::lab::demo::String1DSimulation&>(base);
                const std::size_t midpoint = string.grid_points() / 2;
                trace.row({
                    run_state.time_seconds,
                    string.displacement_m(midpoint),
                    string.total_energy_j(dt_seconds),
                    string.cfl_number(dt_seconds),
                });
            });

        crystal::lab::CsvWriter profile(args.profile_output);
        profile.header("x_m,displacement_m");
        for (std::size_t i = 0; i < simulation.grid_points(); ++i) {
            profile.row({static_cast<double>(i) * simulation.dx_m(), simulation.displacement_m(i)});
        }

        std::cout << "Completed " << simulation.name() << "\n"
                  << "  simulated time: " << state.time_seconds << " s\n"
                  << "  wave speed:     " << simulation.wave_speed_m_per_s() << " m/s\n"
                  << "  CFL number:     " << simulation.cfl_number(dt_seconds) << "\n"
                  << "  trace:          " << args.output << "\n"
                  << "  final profile:  " << args.profile_output << "\n";
        return EXIT_SUCCESS;
    } catch (const std::exception& exception) {
        std::cerr << "string_1d failed: " << exception.what() << '\n';
        return EXIT_FAILURE;
    }
}
