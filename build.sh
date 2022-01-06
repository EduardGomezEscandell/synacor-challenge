#!/bin/bash

# Usage:
# $ bash build.sh

export PROJECT_DIR=$(pwd)
export BUILD_TYPE=${BUILD_TYPE:-Release}

echo "Build type: $BUILD_TYPE"
echo

export BUILD_DIR=$PROJECT_DIR/build/
export SOURCE_DIR=$PROJECT_DIR


mkdir $BUILD_DIR 2> /dev/null
cd $BUILD_DIR

mkdir $BUILD_TYPE 2> /dev/null
cd $BUILD_TYPE

cmake $SOURCE_DIR                               \
-DCMAKE_BUILD_TYPE=$BUILD_TYPE                  \
-DSANITIZER=${SANITIZER}

cmake --build . -- -j $(nproc)

cd $PROJECT_DIR
mv $BUILD_DIR/$BUILD_TYPE/compile_commands.json .

mkdir "bin"               2> /dev/null
mkdir "bin/${BUILD_TYPE}" 2> /dev/null


# Linking VM
export EXECUTABLE="${PROJECT_DIR}/bin/${BUILD_TYPE}/vm"
rm "${EXECUTABLE}"        2> /dev/null
ln -s "${PROJECT_DIR}/build/${BUILD_TYPE}/src/synacor_vm" "${EXECUTABLE}"

echo
echo "Created binary ${EXECUTABLE}"

# Linking tests
export EXECUTABLE="${PROJECT_DIR}/bin/${BUILD_TYPE}/run_tests"

rm "${EXECUTABLE}"        2> /dev/null
ln -s "${PROJECT_DIR}/build/${BUILD_TYPE}/test/run_tests" "${EXECUTABLE}"

echo "Created binary ${EXECUTABLE}"

# Linking Assembler
export EXECUTABLE="${PROJECT_DIR}/bin/${BUILD_TYPE}/assemble"

rm "${EXECUTABLE}"        2> /dev/null
ln -s "${PROJECT_DIR}/build/${BUILD_TYPE}/assembler/assembler" "${EXECUTABLE}"

echo "Created binary ${EXECUTABLE}"
echo