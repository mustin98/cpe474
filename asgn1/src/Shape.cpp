#include <iostream>
#include <Eigen/Dense>
#include "Shape.h"
#include "GLSL.h"

using namespace std;

Shape::Shape() {
}

Shape::~Shape() {
}

void Shape::addObj(const std::string &meshName) {
   ShapeObj toAdd;

   toAdd.load(meshName);
   objs.push_back(toAdd);
}

void Shape::addObj(const std::string &meshName, Eigen::Vector3f &axis) {
   ShapeObj toAdd(axis);

   toAdd.load(meshName);
   objs.push_back(toAdd);
}

void Shape::init() {
   for (std::vector<ShapeObj>::iterator it = objs.begin(); it != objs.end(); ++it) {
      it->init();
   }
}

void Shape::rotatePropellers(Program &prog, MatrixStack &MV, float t) {
   float angle = std::fmod(t*25, M_PI*2);


}

void Shape::draw(Program &prog, MatrixStack &MV, float t) {
   MV.pushMatrix();
   //Eigen::Matrix4f R = Eigen::Matrix4f::Identity();
   //R.block<3,3>(0,0) = q0.toRotationMatrix();
   //MV.multMatrix(R);
   MV.rotate(angle, Eigen::Vector3f(0, 1, 0));
   glUniformMatrix4fv(prog.getUniform("MV"), 1, GL_FALSE, MV.topMatrix().data());
   for (std::vector<ShapeObj>::iterator it = objs.begin(); it != objs.end(); ++it) {
      it->draw(prog.getAttribute("vertPos"), prog.getAttribute("vertNor"), -1);
   }  
   MV.popMatrix();
}
