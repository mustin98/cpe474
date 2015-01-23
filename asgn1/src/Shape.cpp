#include <iostream>
#include <Eigen/Dense>
#include "Shape.h"
#include "GLSL.h"

using namespace std;

Shape::Shape() {
}

Shape::~Shape() {}

void Shape::addObj(const string &meshName) {
   Component toAdd;

   toAdd.obj.load(meshName);
   objs.push_back(toAdd);
}

void Shape::addObj(const string &meshName, Eigen::Vector3f center, Eigen::Vector3f axis) {
   Component toAdd(center, axis);

   toAdd.obj.load(meshName);
   objs.push_back(toAdd);
}

void Shape::init() {
   for (vector<Component>::iterator it = objs.begin(); it != objs.end(); ++it) {
      it->obj.init();
   }
}

void Shape::draw(Program &prog, MatrixStack &MV, float t) {
   float angle = fmod(t*15.0f, (float)M_PI*2.0f);

   for (vector<Component>::iterator it = objs.begin(); it != objs.end(); ++it) {
      MV.pushMatrix();
      //Eigen::Matrix4f R = Eigen::Matrix4f::Identity();
      //R.block<3,3>(0,0) = q0.toRotationMatrix();
      //MV.multMatrix(R);
      
      // Last transform done first
      if (it->spinning) {
         MV.translate(it->center);
         MV.rotate(angle, it->axis);
         MV.translate(-it->center);
      }
      glUniformMatrix4fv(prog.getUniform("MV"), 1, GL_FALSE, MV.topMatrix().data());
      it->obj.draw(prog.getAttribute("vertPos"), prog.getAttribute("vertNor"), -1);
      MV.popMatrix();
   }  
}

Shape::Component::Component() : 
   spinning(false)
{
}

Shape::Component::Component(Eigen::Vector3f center, Eigen::Vector3f axis) :
   spinning(true),
   center(center),
   axis(axis)
{
}

Shape::Component::~Component() {}
