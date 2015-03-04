#ifndef __L11__Scene__
#define __L11__Scene__

#include <stdio.h>
#include <vector>
#include <Eigen/Dense>

class Cloth;
class Particle;
class MatrixStack;
class Program;
class Shape;

class Scene
{
public:
	Scene();
	virtual ~Scene();
	
	void load();
	void init();
	void tare();
	void reset();
	void step();
	
	void draw(MatrixStack &MV, const Program &prog) const;
	
	double getTime() const { return t; }
	
private:
	double t;
	double h;
	Eigen::Vector3d grav;
	
	Shape *sphereShape;

	Cloth *cloth;
	std::vector<Particle *> spheres;
};

#endif /* defined(__L11__Scene__) */
