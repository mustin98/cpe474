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
   Eigen::Matrix4f endTransform(Eigen::Matrix4f transMat);

   Shape *mesh;
   Link *parent;
   Link *child;
   Eigen::Matrix4f Ep;
   Eigen::Matrix4f Em;
   double angle;
   double restAngle;
   float dist;
};

#endif
