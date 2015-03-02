#ifndef __L11__Scene__
#define __L11__Scene__

#include <stdio.h>
#include <vector>
#include <Eigen/Dense>

class Cloth;
class Shape;
class MatrixStack;
class Program;

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

	Cloth *cloth;
	Shape *sphere;
};

#endif /* defined(__L11__Scene__) */
