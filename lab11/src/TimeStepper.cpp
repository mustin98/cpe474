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
		Eigen::MatrixXd i(6,6);
		i.setZero();
		Eigen::Matrix3d I = Eigen::Matrix3d::Identity();
		i.block<3,3>(0,0) = I;
		i.block<3,3>(0,3) = -I;
		i.block<3,3>(3,0) = -I;
		i.block<3,3>(3,3) = I;
		Eigen::MatrixXd m(6,6);
		m.setZero();
		m.block<3,3>(0,0) = m0 * I;
	   m.block<3,3>(3,3) = m1 * I;
	   Eigen::MatrixXd A(6,6);
	   A = m + h*h *stiffness * i;

		Eigen::VectorXd v(6);
		v.setZero();
		v.segment<3>(0) = v0;
		v.segment<3>(3) = v1;
		Eigen::VectorXd x(6);
		x.setZero();
		x.segment<3>(0) = x1 - x0;
		x.segment<3>(3) = x0 - x1;
		Eigen::VectorXd b(6);
		b = m * v + h * stiffness * x;
		
		Eigen::VectorXd X = A.colPivHouseholderQr().solve(b);
		v0 = X.segment<3>(0);
		v1 = X.segment<3>(3);		

		x0 += h * v0;
		x1 += h * v1;

		p0->x = x0;
		p1->x = x1;
		p0->v = v0;
		p1->v = v1;
	} else {
		// Symplectic Euler
		v0 -= h * (1/m0) * stiffness * x0;
		x0 += h * v0;
		v1 -= h * (1/m1) * stiffness * x1;
		x1 += h * v1;
		p0->v = v0;
		p1->v = v1;
		p0->x = x0;
		p1->x = x1;
		/*v0 -= 1 / (1 + h*h * (1/m0) * stiffness) * (v0 - h * (1/m0) * stiffness * x0);
		x0 += h*v0;
		v1 = 1 / (1 + h*h * (1/m1) * stiffness) * (v1 - h * (1/m1) * stiffness * x1);
		x1 += h*v1;
		p0->x = x0;
		p0->v = v0;
		p1->x = x1;
		p1->v = v1;*/
	}
}

void TimeStepper::reset(std::vector<Particle *> &particles)
{
	for(int i = 0; i < (int)particles.size(); ++i) {
		particles[i]->reset();
	}
	t = 0.0;
}

