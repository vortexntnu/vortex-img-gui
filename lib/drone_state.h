#ifndef drone_state_h
#define drone_state_h
#include "typedef.h"

struct thruster {
	f32 Speed;
};

struct v3 {
	f32 X;
	f32 Y;
	f32 Z;
};

struct drone_state {
	thruster Thrusters[6];
	f32 Depth;
	v3 Speed;
	v3 Pos;
};


#endif