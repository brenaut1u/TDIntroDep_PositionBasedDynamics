#pragma once

#include "Vec2.h"
#include <stdint.h>

// ------------------------------------------------

typedef struct Particle {
	Vec2 position;   // current position of the particle
	Vec2 next_pos;   // only used during update loop
	Vec2 velocity;   // current velocity of the particle
	float inv_mass;  // 1/mass of the particle
	float radius;    // radius of the particle
	int solid_id;    // all particles associated to the same solid should have the same id
	int draw_id;     // id used to identify drawing element associated to the sphere
	int* links;
} Particle;

// ------------------------------------------------

