#!/usr/bin/env bash

# fail fast
set -euxo pipefail

VERSION=$(cat VERSION)

# check that the repo is clean

if ! git diff-index --quiet HEAD --; then
    echo "Repo is not clean, aborting"
    exit 1
fi

# udpate the html and js source files

pio run -e esp32dev -t webUI

if ! git diff-index --quiet HEAD --; then
    git commit -am "Update the web UI source files for version ${VERSION}"
fi

# udpate the doxygen docs

git rm -rf docs/generated
doxygen
git add docs/generated
git commit -m "Update the Doxygen docs for version ${VERSION}"

