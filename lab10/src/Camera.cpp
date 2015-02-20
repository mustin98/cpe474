#include "Camera.h"
#include "MatrixStack.h"
#include <iostream>

Camera::Camera() :
	ortho(false),
	aspect(1.0f),
	fovy(45.0f/180.0f*M_PI),
	left(-1.0f),
	right(1.0f),
	bottom(-1.0f),
	top(1.0f),
	znear(0.1f),
	zfar(1000.0f),
	rotations(0.0, 0.0),
	translations(0.0f, 0.0f, 5.0f),
	rfactor(0.01f),
	tfactor(0.005f),
	sfactor(0.005f)
{
}

Camera::~Camera()
{
	
}

void Camera::setPerspective(float aspect, float fovy, float znear, float zfar)
{
	ortho = false;
	this->aspect = aspect;
	this->fovy = fovy;
	this->znear = znear;
	this->zfar = zfar;
}

void Camera::setOrtho(float l, float r, float b, float t, float znear, float zfar)
{
	ortho = true;
	left = l;
	right = r;
	bottom = b;
	top = t;
	this->znear = znear;
	this->zfar = zfar;
}

void Camera::mouseClicked(int x, int y, bool shift, bool ctrl, bool alt)
{
	mousePrev << x, y;
	if(shift && !ctrl && !alt) {
		state = Camera::TRANSLATE;
	} else if(!shift && ctrl && !alt) {
		state = Camera::SCALE;
	} else if(!shift && !ctrl && !alt) {
		state = Camera::ROTATE;
	} else {
		state = Camera::NONE;
	}
}

void Camera::mouseMoved(int x, int y)
{
	Eigen::Vector2f mouseCurr(x, y);
	Eigen::Vector2f dv = mouseCurr - mousePrev;
	switch(state) {
		case Camera::ROTATE:
			rotations += rfactor * dv;
			break;
		case Camera::TRANSLATE:
			translations(0) -= tfactor * dv(0);
			translations(1) += tfactor * dv(1);
			break;
		case Camera::SCALE:
			translations(2) *= (1.0f - sfactor * dv(1));
			break;
	}
	mousePrev = mouseCurr;
}

void Camera::applyProjectionMatrix(MatrixStack *P) const
{
	if(ortho) {
		P->ortho(left, right, bottom, top, znear, zfar);
	} else {
		P->perspective(fovy, aspect, znear, zfar);
	}
}

void Camera::applyViewMatrix(MatrixStack *MV) const
{
	MV->translate(-translations);
	MV->rotate(rotations(1), Eigen::Vector3f(1.0f, 0.0f, 0.0f));
	MV->rotate(rotations(0), Eigen::Vector3f(0.0f, 1.0f, 0.0f));
}
