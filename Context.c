#include "Context.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

// ------------------------------------------------

Particle* getParticlePointer(Context* context, int id)
{
// ### Linked list version
//
//	//The last particle of the linked list is actually the one of index 0
//	assert(id < context->num_particles);
//	int i = 1;
//	Particle* pt = context->particles;
//	while (context->num_particles - i > id)
//	{
//		pt = pt->next;
//		i++;
//	}
//	return pt;

//----------------------------------------------------------

	return context->particles + id;
}


Particle getParticle(Context* context, int id)
{
  //return *getParticlePointer(context, id);
  
  //printf("%f %f\n", context->particles[id].position.x, context->particles[id].position.y);
  return context->particles[id];
}

// ------------------------------------------------

void addParticle(Context* context, float x, float y, float speed_x, float speed_y, float radius, float mass, int draw_id)
{
//	  Particle* new_particle = malloc(sizeof(Particle));
//    new_particle->position.x = x;
//    new_particle->position.y = y;
//    new_particle->velocity.x = speed_x;
//    new_particle->velocity.y = speed_y;
//    new_particle->inv_mass = 1.0F/mass;
//    new_particle->radius = radius;
//    new_particle->draw_id = draw_id;
//    new_particle->status = 0;
//    new_particle->links = 0;
//    new_particle->next = context->particles;
//	  context->particles = new_particle;

	if (context->num_particles == context->capacity)
	{
		context->capacity *= 2;
		Particle* particles = malloc(context->capacity * sizeof(Particle));
		memset(particles,0,context->capacity*sizeof(Particle));
		int i;
		for (i=0; i < context->num_particles; i++)
		{
			particles[i] = getParticle(context, i);
		}
		context->particles = particles;
	}
	
    context->particles[context->num_particles].position.x = x;
    context->particles[context->num_particles].position.y = y;
    context->particles[context->num_particles].velocity.x = speed_x;
    context->particles[context->num_particles].velocity.y = speed_y;
    context->particles[context->num_particles].inv_mass = 1.0F/mass;
    context->particles[context->num_particles].radius = radius;
    context->particles[context->num_particles].draw_id = draw_id;
    context->particles[context->num_particles].status = 0;
    context->particles[context->num_particles].links = 0;
    context->num_particles += 1;
}

void addCoupledParticles(Context* context, float x, float y, float speed_x, float speed_y, float radius, float mass, int draw_id1, int draw_id2, int draw_id3, int draw_id4)
{
	addParticle(context, x, y, speed_x, speed_y, radius, mass, draw_id1);
	addParticle(context, x + 2*radius, y, speed_x, speed_y, radius, mass, draw_id2);
	addParticle(context, x, y + 2*radius, speed_x, speed_y, radius, mass, draw_id3);
	addParticle(context, x + 2*radius, y + 2*radius, speed_x, speed_y, radius, mass, draw_id4);
	getParticlePointer(context, context->num_particles - 4)->links += 1 << (context->num_particles - 3);
	getParticlePointer(context, context->num_particles - 4)->links += 1 << (context->num_particles - 2);
	getParticlePointer(context, context->num_particles - 3)->links += 1 << (context->num_particles - 4);
	getParticlePointer(context, context->num_particles - 3)->links += 1 << (context->num_particles - 1);
	getParticlePointer(context, context->num_particles - 2)->links += 1 << (context->num_particles - 4);
	getParticlePointer(context, context->num_particles - 2)->links += 1 << (context->num_particles - 1);
	getParticlePointer(context, context->num_particles - 1)->links += 1 << (context->num_particles - 3);
	getParticlePointer(context, context->num_particles - 1)->links += 1 << (context->num_particles - 2);
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
  context->particles = malloc(context->capacity * sizeof(Particle)); //NULL;
  memset(context->particles,0,context->capacity*sizeof(Particle));

  context->num_ground_sphere = 2;
  context->ground_spheres = malloc(2*sizeof(SphereCollider));
  Vec2 p0 = {0.0f, 3.0f};
  context->ground_spheres[0].center = p0;
  context->ground_spheres[0].radius = 1;
  
  Vec2 p1 = {1.0f, -1.0f};
  context->ground_spheres[1].center = p1;
  context->ground_spheres[1].radius = 0.7;
  
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

	addDynamicContactConstraints(context);
	addStaticContactConstraints(context);

	int k;
	for(k=0; k<num_constraint_relaxation; ++k)
	{
		projectConstraints(context);
	}

	//updateExpectedPosition(context, dt);

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
		force.y = -G;
		particle->velocity = sum(particle->velocity, mult(force, dt));
	}
}

void checkContactWithSphere(Context* context, int particle_id, int sphere_id)
{
	Particle* particle = getParticlePointer(context, particle_id);
	SphereCollider* sphere = context->ground_spheres + sphere_id;
	if (norm(subs(particle->next_pos, sphere->center)) <= sphere->radius + particle->radius)
	{
		particle->status = particle->status | (1 << sphere_id);
	}
	else
	{
		if (((particle->status >> sphere_id) & 1) == 1)
		{
			particle->status -= 1 << sphere_id;
		}
	}
}

void checkContactWithPlane(Context* context, int particle_id, int plane_id)
{
	Particle* particle = getParticlePointer(context, particle_id);
	PlaneCollider* plane = context->ground_planes + plane_id;
	if (dot(subs(particle->next_pos, plane->center), plane->normal) <= particle->radius)
	{
		particle->status = particle->status | (1 << (context->num_ground_sphere + plane_id));
	}
	else
	{
		if (((particle->status >> (context->num_ground_sphere + plane_id)) & 1) == 1)
		{
			particle->status -= 1 << (context->num_ground_sphere + plane_id);
		}
	}
}

void checkContactWithParticle(Context* context, int particle1_id, int particle2_id)
{
	Particle* particle1 = getParticlePointer(context, particle1_id);
	Particle* particle2 = getParticlePointer(context, particle2_id);
	if (norm(subs(particle1->next_pos, particle2->next_pos)) <= particle1->radius + particle2->radius && particle1_id != particle2_id)
	{
		particle1->status = particle1->status | (1 << (context->num_ground_sphere + context->num_ground_plane + particle2_id));
		particle2->status = particle2->status | (1 << (context->num_ground_sphere + context->num_ground_plane + particle1_id));
	}
	else
	{
		if (((particle1->status >> (context->num_ground_sphere + context->num_ground_plane + particle2_id)) & 1) == 1) // if there is currently no contact but there was one previously
		{
			particle1->status -= 1 << (context->num_ground_sphere + context->num_ground_plane + particle2_id); // update the status so as to mark no contact
		}
		if (((particle2->status >> (context->num_ground_sphere + context->num_ground_plane + particle1_id)) & 1) == 1) // if there is currently no contact but there was one previously
		{
			particle2->status -= 1 << (context->num_ground_sphere + context->num_ground_plane + particle1_id); // update the status so as to mark no contact
		}
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

void addDynamicContactConstraints(Context* context)
{
  int i, j;
  for (i=0; i < context->num_particles; i++)
  {
  	for (j=0; j < context->num_particles; j++)
  	{
  		checkContactWithParticle(context, i, j);
	}	
  }
}

void addStaticContactConstraints(Context* context)
{
  int i, j;
  for (i=0; i < context->num_particles; i++)
  {
  	for (j=0; j < context->num_ground_sphere; j++)
  	{
  		checkContactWithSphere(context, i, j);
	}
  	for (j=0; j < context->num_ground_plane; j++)
  	{
  		checkContactWithPlane(context, i, j);
	}	
  }
}

void enforceStaticGroundConstraint(Context* context, Particle* particle)
{
	int i;
  	for (i=0; i < context->num_ground_sphere; i++)
  	{
  		if (((particle->status >> i) & 1) == 1)  // if there is a contact with the sphere i
  		{ 			
 			SphereCollider* sphere = context->ground_spheres + i;
 			
		//---------------------------------------------------------------------------------------
		
		// ### First version, using speed
		 			
		//  Vec2 v1 = normalize(subs(particle->next_pos, sphere->center));
		//  Vec2 v2 = mult(v1, dot(particle->velocity, v1));
		//  
		//  if (dot(particle->velocity, v1) < 0)
		//  {
		//  	particle->velocity = subs(particle->velocity, mult(v2,1.7)); // le produit mult(v2,1.7) permet de faire rebondir les particules
		//  }
		
		//---------------------------------------------------------------------------------------
		
		// ### Second version, separating the cases where the particle rolls or bounces

			Vec2 v1 = subs(particle->next_pos, sphere->center);
			if (fabs((double) dot(particle->velocity, normalize(v1))) >= MINIMUM_SPEED_FOR_BOUNCE_SPHERE)
			{
				particle->next_pos = sum(particle->position, mult(normalize(v1), BOUNCE_FACTOR*dot(particle->velocity, normalize(v1))));
			}
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
  		if (((particle->status >> (i + context->num_ground_sphere)) & 1) == 1) // if there is a contact with the plane i
  		{
  			PlaneCollider* plane = context->ground_planes + i;

		//---------------------------------------------------------------------------------------
		
		// ### First version, using speed
		  			
		//	Vec2 v1 = mult(plane->normal, dot(particle->velocity, plane->normal));
		//
		//	if (dot(particle->velocity, plane->normal) < 0)
		//	{
		// 		particle->velocity = subs(particle->velocity, mult(v1,1.7)); // le produit mult(v1,1.7) permet de faire rebondir les particules
		// 	}
					 
		//	Vec2 v1 = mult(plane->normal, dot(subs(particle->next_pos, plane->center), plane->normal));
		//	Vec2 v2 = mult(normalize(v1), norm(v1) - particle->radius);
		//	particle->next_pos = subs(particle->next_pos, mult(v2,3));
		
		//---------------------------------------------------------------------------------------
		
		// ### Second version, separating the cases where the particle rolls or bounces

			Vec2 v1 = subs(particle->next_pos, plane->center);
			if (fabs((double) dot(particle->velocity, plane->normal)) >= MINIMUM_SPEED_FOR_BOUNCE_PLANE)
			{
				particle->next_pos = subs(particle->position, mult(plane->normal, BOUNCE_FACTOR*dot(particle->velocity, plane->normal)));
			}
			else
			{
				Vec2 v2 = mult(plane->normal, dot(v1, plane->normal) - particle->radius);
				particle->next_pos = subs(particle->next_pos, v2);
			}

		//---------------------------------------------------------------------------------------
		
		// ### Third version, more realistic, but crashes when there is contact with more than one plane
		
		//	Vec2 v1 = subs(particle->next_pos, plane->center);
		//			
		//	Vec2 v2 = mult(plane->normal, dot(v1, plane->normal) - particle->radius);
		//	Vec2 v3 = subs(particle->position, subs(particle->next_pos, v2));
		//	float teta = atan2f(plane->normal.y, plane->normal.x) - atan2f(v3.y, v3.x);
		//	Vec2 v4 = {cosf(teta) * norm(v3) * plane->normal.x - sinf(teta) * norm(v3) * plane->normal.y,
		//	           sinf(teta) * norm(v3) * plane->normal.x + cosf(teta) * norm(v3) * plane->normal.y};
		//	particle->next_pos = sum(particle->position, v4);
		//	//printf("%f %f\n", particle->next_pos.x, particle->next_pos.y);
		
		//---------------------------------------------------------------------------------------
		
		// ### Version given in the equations document
		
		//	Vec2 q = subs(particle->next_pos, mult(plane->normal, dot(plane->normal, subs(particle->next_pos, plane->center))));
		//	float C = dot(subs(particle->next_pos, q), plane->normal) - particle->radius;
		//	particle->next_pos = subs(particle->position, mult(plane->normal, C));	
		}  		
	}	
}

void enforceDynamicConstraint(Context* context, Particle* particle)
{
	int i;
  	for (i=0; i < context->num_particles; i++)
  	{
		if ((particle->links >> i & 1) == 1)  // if there is a link with the particle i
		{
 			Particle* other = getParticlePointer(context, i);
//  		Vec2 v1 = normalize(subs(particle->next_pos, other->next_pos));
//  		Vec2 v2 = mult(v1, dot(particle->velocity, v1));
//  		if (dot(particle->velocity, v1) > 0)
//  		{
//				particle->velocity = subs(particle->velocity, v2);
//			}
			Vec2 d = subs(particle->next_pos, other->next_pos);
			float C = norm(d) - particle->radius - other->radius;
			particle->next_pos = subs(particle->next_pos, mult(normalize(d), C * particle->inv_mass / (particle->inv_mass + other->inv_mass)));
		}
		
  		else if (((particle->status >> (i + context->num_ground_sphere + context->num_ground_plane)) & 1) == 1)  // if there is a contact with the particle i
  		{ 			
 			Particle* other = getParticlePointer(context, i);
//  		Vec2 v1 = normalize(subs(particle->next_pos, other->next_pos));
//  		Vec2 v2 = mult(v1, dot(particle->velocity, v1));
//  			
//  		if (dot(particle->velocity, v1) < 0)
//  		{
//  			particle->velocity = subs(particle->velocity, mult(v2,1.7 * norm(particle->velocity) / (norm(particle->velocity) + norm(other->velocity))));
//  		}

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
		Particle* particle = getParticlePointer(context, i);
		enforceDynamicConstraint(context, particle);
		enforceStaticGroundConstraint(context, particle);
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

// ------------------------------------------------

//int main()
//{
//	Context* context = initializeContext(10);
//	addParticle(context, 100, 5, 10, 1, 0);
//	printf("%f\n", getParticle(context, 0)->position.y);
//	applyExternalForce(context, 1);
//	updateExpectedPosition(context, 1);
//	updateVelocityAndPosition(context, 1);
//	printf("%f\n", getParticle(context, 0)->position.y);	
//}
