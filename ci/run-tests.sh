#!/bin/bash

set -euo pipefail

# Compile all test suites for the specified platform/device without uploading.

pio test --without-uploading --without-testing -e "${CI_PLATFORM}_${CI_DEVICE}"
