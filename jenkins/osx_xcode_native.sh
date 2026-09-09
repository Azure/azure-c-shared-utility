#!/bin/bash
# Copyright (c) Microsoft. All rights reserved.
# Licensed under the MIT license. See LICENSE file in the project root for full license information.
#

set -e

script_dir=$(cd "$(dirname "$0")" && pwd)
build_root=$(cd "${script_dir}/.." && pwd)
build_folder=$build_root"/cmake"

CORES=$(grep -c ^processor /proc/cpuinfo 2>/dev/null || sysctl -n hw.ncpu)

rm -r -f $build_folder
mkdir -p $build_folder
pushd $build_folder
# This is the only leg that builds the Apple TLS adapter (use_applessl is selected when
# openssl is not requested), so the integration tests are enabled here to give that adapter
# runtime coverage rather than compile-only coverage.
cmake .. -Drun_unittests:bool=ON -Drun_int_tests:bool=ON -G Xcode
cmake --build . -- --jobs=$CORES
ctest -C "debug" -V
popd
