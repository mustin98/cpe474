#include "Spring.h"
#include "Particle.h"

Spring::Spring(Particle *p0, Particle *p1) :
	K(1.0)
{
	assert(p0);
	assert(p1);
	assert(p0 != p1);
	this->p0 = p0;
	this->p1 = p1;
	Eigen::Vector3d x0 = p0->x;
	Eigen::Vector3d x1 = p1->x;
	Eigen::Vector3d dx = x1 - x0;
	L = dx.norm();
	assert(L > 0.0);
}

Spring::~Spring()
{
	
}
