#!/bin/bash

mkdir -p build
pushd build || exit

g++ -I../../extern/imgui \
  -I../../extern/SDA3/include/ \
  -I../../extern/imgui/backends/ \
  -I ../../extern/implot/ \
  -DPLATFORM_LINUX \
  ../main.cpp \
  -L../../extern-lib/ \
  ../../extern-lib/libSDL3.so.0.4.16 \
  -Wl,-rpath,'$ORIGIN' \
  -g3 -O0 -fno-omit-frame-pointer \
  -lm -lpthread -o gui-client

cp ../../extern-lib/libSDL3.so.0.4.16 .
cp -r ../asset .
ln -s libSDL3.so.0.4.16 libSDL3.so.0
ln -s libSDL3.so.0 libSDL3.so
popd || exit
