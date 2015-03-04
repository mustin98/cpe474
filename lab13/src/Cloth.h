#pragma once
#ifndef __L12__Cloth__
#define __L12__Cloth__

#include <stdio.h>
#include <vector>
#include <Eigen/Dense>

class Particle;
class Spring;
class MatrixStack;
class Program;

class Cloth
{
public:
	Cloth(int rows, int cols,
		  const Eigen::Vector3d &x00,
		  const Eigen::Vector3d &x01,
		  const Eigen::Vector3d &x10,
		  const Eigen::Vector3d &x11,
		  double mass, double stiffness);
	virtual ~Cloth();
	
	void tare();
	void reset();
	void updatePosNor();
	void step(double h, const Eigen::Vector3d &grav, const std::vector<Particle *> &spheres);
	
	void init();
	void draw(MatrixStack &MV, const Program &p) const;
	
private:
	int rows;
	int cols;
	int n;
	std::vector<Particle*> particles;
	std::vector<Spring*> springs;
	
	Eigen::MatrixXd M;
	Eigen::MatrixXd K;
	Eigen::VectorXd v;
	Eigen::VectorXd f;
	
	std::vector<unsigned int> eleBuf;
	std::vector<float> posBuf;
	std::vector<float> norBuf;
	std::vector<float> texBuf;
	unsigned eleBufID;
	unsigned posBufID;
	unsigned norBufID;
	unsigned texBufID;
};

#endif /* defined(__L12__Cloth__) */
