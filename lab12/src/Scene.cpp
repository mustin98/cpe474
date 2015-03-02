#include <iostream>
#include "Scene.h"
#include "Shape.h"
#include "Cloth.h"

Scene::Scene() :
	t(0.0),
	h(1e-2),
	grav(0.0, 0.0, 0.0),
	sphere(NULL),
	cloth(NULL)
{
}

Scene::~Scene()
{
	delete cloth;
	delete sphere;
}

void Scene::load()
{
	// Units: meters, kilograms, seconds
	h = 1e-2;
	
	grav << 0.0, -9.8, 0.0;
	
	int rows = 10;
	int cols = 10;
	double mass = 0.1;
	double stiffness = 1e2;
	Eigen::Vector3d x00(-0.25, 0.5, 0.0);
	Eigen::Vector3d x01(0.25, 0.5, 0.0);
	Eigen::Vector3d x10(-0.25, 0.5, -0.5);
	Eigen::Vector3d x11(0.25, 0.5, -0.5);
	sphere = new Shape();
	sphere->loadMesh("sphere2.obj");
	cloth = new Cloth(sphere, rows, cols, x00, x01, x10, x11, mass, stiffness);
	cloth->setImplicit(true);
	cloth->setShowParticles(false);
}

void Scene::init()
{
	sphere->init();
	cloth->init();
}

void Scene::tare()
{
	cloth->tare();
}

void Scene::reset()
{
	cloth->reset();
}

void Scene::step()
{
	t += h;
	cloth->step(h, grav);
}

void Scene::draw(MatrixStack &MV, const Program &prog) const
{
	cloth->draw(MV, prog);
}
