#pragma  once
#ifndef __Camera__
#define __Camera__

#include <Eigen/Dense>

class MatrixStack;

class Camera
{
public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW
	
	enum {
		ROTATE = 0,
		TRANSLATE,
		SCALE
	};
	
	Camera();
	virtual ~Camera();
	void setPerspective(float aspect, float fovy, float znear, float zfar);
	void setOrtho(float l, float r, float b, float t, float znear, float zfar);
	void setRotationFactor(float f) { rfactor = f; }
	void setTranslationFactor(float f) { tfactor = f; }
	void setScaleFactor(float f) { sfactor = f; }
	void mouseClicked(int x, int y, bool shift, bool ctrl, bool alt);
	void mouseMoved(int x, int y);
	void applyProjectionMatrix(MatrixStack *P) const;
	void applyViewMatrix(MatrixStack *MV) const;
	
private:
	bool ortho;
	float aspect;
	float fovy;
	float left;
	float right;
	float bottom;
	float top;
	float znear;
	float zfar;
	Eigen::Vector2f rotations;
	Eigen::Vector3f translations;
	float scale;
	Eigen::Vector2f mousePrev;
	int state;
	float rfactor;
	float tfactor;
	float sfactor;
};

#endif
