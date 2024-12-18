#!/bin/bash

set -euo pipefail

# assume this command is always run from the project root directory

export PROJDIR=$(pwd)

# the example to build comes from $PLATFORMIO_CI_SRC

pio ci -e ${CI_PLATFORM}_${CI_DEVICE} --project-conf platformio.ini --lib . --exclude examples --exclude docs --exclude .pio
