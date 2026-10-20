#!/bin/bash

pushd ~/code/ros2_ws/ || return 1

colcon build --packages-select gui_server
source install/setup.bash

popd || return 1
