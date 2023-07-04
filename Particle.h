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
  unsigned long long int status; // a bitfield with this format:
  				   //   0100010000 01001000 01000001
  				   //   particles | planes | spheres
  				   // where a bit 1 means there is a contact with the particle/plane/sphere that has the corresponding index
  unsigned long long int links;  // a bitfield where 1 means there is a link with the particle that has the corresponding index
  struct Particle* next; // used for the linked list 
} Particle;

// ------------------------------------------------

