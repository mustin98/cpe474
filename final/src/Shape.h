#pragma once
#ifndef _SHAPE_H_
#define _SHAPE_H_

#define T_MAX 10.0

#include <utility>
#include "ShapeObj.h"
#include "MatrixStack.h"
#include "Program.h"

class Shape {
public:
   Shape();
   virtual ~Shape();
   void addObj(const std::string &meshName,
               const std::string &mtlBasePath);
   void addObj(const std::string &meshName,
               const std::string &mtlBasePath,
               Eigen::Vector3f center,
               Eigen::Vector3f axis);
   void init();
   void draw(Program &prog, MatrixStack &MV, float t);
   void drawSpline();
   void drawCPs();
   void drawCBs();
   void addCB(Eigen::Vector3f center, Eigen::Vector3f dim, bool two);
   void rescale(float scale);
   void center(Eigen::Vector3f center);
   void switchCB(int change);
   void xMove(float dx);
   void yMove(float dy);
   void zMove(float dz);

private:
   class ControlBox{
      public:
         ControlBox(Eigen::Vector3f center, Eigen::Vector3f dimensions, bool two);
         virtual ~ControlBox();
         void draw(bool active);
         void keepInside();

         Eigen::Vector3f pos;
         Eigen::Vector3f dimensions;
         std::vector<Eigen::Vector3f> cps;
   };
   class Component {
      public:
         Component();
         Component(Eigen::Vector3f center, Eigen::Vector3f axis);
         virtual ~Component();

         ShapeObj obj;
         bool spinning;
         Eigen::Vector3f center;
         Eigen::Vector3f axis;
   };
   
   void buildTable();
   float t2s(float t);
   float s2u(float s);
   void fillCPs();
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
   std::vector<ControlBox> cbs;
   int activeCB; // current controlled ControlBox
   std::vector<Eigen::Vector3f> cps;
   int ncps;
   std::vector<std::pair<float,float> > usTable;
   float scale;
   Eigen::Vector3f offset;
   float vMin; // Minimum velocity
   float v;    // Velocity
   float m;    // Mass
   float g;    // Gravity
   Eigen::Vector3f tangent; // Direction
   Eigen::Vector3f axis;    // Axis of rotation
   Eigen::Vector3f point;   // Initial direction
   float tangentAngle;      // Rotation angle
};

#endif
