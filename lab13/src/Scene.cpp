#include <iostream>
#include "Scene.h"
#include "Particle.h"
#include "Cloth.h"
#include "Shape.h"
#include "Program.h"

Scene::Scene() :
	t(0.0),
	h(1e-2),
	grav(0.0, 0.0, 0.0),
	cloth(NULL)
{
}

Scene::~Scene()
{
	while(!spheres.empty()) {
		delete spheres.back();
		spheres.pop_back();
	}

	delete cloth;
	
	delete sphereShape;
}

void Scene::load()
{
	// Units: meters, kilograms, seconds
	h = 1e-2;
	
	grav << 0.0, -9.8, 0.0;
	
	int rows = 15;
	int cols = 15;
	double mass = 0.1;
	double stiffness = 1e2;
	double dz = 0.00;
	Eigen::Vector3d x00(-0.25, 0.5, 0.0+dz);
	Eigen::Vector3d x01(0.25, 0.5, 0.0+dz);
	Eigen::Vector3d x10(-0.25, 0.5, -0.5+dz);
	Eigen::Vector3d x11(0.25, 0.5, -0.5+dz);
	cloth = new Cloth(rows, cols, x00, x01, x10, x11, mass, stiffness);
	
	sphereShape = new Shape();
	sphereShape->loadMesh("sphere2.obj");
	
	Particle *sphere = new Particle(sphereShape);
	spheres.push_back(sphere);
	sphere->r = 0.1;
}

void Scene::init()
{
	sphereShape->init();
	cloth->init();
}

void Scene::tare()
{
	for(int i = 0; i < (int)spheres.size(); ++i) {
		spheres[i]->tare();
	}
	cloth->tare();
}

void Scene::reset()
{
	t = 0.0;
	for(int i = 0; i < (int)spheres.size(); ++i) {
		spheres[i]->reset();
	}
	cloth->reset();
}

void Scene::step()
{
	t += h;
	
	// Move the sphere
	if(!spheres.empty()) {
		Eigen::Vector3d x;
		double radius = 0.5;
		double a = 2.0*t;
		x << 0.0, radius * sin(a), 0.0;
		spheres.front()->x = x;
	}
	
	// Simulate the cloth
	cloth->step(h, grav, spheres);
}

void Scene::draw(MatrixStack &MV, const Program &prog) const
{
	glUniform3fv(prog.getUniform("kdFront"), 1, Eigen::Vector3f(1.0, 1.0, 1.0).data());
	for(int i = 0; i < (int)spheres.size(); ++i) {
		spheres[i]->draw(MV, prog);
	}
	
	cloth->draw(MV, prog);
}
