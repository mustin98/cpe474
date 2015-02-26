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
	
	Particle(Shape *shape);
	virtual ~Particle();
	void tare();
	void reset();
	void draw(MatrixStack &MV, const Program &p) const;
	
	double m;
	Eigen::Vector3d x0;
	Eigen::Vector3d v0;
	Eigen::Vector3d x;
	Eigen::Vector3d v;
	
	float r;
	Shape *shape;
};

#endif /* defined(__L11__Particle__) */
