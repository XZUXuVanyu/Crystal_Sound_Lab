#pragma once

#include <filesystem>
#include <fstream>
#include <initializer_list>
#include <iomanip>
#include <stdexcept>
#include <string_view>

namespace crystal::lab {

/// Minimal, dependency-free output for reproducible numerical experiments.
class CsvWriter {
public:
    explicit CsvWriter(const std::filesystem::path& path)
        : stream_(path) {
        if (!stream_) {
            throw std::runtime_error("Could not open CSV output: " + path.string());
        }
        stream_ << std::setprecision(17);
    }

    void header(std::string_view text) {
        stream_ << text << '\n';
    }

    void row(std::initializer_list<double> values) {
        bool first = true;
        for (const double value : values) {
            if (!first) {
                stream_ << ',';
            }
            stream_ << value;
            first = false;
        }
        stream_ << '\n';
    }

private:
    std::ofstream stream_;
};

}  // namespace crystal::lab
