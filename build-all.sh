#!/bin/bash

./bootstrap.sh || exit 1
# export CFLAGS="-Wall -Wpedantic -Wextra -g -O0 -fprofile-arcs -ftest-coverage"
# export CXXFLAGS="-Wall -Wpedantic -Wextra -g -O0 -fprofile-arcs -ftest-coverage"
# export LDFLAGS="-fprofile-arcs"
# ./configure --disable-remote --disable-java  --disable-simd --disable-encryption --enable-debug || exit 1

export CFLAGS="-g -O0 -fprofile-arcs -ftest-coverage"
export CXXFLAGS="-g -O0 -fprofile-arcs -ftest-coverage"
export LDFLAGS="-fprofile-arcs"

./configure \
    --disable-remote \
    --disable-java  \
    --disable-simd \
    --disable-encryption \
    --enable-dependency-tracking || exit 1
    
make -j5 || exit 1
cd unittests && make plugin || exit 1
cd ..
# make test || exit 1
make test



#lcov --directory . --capture --output-file ./code_coverage.info -rc lcov_branch_coverage=1

#genhtml code_coverage.info --branch-coverage --output-directory ./code_coverage_report/

