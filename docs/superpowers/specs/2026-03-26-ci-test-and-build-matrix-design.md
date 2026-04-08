# CI Test and Build Matrix Redesign

**Date:** 2026-03-26
**Status:** Approved

## Problem

The current CI workflow builds 6 examples across 3 platform/device combinations (17 jobs total) but runs no unit tests. With 7 test suites now in the repo, CI should verify they compile. The example matrix is also larger than needed — a representative sample provides the same coverage with fewer jobs.

## Design

### Test Jobs (new)

A new `test` job compiles all test suites (without uploading) for each platform/device combination:

| Platform | Device |
|----------|--------|
| arduino | esp32 |
| pioarduino | esp32 |
| pioarduino | esp32c3 |

Command: `pio test --without-uploading --without-testing -e {platform}_{device}`

Exclusion: arduino + esp32c3 (not supported by the arduino platform).

All 7 test suites must be verified to compile against all 3 environments before merging. If any test suite requires platform-specific features, add `test_ignore` entries in `platformio.ini` for the affected environments.

### Example Build Jobs (reduced)

Three representative examples replace the previous six:

| Example | Why |
|---------|-----|
| `minimal_app.cpp` | MinimalApp path, HTTP server, digital inputs, transforms |
| `rpm_counter.cpp` | Full SensESPApp, DigitalInputCounter, SignalK output, Frequency |
| `ssl_connection.cpp` | SSL/TLS support (pioarduino only) |

Dropped examples and coverage justification:
- `lambda_transform.cpp` — LambdaTransform is exercised by `minimal_app.cpp`; transform correctness is covered by `test_transform_correctness` test suite
- `listener.cpp` — ValueConsumer/listener pattern is exercised by every example via `connect_to()`; also covered by `test_valueproducer` test suite
- `repeat_sensor_analog_input.cpp` — RepeatSensor is a thin wrapper; analog input is hardware-specific and not meaningfully tested by compilation alone

Matrix:

| Example | arduino/esp32 | pioarduino/esp32 | pioarduino/esp32c3 |
|---------|:---:|:---:|:---:|
| minimal_app.cpp | ✓ | ✓ | ✓ |
| rpm_counter.cpp | ✓ | ✓ | ✓ |
| ssl_connection.cpp | — | ✓ | ✓ |

Exclusions: arduino + esp32c3, arduino + ssl_connection.

### Total Jobs

11 (3 test + 8 example build), down from 17.

### File Changes

- **`.github/workflows/ci.yml`** — Add `test` job with its own matrix; reduce `build` job example list to 3. Keep existing triggers (`on: [push]`) unchanged.
- **`ci/run-tests.sh`** (new) — Single command: `pio test --without-uploading -e ${CI_PLATFORM}_${CI_DEVICE}`. Uses `set -euo pipefail` (same as `run-ci.sh`). Runs from project root.
- **`ci/run-ci.sh`** — Unchanged.

### Out of Scope

- Workflow trigger changes (adding `pull_request`, branch filters)
- Cache key improvements (current `${{ runner.os }}-pio` is shared across all jobs)
- Cleaning up unused `platformio.ini` environments (e.g., `arduino_esp32c3`)
