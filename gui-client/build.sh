#!/bin/bash

mkdir -p build
pushd build || exit

g++ -I../../extern/imgui \
  -I../../extern/SDA3/include/ \
  -I../../extern/imgui/backends/ \
  -DPLATFORM_LINUX \
  ../main.cpp \
  -L../../extern-lib/ \
  ../../extern-lib/libSDL3.so.0.4.16 \
  -Wl,-rpath,'$ORIGIN' \
  -lm -lpthread -o gui-server

cp ../../extern-lib/libSDL3.so.0.4.16 .
cp -r ../asset .
ln -s libSDL3.so.0.4.16 libSDL3.so.0
ln -s libSDL3.so.0 libSDL3.so
popd || exit
