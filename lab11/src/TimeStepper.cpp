#include "TimeStepper.h"
#include "Particle.h"

TimeStepper::TimeStepper()
{
	t = 0.0;
	h = 1e-2;
	type = 0;
}

TimeStepper::~TimeStepper()
{
	
}

void TimeStepper::step(std::vector<Particle*> &particles)
{
	t += h;
	Particle *p0 = particles[0];
	Particle *p1 = particles[1];
	double m0 = p0->m;
	double m1 = p1->m;
	Eigen::Vector3d x0 = p0->x;
	Eigen::Vector3d x1 = p1->x;
	Eigen::Vector3d v0 = p0->v;
	Eigen::Vector3d v1 = p1->v;
	double stiffness = 1e1;
	if(type == 0) {
		// Explicit Euler
		Eigen::Vector3d vtmp = v0;
		v0 -= h * (1/m0) * stiffness * x0;
		x0 += vtmp * h;
		p0->v = v0;
		p0->x = x0;
		vtmp = v1;
		v1 -= h * (1/m1) * stiffness * x1;
		x1 += vtmp * h;
		p1->v = v1;
		p1->x = x1;
	} else if(type == 1) {
		// Implicit Euler
		Eigen::Matrix3d M0 = m0 * Eigen::Matrix3d::Identity();
	} else {
		// Symplectic Euler
		v0 = 1 / (1 + h*h * (1/m0) * stiffness) * (v0 - h * (1/m0) * stiffness * x0);
		x0 += h*v0;
		v1 = 1 / (1 + h*h * (1/m1) * stiffness) * (v1 - h * (1/m1) * stiffness * x1);
		x1 += h*v1;
		p0->x = x0;
		p0->v = v0;
		p1->x = x1;
		p1->v = v1;
	}
}

void TimeStepper::reset(std::vector<Particle *> &particles)
{
	for(int i = 0; i < (int)particles.size(); ++i) {
		particles[i]->reset();
	}
	t = 0.0;
}

