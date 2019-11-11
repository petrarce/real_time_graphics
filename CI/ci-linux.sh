#!/bin/bash

COMPILER=$1
LANGUAGE=$2

# Exit script on any error
set -e 

OPTIONS=""
MAKE_OPTIONS=""
BUILDPATH=""

if [ "$COMPILER" == "gcc" ]; then
  echo "Building with GCC";
  BUILDPATH="gcc"

  # without icecc: no options required
  OPTIONS="$OPTIONS -DCMAKE_CXX_COMPILER=/usr/lib/icecc/bin/g++ -DCMAKE_C_COMPILER=/usr/lib/icecc/bin/gcc"
  MAKE_OPTIONS="-j16"
  export ICECC_CXX=/usr/bin/g++ ; export ICECC_CC=/usr/bin/gcc

elif [ "$COMPILER" == "clang" ]; then

  OPTIONS="$OPTIONS -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_C_COMPILER=clang"
  echo "Building with CLANG";
  BUILDPATH="clang"  
  MAKE_OPTIONS="-j8"
fi

#=====================================
# Color Settings:
#=====================================
NC='\033[0m'
OUTPUT='\033[0;32m'
WARNING='\033[0;93m'


echo -e "${OUTPUT}"
echo ""
echo "======================================================================"
echo "Basic configuration details:"
echo "======================================================================"
echo -e "${NC}"

echo "Compiler:     $COMPILER"
echo "Options:      $OPTIONS"
echo "Language:     $LANGUAGE"
echo "Make Options: $OPTIONS"
echo "BuildPath:    $BUILDPATH"
echo "Path:         $PATH"
echo "Language:     $LANGUAGE"

echo -e "${OUTPUT}"
echo ""
echo "========================="
echo "Building Release versions"
echo "========================="
echo -e "${NC}"


# Create build dir if it does not exist
if [ ! -d build-release-$BUILDPATH ]; then
  mkdir build-release-$BUILDPATH
fi


# This is the file that indicates a "buildable" assignment
fileCheck="CMakeLists.txt"

# Iterate over all folders starting with "assignment"
for ass in assignment*
do
    # Skip theoretical assignments
    if [ ! -f $ass/$fileCheck ]; then
        echo "Skipping $ass as it does not contain a $fileCheck file (probably a theoretical sheet)"
        continue
    fi

    cd build-release-$BUILDPATH
    if [ ! -d ${ass}-build ]; then
        mkdir ${ass}-build
    fi
    cd ${ass}-build
    echo "Call cmake and make for $ass"
    #pwd
    cmake -DCMAKE_BUILD_TYPE=Release $OPTIONS ../../$ass
    make $MAKE_OPTIONS
    cd ..
    cd ..

done


# back to root
cd ..
