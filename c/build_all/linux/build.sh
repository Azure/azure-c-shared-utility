#!/bin/bash
#set -o pipefail
#

set -e

build_clean=
script_dir=$(cd "$(dirname "$0")" && pwd)
build_root=$(cd "${script_dir}/../.." && pwd)
log_dir=$build_root
run_unit_tests=ON

usage ()
{
    echo "build.sh [options]"
    echo "options"
    echo " -x,  --xtrace                 print a trace of each command"
    echo " -c,  --clean                  remove artifacts from previous build before building"
    echo " -cl, --compileoption <value>  specify a compile option to be passed to gcc"
    echo "   Example: -cl -O1 -cl ..."
    exit 1
}

process_args ()
{
    build_clean=0
    save_next_arg=0
    extracloptions=" "

    for arg in $*
    do
      if [ $save_next_arg == 1 ]
      then
        # save arg to pass to gcc
        extracloptions="$extracloptions $arg"
        save_next_arg=0
      else
          case "$arg" in
              "-x" | "--xtrace" ) set -x;;
              "-c" | "--clean" ) build_clean=1;;
              "-cl" | "--compileoption" ) save_next_arg=1;;
              * ) usage;;
          esac
      fi
    done
}

process_args $*

rm -r -f ~/cmake
mkdir ~/cmake
pushd ~/cmake
cmake $build_root
make --jobs=$(nproc)
ctest -C "Debug" -V
popd