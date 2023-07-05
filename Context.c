#include "Context.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

// ------------------------------------------------

Particle* getParticlePointer(Context* context, int id)
{
	return context->particles + id;
}


Particle getParticle(Context* context, int id)
{
  return context->particles[id];
}

// ------------------------------------------------

void addParticle(Context* context, float x, float y, float speed_x, float speed_y, float radius, float mass, int draw_id)
{
	if (context->num_particles == context->capacity)  // if the capacity is not big enough to add a new particle, we double it and reallocate the needed memory
	{
		Particle* particles = malloc(2 * context->capacity * sizeof(Particle));
		memset(particles, 0, 2 * context->capacity * sizeof(Particle));
		int i;
		for (i=0; i < context->num_particles; i++)
		{
			particles[i] = getParticle(context, i);  // copy the particles to the new array
			
			int* links = malloc(2 * context->capacity * sizeof(int));	// update the links arrays of each particle so it is big enough to store the links data for every other particles
			int j;
		    for (j=0; j < context->capacity; j++)
		    {
		    	links[j] = particles[i].links[j];
			}
			for (j=context->capacity; j < 2 * context->capacity; j++)
			{
				links[j] = 0;
			}
			free(particles[i].links);
			particles[i].links = links;
		}
		free(context->particles);
		context->particles = particles;
		context->capacity *= 2;
	}
	
	// add the new particle	
    context->particles[context->num_particles].position.x = x;
    context->particles[context->num_particles].position.y = y;
    context->particles[context->num_particles].velocity.x = speed_x;
    context->particles[context->num_particles].velocity.y = speed_y;
    context->particles[context->num_particles].inv_mass = 1.0F/mass;
    context->particles[context->num_particles].radius = radius;
    context->particles[context->num_particles].draw_id = draw_id;
    int* links = malloc(context->capacity * sizeof(int));
    int i;
    for (i=0; i < context->capacity; i++)
    {
    	links[i] = 0;
	}
	context->particles[context->num_particles].links = links;
	
    context->num_particles += 1;
}

void addCoupledParticles(Context* context, float x, float y, float speed_x, float speed_y, float radius, float mass, int draw_id1, int draw_id2, int draw_id3, int draw_id4)
{
	addParticle(context, x, y, speed_x, speed_y, radius, mass, draw_id1);
	addParticle(context, x + 2*radius, y, speed_x, speed_y, radius, mass, draw_id2);
	addParticle(context, x, y + 2*radius, speed_x, speed_y, radius, mass, draw_id3);
	addParticle(context, x + 2*radius, y + 2*radius, speed_x, speed_y, radius, mass, draw_id4);

	getParticlePointer(context, context->num_particles - 4)->links[context->num_particles - 3] = 1;
	getParticlePointer(context, context->num_particles - 4)->links[context->num_particles - 2] = 1;
	getParticlePointer(context, context->num_particles - 3)->links[context->num_particles - 4] = 1;
	getParticlePointer(context, context->num_particles - 3)->links[context->num_particles - 1] = 1;
	getParticlePointer(context, context->num_particles - 2)->links[context->num_particles - 4] = 1;
	getParticlePointer(context, context->num_particles - 2)->links[context->num_particles - 1] = 1;
	getParticlePointer(context, context->num_particles - 1)->links[context->num_particles - 3] = 1;
	getParticlePointer(context, context->num_particles - 1)->links[context->num_particles - 2] = 1;
}

// ------------------------------------------------

void setDrawId(Context* context, int sphere_id, int draw_id)
{
  getParticlePointer(context, sphere_id)->draw_id = draw_id;
}

// ------------------------------------------------

SphereCollider getGroundSphereCollider(Context* context, int id)
{
  return context->ground_spheres[id];
}

PlaneCollider getGroundPlaneCollider(Context* context, int id)
{
  return context->ground_planes[id];
}

// ------------------------------------------------

Context* initializeContext()
{
  Context* context = malloc(sizeof(Context));
  context->num_particles = 0;
  context->capacity = 2;
  context->particles = malloc(context->capacity * sizeof(Particle));
  memset(context->particles,0,context->capacity*sizeof(Particle));

  context->water_surface = -2.5f;

  context->num_ground_sphere = 3;
  context->ground_spheres = malloc(3*sizeof(SphereCollider));
  Vec2 p0 = {2.0f, 3.0f};
  context->ground_spheres[0].center = p0;
  context->ground_spheres[0].radius = 1;
  
  Vec2 p1 = {-2.0f, 4.0f};
  context->ground_spheres[1].center = p1;
  context->ground_spheres[1].radius = 0.7;
  
  Vec2 p6 = {-1.0f, 1.0f};
  context->ground_spheres[2].center = p6;
  context->ground_spheres[2].radius = 0.9;
  
  context->num_ground_plane = 4;
  context->ground_planes = malloc(4*sizeof(PlaneCollider));
  Vec2 p2 = {0.0f, -4.0f};
  Vec2 n2 = {0.2f, 1.0f};
  n2 = normalize(n2);
  context->ground_planes[0].center = p2;
  context->ground_planes[0].normal = n2;
  
  Vec2 p3 = {-10.0f, 0.0f};
  Vec2 n3 = {1.0f, 0.0f};
  n3 = normalize(n3);
  context->ground_planes[1].center = p3;
  context->ground_planes[1].normal = n3;
  
  Vec2 p4 = {10.0f, 0.0f};
  Vec2 n4 = {-1.0f, 0.0f};
  n4 = normalize(n4);
  context->ground_planes[2].center = p4;
  context->ground_planes[2].normal = n4;
  
  Vec2 p5 = {0.0f, 10.0f};
  Vec2 n5 = {0.0f, -1.0f};
  n5 = normalize(n5);
  context->ground_planes[3].center = p5;
  context->ground_planes[3].normal = n5;
  return context;
}

// ------------------------------------------------

void updatePhysicalSystem(Context* context, float dt, int num_constraint_relaxation)
{
	applyExternalForce(context, dt);
	updateExpectedPosition(context, dt);

	int k;
	for(k=0; k<num_constraint_relaxation; ++k)
	{
		projectConstraints(context);
	}

	updateVelocityAndPosition(context, dt);
}

// ------------------------------------------------

void applyExternalForce(Context* context, float dt)
{
	int i;
	for (i = 0; i < context->num_particles; i++)
	{
		Particle* particle = getParticlePointer(context, i);
		Vec2 force;
		force.x = 0;
		if (particle->position.y > context->water_surface)
		{
			force.y = -G;
		}
		else
		{
			force.y = -G + particle->inv_mass * A;
		}
		particle->velocity = sum(particle->velocity, mult(force, dt));
	}
}

int checkContactWithSphere(Context* context, int particle_id, int sphere_id)
{
	Particle* particle = getParticlePointer(context, particle_id);
	SphereCollider* sphere = context->ground_spheres + sphere_id;
	if (norm(subs(particle->next_pos, sphere->center)) <= sphere->radius + particle->radius)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

int checkContactWithPlane(Context* context, int particle_id, int plane_id)
{
	Particle* particle = getParticlePointer(context, particle_id);
	PlaneCollider* plane = context->ground_planes + plane_id;
	if (dot(subs(particle->next_pos, plane->center), plane->normal) <= particle->radius)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

int checkContactWithParticle(Context* context, int particle1_id, int particle2_id)
{
	Particle* particle1 = getParticlePointer(context, particle1_id);
	Particle* particle2 = getParticlePointer(context, particle2_id);
	if (norm(subs(particle1->next_pos, particle2->next_pos)) <= particle1->radius + particle2->radius && particle1_id != particle2_id)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

void updateExpectedPosition(Context* context, float dt)
{
	int i;
	for (i = 0; i < context->num_particles; i++)
	{
		Particle* particle = getParticlePointer(context, i);
		particle->next_pos = sum(particle->position, mult(particle->velocity, dt));
	}	
}

void enforceStaticGroundConstraint(Context* context, int particle_id)
{
	Particle* particle = getParticlePointer(context, particle_id);
	int i;
  	for (i=0; i < context->num_ground_sphere; i++)
  	{
  		if (checkContactWithSphere(context, particle_id, i) == 1)
  		{ 			
 			SphereCollider* sphere = context->ground_spheres + i;

			Vec2 v1 = subs(particle->next_pos, sphere->center);
			
			// if the speed's component normal to the surface of the sphere is greater than a certain speed, the particle bounces
			if (fabs((double) dot(particle->velocity, normalize(v1))) >= MINIMUM_SPEED_FOR_BOUNCE_SPHERE)
			{
				particle->next_pos = sum(particle->position, mult(normalize(v1), BOUNCE_FACTOR*dot(particle->velocity, normalize(v1))));
			}
			
			// otherwise, it rolls
			else
			{
				Vec2 v2 = mult(normalize(v1), sphere->radius + particle->radius);
				Vec2 v3 = subs(v2, v1);
				particle->next_pos = sum(particle->next_pos, v3);
			}
		}
	}
  	for (i=0; i < context->num_ground_plane; i++)
  	{
  		if (checkContactWithPlane(context, particle_id, i) == 1)
		{
  			PlaneCollider* plane = context->ground_planes + i;

			Vec2 v1 = subs(particle->next_pos, plane->center);
			
			// if the speed's component normal to the surface of the plane is greater than a certain speed, the particle bounces
			if (fabs((double) dot(particle->velocity, plane->normal)) >= MINIMUM_SPEED_FOR_BOUNCE_PLANE)
			{
				particle->next_pos = subs(particle->position, mult(plane->normal, BOUNCE_FACTOR*dot(particle->velocity, plane->normal)));
			}
			
			// otherwise, it rolls
			else
			{
				Vec2 v2 = mult(plane->normal, dot(v1, plane->normal) - particle->radius);
				particle->next_pos = subs(particle->next_pos, v2);
			}	
		}  		
	}	
}

void enforceDynamicConstraint(Context* context, int particle_id)
{
	Particle* particle = getParticlePointer(context, particle_id);
	int i;
  	for (i=0; i < context->num_particles; i++)
  	{
		if (particle->links[i] == 1)
		{
 			Particle* other = getParticlePointer(context, i);
			Vec2 d = subs(particle->next_pos, other->next_pos);
			float C = norm(d) - particle->radius - other->radius;
			particle->next_pos = subs(particle->next_pos, mult(normalize(d), C * particle->inv_mass / (particle->inv_mass + other->inv_mass)));
		}
		
  		else if (checkContactWithParticle(context, particle_id, i) == 1)
		{ 			
 			Particle* other = getParticlePointer(context, i);
			if (particle != other)
			{
				Vec2 d = subs(particle->next_pos, other->next_pos);
				float C = norm(d) - particle->radius - other->radius;
				particle->next_pos = subs(particle->next_pos, mult(normalize(d), C * particle->inv_mass / (particle->inv_mass + other->inv_mass)));
			}
		}
	}
}

void projectConstraints(Context* context)
{
	int i;
	for (i = 0; i < context->num_particles; i++)
	{
		enforceDynamicConstraint(context, i);
		enforceStaticGroundConstraint(context, i);
	}
}

void updateVelocityAndPosition(Context* context, float dt)
{
	int i;
	for (i = 0; i < context->num_particles; i++)
	{
		Particle* particle = getParticlePointer(context, i);
		particle->velocity = mult(subs(particle->next_pos, particle->position), 1/dt);
		particle->position = particle->next_pos;
	}	
}

void freeAll(Context* context)
{
	int i;
	for (i=0; i < context->num_particles; i++)
	{
		free(getParticlePointer(context, i)->links);
	}
	free(context->particles);
	free(context->ground_spheres);
	free(context->ground_planes);
	free(context);
}
