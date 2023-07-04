#include "Vec2.h"
#include <math.h>

// ------------------------------------------------
Vec2 mult(Vec2 vect, float a)
{
	Vec2 new_vect;
	new_vect.x = a * vect.x;
	new_vect.y = a * vect.y;
	return new_vect;
}

Vec2 sum(Vec2 vect1, Vec2 vect2)
{
	Vec2 new_vect;
	new_vect.x = vect1.x + vect2.x;
	new_vect.y = vect1.y + vect2.y;
	return new_vect;	
}

Vec2 subs(Vec2 vect1, Vec2 vect2)
{
	Vec2 new_vect;
	new_vect.x = vect1.x - vect2.x;
	new_vect.y = vect1.y - vect2.y;
	return new_vect;	
}

float dot(Vec2 vect1, Vec2 vect2)
{
	return vect1.x * vect2.x + vect1.y * vect2.y;
}

float norm(Vec2 vect)
{
	return sqrt(vect.x * vect.x + vect.y * vect.y);
}

Vec2 normalize(Vec2 vect)
{
	return mult(vect, 1 / norm(vect));
}
// ------------------------------------------------
