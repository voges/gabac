#!/usr/bin/env bash

# Get the Git root directory
readonly git_root_dir="$(git rev-parse --show-toplevel)"

# Run C/C++ unit tests
"$git_root_dir/build/tests" || exit -1

# Get the correct shared library extension depending whether we are on
# Linux (*.so) or macOS (*.dylib)
dl_extension=""
if [[ $(uname -s) == Linux* ]]; then
    dl_extension="so"
elif [[ $(uname -s) == Darwin* ]]; then
    dl_extension="dylib"
else
    echo "Not a Linux or macOS build; skipping Python unit tests"
    exit 0
fi

# Go to the folder of gabac.py and fire up the unit tests
cd "$git_root_dir/source/python_api/" || exit -1
LIBGABAC_PATH="$git_root_dir/build/libgabac.$dl_extension" python -m unittest discover -v "$git_root_dir/tests/python_api/"
