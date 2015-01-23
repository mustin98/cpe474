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
   void init();
   void draw(Program &prog, MatrixStack &MV, float t);
   
private:
   /* Specific for helicopters */
   void rotatePropellers(Program &prog, MatrixStack &MV, float t) {
   std::vector<ShapeObj> objs;
   MatrixStack stack;
};

#endif
