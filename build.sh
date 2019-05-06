#!/bin/bash

#load latest gcc version
#module load gcc/8.2.0

cd ..

workingDir=$PWD

#build latest boost
wget https://dl.bintray.com/boostorg/release/1.69.0/source/boost_1_69_0.tar.gz
tar xvzf boost_1_69_0.tar.gz
cd boost_1_69_0
./bootstrap.sh
./b2
cd ..

#build latest cmake
wget https://github.com/Kitware/CMake/releases/download/v3.14.1/cmake-3.14.1.tar.gz
tar xvzf cmake-3.14.1.tar.gz
cd cmake-3.14.1
./configure
make
cd ..

#build gabac
cd gabac
git checkout experimental
cd ..
mkdir gabac-build
cd gabac-build
../cmake-3.14.1/bin/cmake -D CMAKE_C_COMPILER=gcc -D CMAKE_BUILD_TYPE=Release -D BOOST_ROOT=$workingDir/boost_1_69_0/ ../gabac
make gabacify
cd ..
