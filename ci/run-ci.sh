#!/bin/bash

set -euo pipefail

# assume this command is always run from the project root directory

export PROJDIR=$(pwd)

# the example to build comes from $PLATFORMIO_CI_SRC

pio ci -c ci/platformio_${CI_PLATFORM}.ini
