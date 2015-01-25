#pragma once
#ifndef _SHAPE_H_
#define _SHAPE_H_

#define T_MAX 5.0

#include <utility>
#include "ShapeObj.h"
#include "MatrixStack.h"
#include "Program.h"

class Shape {
public:
   Shape();
   virtual ~Shape();
   void addObj(const std::string &meshName, int matID);
   void addObj(const std::string &meshName, Eigen::Vector3f center, Eigen::Vector3f axis, int matID);
   void init();
   void draw(Program &prog, MatrixStack &MV, float t);
   void drawKeyFrames(Program &prog, MatrixStack &MV);
   void drawSpline();
   void addCP(Eigen::Vector3f pt, Eigen::AngleAxisf rot);
   
private:
   class Component {
      public:
         Component(int matID);
         Component(Eigen::Vector3f center, Eigen::Vector3f axis, int matID);
         virtual ~Component();
         void setMaterial(Program &prog);

         ShapeObj obj;
         bool spinning;
         Eigen::Vector3f center;
         Eigen::Vector3f axis;
         int matID;
   };
   class KeyFrame {
      public:
         KeyFrame(Eigen::Vector3f pos, Eigen::Quaternionf q);
         virtual ~KeyFrame();

         Eigen::Vector3f pos;
         Eigen::Quaternionf q;
   };
   
   void buildTable();
   float t2s(float t);
   float s2u(float s);
   static Eigen::Matrix4f getCatmullMatrix() {
      Eigen::Matrix4f B;
      B <<  0, -1,  2, -1,
            2,  0, -5,  3,
            0,  1,  4, -3,
            0,  0, -1,  1;
      return B * 0.5;
   };
   static Eigen::Vector4f getUVec(float u) { return Eigen::Vector4f(1, u, u*u, u*u*u); };
   static Eigen::Vector4f getUVecP(float u) { return Eigen::Vector4f(0, 1, 2*u, 3*u*u); };
   static Eigen::Vector4f getUVecPP(float u) { return Eigen::Vector4f(0, 0, 2, 6*u); };

   std::vector<Component> objs;
   std::vector<KeyFrame> frames;
   std::vector<std::pair<float,float> > usTable;
};

#endif
