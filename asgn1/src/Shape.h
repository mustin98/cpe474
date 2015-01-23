#pragma once
#ifndef _SHAPE_H_
#define _SHAPE_H_

#include "ShapeObj.h"
#include "MatrixStack.h"
#include "Program.h"

class Shape {
public:
   Shape();
   virtual ~Shape();
   void addObj(const std::string &meshName);
   void addObj(const std::string &meshName, Eigen::Vector3f center, Eigen::Vector3f axis);
   void init();
   void draw(Program &prog, MatrixStack &MV, float t);
   
private:
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
   class KeyFrame {
   public:

   };
   std::vector<Component> objs;
   std::vector<KeyFrame> frames;
};

#endif
