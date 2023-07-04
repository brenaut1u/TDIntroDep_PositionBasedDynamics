#ifndef CONSTRAINT_H_
#define CONSTRAINT_H_

// ------------------------------------------------

typedef struct PlaneCollider {
  Vec2 center;
  Vec2 normal;
} PlaneCollider;

typedef struct SphereCollider {
  Vec2 center;
  float radius;
} SphereCollider;

// ------------------------------------------------

#endif
