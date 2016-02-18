#!/bin/bash
#set -o pipefail
#

set -e

build_folder=~/shared-util
script_dir=$(cd "$(dirname "$0")" && pwd)
build_root=$(cd "${script_dir}/../.." && pwd)
log_dir=$build_root
run_unit_tests=ON
make_install=

usage ()
{
    echo "build.sh [options]"
    echo "options"
    echo " -cl, --compileoption <value>  specify a compile option to be passed to gcc"
    echo "   Example: -cl -O1 -cl ..."

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
              * ) usage;;
          esac
      fi
    done
}

process_args $*

rm -r -f $build_folder
mkdir $build_folder
pushd $build_folder
cmake -DcompileOption_C:STRING="$extracloptions" $build_root
make --jobs=$(nproc)
if [ $make_install == 1 ]
then
    echo "Installing packaging" 
    # install the package
    make install
fi

#run tests quickly - if they fail set -e will take care of not running the tests under valgrind
ctest -C "Debug" -V

#run the tests under valgrind: SLOW (like 20 times slower)
set +e
echo "running VALGRIND... patience... (no output is expected)"
ctest -j $(nproc) -D ExperimentalMemCheck | grep -E '^Memory Leak|^Mismatched deallocation|^Uninitialized Memory Conditional|^Uninitialized Memory Read'
grepReturnCode=$?
#grep return "1" if it does not find any text...
if [[ $grepReturnCode -ne 1 ]];  then
	echo "VALGRIND DETECTED MEMORY LEAKS. See $(pwd)/Testing folder"; popd; exit 1;
else
	echo "VALGRIND did not detect anything interesting";
fi
set -e
popd
