#!/usr/bin/env bash

# CMake
wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | sudo apt-key add -
sudo apt-add-repository "deb https://apt.kitware.com/ubuntu/ bionic main"
sudo apt-get update
sudo apt-get install -y cmake

sudo apt install -y build-essential
sudo apt-get install libboost-all-dev

wget https://dl.bintray.com/boostorg/release/1.74.0/source/boost_1_74_0.tar.bz2
tar xvf boost_1_74_0.tar.bz2
export BOOST_ROOT=/home/foijord/boost_1_74_0

cd Vulkan/Server
mkdir build
cd build
cmake ..
make
