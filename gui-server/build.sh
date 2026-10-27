#!/bin/bash

pushd ~/code/ros2_ws/ || return 1

colcon build --packages-select gui_server --cmake-args -DMAKE_BUILD_TYPE=Debug
source install/setup.bash

popd || return 1
