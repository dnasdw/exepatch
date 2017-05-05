#!/bin/bash

function FILENOTFOUND {
    echo "$1 not found"
}

if [ "$1" == "" ] || [ "$2" == "" ]; then
    echo "usage $0 ico_path patch_path name_prefix"
    exit
fi
if [ ! -f "$1" ]; then
    FILENOTFOUND "$1"
    exit
fi

cwdir_exepatch=`pwd`
rootdir_exepatch=`dirname "$0"`
cd "$rootdir_exepatch"
rootdir_exepatch=`pwd`
mkdir -p "$rootdir_exepatch/project"
cd "$rootdir_exepatch/project"
cmake ..
cmake --build . --target install --config MinSizeRel --clean-first
cd "$cwdir_exepatch"
cat "$rootdir_exepatch/bin/exepatch" "$1" > "$2"
chmod +x "$2"
rm -rf "$rootdir_exepatch/bin"
rm -rf "$rootdir_exepatch/project"
