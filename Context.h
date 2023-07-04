#pragma once

#include "Vec2.h"
#include "Particle.h"
#include "Constraint.h"

#define G 9.81
#define BOUNCE_FACTOR 0.0012
#define MINIMUM_SPEED_FOR_BOUNCE_PLANE 2
#define MINIMUM_SPEED_FOR_BOUNCE_SPHERE 0.5

// ------------------------------------------------

typedef struct Context {
  int num_particles;
  Particle* particles;

  // Ground colliders 
  int num_ground_plane;
  int num_ground_sphere;
  
  PlaneCollider* ground_planes;
  SphereCollider* ground_spheres;
  
  int capacity;
} Context;

// ------------------------------------------------

Context* initializeContext();

// ------------------------------------------------

void addParticle(Context* context, float x, float y, float speed_x, float speed_y, float radius, float mass, int draw_id);
void addCoupledParticles(Context* context, float x, float y, float speed_x, float speed_y, float radius, float mass, int draw_id1, int draw_id2, int draw_id3, int draw_id4);

// ------------------------------------------------

Particle* getParticlePointer(Context* context, int id);

Particle getParticle(Context* context, int id);

SphereCollider getGroundSphereCollider(Context* context, int id);

PlaneCollider getGroundPlaneCollider(Context* context, int id);

// ------------------------------------------------

void setDrawId(Context* context, int sphere_id, int draw_id);

// ------------------------------------------------

void updatePhysicalSystem(Context* context, float dt, int num_constraint_relaxation);

// ------------------------------------------------
// Methods below are called by updatePhysicalSystem
// ------------------------------------------------

void applyExternalForce(Context* context, float dt);
void checkContactWithSphere(Context* context, int particle_id, int sphere_id);
void checkContactWithPlane(Context* context, int particle_id, int plane_id);
void checkContactWithParticle(Context* context, int particle1_id, int particle2_id);
void updateExpectedPosition(Context* context, float dt);
void addDynamicContactConstraints(Context* context);
void addStaticContactConstraints(Context* context);
void enforceStaticGroundConstraint(Context* context, Particle* particle);
void enforceDynamicConstraint(Context* context, Particle* particle);
void projectConstraints(Context* context);
void updateVelocityAndPosition(Context* context, float dt);

// ------------------------------------------------

