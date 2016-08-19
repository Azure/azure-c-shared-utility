#!/bin/bash
#set -o pipefail
#

set -e

script_dir=$(cd "$(dirname "$0")" && pwd)
build_root=$(cd "${script_dir}/../.." && pwd)
log_dir=$build_root
run_unit_tests=ON
make_install=
run_valgrind=0
build_folder=$build_root"/cmake/shared-util_linux"
skip_unittests=OFF

usage ()
{
    echo "build.sh [options]"
    echo "options"
    echo " -cl, --compileoption <value>  specify a compile option to be passed to gcc"
    echo "   Example: -cl -O1 -cl ..."
    echo "-rv, --run_valgrind will execute ctest with valgrind"
    echo "--skip-unittests do not build or run unit tests"

    exit 1
}

process_args ()
{
    save_next_arg=0
    extracloptions=" "

    for arg in $*
    do
      if [ $save_next_arg == 1 ]
      then
        # save arg to pass to gcc
        extracloptions="$arg $extracloptions"
        save_next_arg=0
      else
          case "$arg" in
              "-cl" | "--compileoption" ) save_next_arg=1;;
              "-i" | "--install" ) make_install=1;;
              "-rv" | "--run_valgrind" ) run_valgrind=1;;
              "--skip-unittests" ) skip_unittests=ON;;
              * ) usage;;
          esac
      fi
    done
}

process_args $*

# Set the default cores
CORES=$(grep -c ^processor /proc/cpuinfo 2>/dev/null || sysctl -n hw.ncpu)

rm -r -f $build_folder
mkdir -p $build_folder
pushd $build_folder
cmake -DcompileOption_C:STRING="$extracloptions" -Drun_valgrind:BOOL=$run_valgrind $build_root -Dskip_unittests:BOOL=$skip_unittests
make --jobs=$CORES
if [[ $make_install == 1 ]] ;
then
    echo "Installing packaging" 
    # install the package
    make install
fi

if [[ $run_valgrind == 1 ]] ;
then
	#use doctored openssl
	export LD_LIBRARY_PATH=/usr/local/ssl/lib
	ctest -j $CORES --output-on-failure
	export LD_LIBRARY_PATH=
else
	ctest -j $CORES -C "Debug" --output-on-failure
fi

popd
