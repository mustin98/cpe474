#include <iostream>
#include <Eigen/Dense>
#include "Shape.h"
#include "GLSL.h"

using namespace std;

Shape::Shape() {}

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

void Shape::addCP(Eigen::Vector3f pt, Eigen::AngleAxisf rot) {
   if (cps.size() == 0) {
      frames.push_back(KeyFrame(pt, rot));
      for (int i = 0; i < 4; i++)
         cps.push_back(pt);
   }
   else {
      vector<Eigen::Vector3f>::iterator it = cps.end();
      cps.insert(it-2, pt);
   }
}

void Shape::init() {
   for (vector<Component>::iterator it = objs.begin(); it != objs.end(); ++it) {
      it->obj.init();
   }
}

void Shape::drawSpline() {
   int ncps = (int)cps.size();

   glPointSize(3.0f);
   glColor3f(0.0f, 0.0f, 0.0f);
   glBegin(GL_POINTS);
   for(int i = 0; i < ncps; ++i) {
      Eigen::Vector3f cp = cps[i];
      glVertex3f(cp(0), cp(1), cp(2));
   }
   glEnd(); 
   
   glLineWidth(1.0f);
   if(ncps >= 4) {
      Eigen::Matrix4f B;
      Eigen::MatrixXf G(3,4);
      B = getCatmullMatrix();
      for (int i = 0; i <= ncps - 4 && ncps >= 4; i++) {
         for (int idx = i; idx < i + 4; idx++) {
            G(0,idx - i) = cps[idx](0);
            G(1,idx - i) = cps[idx](1);
            G(2,idx - i) = cps[idx](2);
         }     
         glBegin(GL_LINE_STRIP);
         glColor3f(1.0f, 0.0f, 1.0f);
         for (float u = 0; u < 1; u += 0.01) {
            Eigen::Vector3f p = G*B*getUVec(u);
            
            glVertex3fv(p.data());
         }
         glEnd();
      }
   }
}

void Shape::drawKeyFrames(Program &prog, MatrixStack &MV) {
   for (vector<KeyFrame>::iterator it = frames.begin(); it != frames.end(); ++it) {
      MV.pushMatrix();
      
      MV.popMatrix();
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

Shape::KeyFrame::KeyFrame(Eigen::Vector3f pos, Eigen::AngleAxisf rot) :
   pos(pos),
   q(rot)
{
}
Shape::KeyFrame::~KeyFrame() {}