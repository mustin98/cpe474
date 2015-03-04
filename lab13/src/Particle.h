#pragma once
#ifndef __L11__Particle__
#define __L11__Particle__

#include <stdio.h>
#include <vector>
#include <Eigen/Dense>
#include "MatrixStack.h"

class Shape;
class Program;

class Particle
{
public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW
	
	Particle();
	Particle(const Shape *shape);
	virtual ~Particle();
	void tare();
	void reset();
	void draw(MatrixStack &MV, const Program &p) const;
	
	double r; // radius
	double m; // mass
	int i;  // starting index
	Eigen::Vector3d x0; // initial position
	Eigen::Vector3d v0; // initial velocity
	Eigen::Vector3d x;  // position
	Eigen::Vector3d v;  // velocity
	bool fixed;
	
private:
	const Shape *sphere;
};

#endif /* defined(__L11__Particle__) */
