#!/bin/sh

set -e

if [ ! -d "build" ]; then
    mkdir build
    cd build
    cmake .. -DENABLE_COVERAGE=ON
    cmake .. -DENABLE_COVERAGE=ON
    cd ..
fi

cd build
#find -name "*.gcno" -exec rm '{}' \;
#rm -f cov.info
#rm -rf html
make -j4
make test
lcov -c -d . -o cov.info
genhtml -o html cov.info

