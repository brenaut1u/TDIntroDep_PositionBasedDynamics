#ifndef VEC2_H_
#define VEC2_H_

// ------------------------------------------------

typedef struct Vec2 {
  float x;
  float y;
} Vec2;

// ------------------------------------------------

Vec2 mult(Vec2 vect, float a);
Vec2 sum(Vec2 vect1, Vec2 vect2);
Vec2 subs(Vec2 vect1, Vec2 vect2);
float dot(Vec2 vect1, Vec2 vect2);
float norm(Vec2 vect);
Vec2 normalize(Vec2 vect);

// ------------------------------------------------

#endif
