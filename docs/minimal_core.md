# Minimal-core design rules

## What belongs in `core/`

Only mechanisms that are independent of a particular physical model:

- `Simulation`: reset + fixed-step evolution contract;
- `FixedStepRunner`: deterministic, single-threaded scheduling;
- `CsvWriter`: simple reproducible output.

The core intentionally has no dependency on a window system, audio API, JUCE,
OpenGL, a logger, a thread pool, or an event bus.

## What belongs in a demo

Everything that expresses the physics:

- state variables and units;
- governing equations;
- boundary and initial conditions;
- discretization;
- stability checks;
- observables and conservation laws;
- output schema;
- a validation case.

## Before adding an abstraction

Write down the two concrete demos that require it. If there is only one, keep
its implementation local to that demo. Typical examples:

- add a parameter snapshot object after two demos need live parameter changes;
- add a renderer after two demos need the same rendering primitive;
- add a producer/consumer queue only when a real device callback must communicate
  with a simulation thread;
- add an audio layer only when a demo has a defined sonic observable and needs
  real-time playback.

## First demo roadmap

1. Verify the undamped `string_1d` demo against normal modes and its numerical
   CFL limit.
2. Add an observation-point FFT and compare it with the expected mode spectrum.
3. Add a bridge/pickup observation model: do not assume displacement at one
   point is identical to radiated sound.
4. Only then decide whether a JUCE audio front end is needed.

This sequence is directly useful for the "singing ruler", guitar string, and
solid-to-air coupling questions.
