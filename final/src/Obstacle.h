#pragma once
#ifndef _OBSTACLE_H_
#define _OBSTACLE_H_

#include <utility>
#include "ShapeObj.h"
#include "MatrixStack.h"
#include "Program.h"

class Obstacle {
public:
   Obstacle();
   virtual ~Obstacle();
   void mesh(const std::string &meshName,
             Eigen::Vector3f pos);
   void init();
   void draw(Program &prog, MatrixStack &MV);
   ShapeObj obj;
   float radius;
   Eigen::Vector3f pos; // position
};

#endif
