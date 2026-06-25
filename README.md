# Crystal Sound Lab — minimal C++ core

This directory is a deliberately small replacement starting point for future
physics demos. It does **not** try to be a general game engine.

Its only stable promises are:

1. a simulation owns and advances its own physical state;
2. a deterministic fixed-step runner advances that simulation on the caller's thread;
3. experiments can write plain CSV output for external validation and plotting.

There is deliberately no event bus, global singleton, render system, entity
system, background engine thread, or platform abstraction here. Add one only
when at least two independent demos have demonstrated the same need.

## Build

Requirements: a C++20 compiler and CMake 3.25 or newer.

```bash
cmake -S Lab -B build/lab -DCMAKE_BUILD_TYPE=Debug
cmake --build build/lab
ctest --test-dir build/lab --output-on-failure
```

On Visual Studio, use a multi-config build directory instead:

```powershell
cmake -S Lab -B build/lab
cmake --build build/lab --config Debug
ctest --test-dir build/lab -C Debug --output-on-failure
```

## First experiment: `string_1d`

The demo solves the fixed-end taut-string equation

```text
u_tt = c^2 u_xx,     c = sqrt(T / mu)
```

using a centered explicit finite-difference scheme. It starts from a triangular
pluck with zero initial velocity and writes:

- `string_1d_trace.csv`: time, midpoint displacement, total energy, and CFL number;
- `string_1d_final_profile.csv`: the final displacement profile.

Run it from the build directory:

```bash
./demos/string_1d/string_1d --steps 120000 --observe-every 100
```

The runner selects `dt = 0.90 dx / c`, so the one-dimensional CFL number is
0.90. Change the string parameters in `String1DConfig`, compare the output with
normal-mode theory, and add damping only after first verifying the undamped
energy behavior.

## Recommended increment path

1. Keep `core/` small and stable.
2. Treat every folder under `demos/` as an independent physical experiment.
3. Create a Python or notebook reference solution before optimizing a C++ demo.
4. Add a core abstraction only after two independent demos need it.
5. Keep equations, assumptions, units, numerical scheme, and validation notes
   next to each demo.

See [`docs/minimal_core.md`](docs/minimal_core.md) for the extension rules.
