#!/bin/bash

set -euo pipefail

# assume this command is always run from the project root directory

PROJDIR=$(pwd)

# replace the 
sed -e "s|\${PROJDIR}|$PROJDIR|" ci/platformio-${CI_TARGET_DEVICE}.ini > ci/platformio.ini

# the example to build comes from $PLATFORMIO_CI_SRC

pio ci -c ci/platformio.ini
