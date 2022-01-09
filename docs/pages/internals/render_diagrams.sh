#!/bin/bash

set -euo pipefail

for n in assets/*.dot; do
    echo "Rendering $n"
    dot -Tsvg "$n" -o "${n%.dot}.svg"
done
