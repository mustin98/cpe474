#pragma once
#ifndef _SHAPE_H_
#define _SHAPE_H_

#include <utility>
#include "ShapeObj.h"
#include "MatrixStack.h"
#include "Program.h"

class Shape {
public:
   Shape();
   virtual ~Shape();
   void addObj(const std::string &meshName, int matID);
   void init();
   void draw(Program &prog, MatrixStack &MV, float t);
   void loadBones(const char* bones);
   void loadAnimation(const char* animation);
   
private:
   class Component {
      public:
         Component(int matID);
         virtual ~Component();
         void setMaterial(Program &prog);
         void animate(int frame);

         ShapeObj obj;
         int numVerts;
         int numBones;
         int numFrames;
         std::vector<float> pos;
         std::vector<float> nor;
         std::vector<float> weights;
         std::vector< std::vector<float> > bones;
         std::vector<float> aFrames;
         std::vector<float> bindPose;
         int matID;
   };
   
   std::vector<Component> objs;
};

#endif
