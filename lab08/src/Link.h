#pragma once
#ifndef _LINK_H_
#define _LINK_H_

#include <Eigen/Dense>
#include <vector>

#include "MatrixStack.h"
#include "Program.h"
#include "Shape.h"

class Link {
public:
   Link();
   virtual ~Link();
   void draw(MatrixStack &M, Program *prog);
   void init(Shape *mesh, Link *parent, float dist);

   Shape *mesh;
   Link *parent;
   std::vector<Link*> children;
   Eigen::Matrix4f Ep;
   Eigen::Matrix4f Em;
   float angle;
   float dist;
};

#endif
