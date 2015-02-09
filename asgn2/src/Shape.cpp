#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <Eigen/Dense>
#include "Shape.h"
#include "GLSL.h"

using namespace std;

Shape::Shape() {}

Shape::~Shape() {}

void Shape::addObj(const string &meshName, int matID) {
   Component toAdd(matID);

   toAdd.obj.load(meshName);
   toAdd.pos = toAdd.obj.shapes[0].mesh.positions;
   toAdd.nor = toAdd.obj.shapes[0].mesh.normals;
   objs.push_back(toAdd);
}

void Shape::loadBones(const char* bones) {
   Component &obj = objs.back();
   ifstream in;
   in.open(bones);
   if (!in.good()) {
      cout << "Cannot read " << bones << endl;
      return;
   }
   string line;
   getline(in, line);
   getline(in, line);
   in >> obj.numVerts;
   in >> obj.numBones;
   obj.bones.resize(obj.numVerts);
   int vert = 0;

   while(1) {
      getline(in, line);
      if (in.eof()) {
         break;
      }
      // Skip empty lines
      if(line.size() < 2) {
         continue;
      }
      // Skip comments
      if(line.at(0) == '#') {
         continue;
      }
      // Parse line
      float weight;
      stringstream ss(line);
      for (int bone = 0; bone < obj.numBones; bone++) {
         ss >> weight;
         obj.weights.push_back(weight);
         if (weight > 0.0) {
            obj.bones[vert].push_back(bone);
         }
      }
      vert++;
   }
   in.close();
}

void Shape::loadAnimation(const char* animation) {
   Component &obj = objs.back();
   ifstream in;
   in.open(animation);
   if (!in.good()) {
      cout << "Cannot read " << animation << endl;
      return;
   }
   string line;
   getline(in, line);
   getline(in, line);
   getline(in, line);
   in >> obj.numFrames;
   in >> obj.numBones;
   float qx, qy, qz, qw, px, py, pz;
   for (int bones = 0; bones < obj.numBones; bones++) {
      in >> qx;
      in >> qy;
      in >> qz;
      in >> qw;
      in >> px;
      in >> py;
      in >> pz;
      obj.bindPose.push_back(qx);
      obj.bindPose.push_back(qy);
      obj.bindPose.push_back(qz);
      obj.bindPose.push_back(qw);
      obj.bindPose.push_back(px);
      obj.bindPose.push_back(py);
      obj.bindPose.push_back(pz);
   }

   while(1) {
      getline(in, line);
      if (in.eof()) {
         break;
      }
      // Skip empty lines
      if(line.size() < 2) {
         continue;
      }
      // Skip comments
      if(line.at(0) == '#') {
         continue;
      }
      // Parse line
      stringstream ss(line);
      // First line is bind pose
      for (int bones = 0; bones < obj.numBones; bones++) {
         ss >> qx;
         ss >> qy;
         ss >> qz;
         ss >> qw;
         ss >> px;
         ss >> py;
         ss >> pz;
         obj.aFrames.push_back(qx);
         obj.aFrames.push_back(qy);
         obj.aFrames.push_back(qz);
         obj.aFrames.push_back(qw);
         obj.aFrames.push_back(px);
         obj.aFrames.push_back(py);
         obj.aFrames.push_back(pz);
      }
   }
   in.close();
}

void Shape::init() {
   for (vector<Component>::iterator it = objs.begin(); it != objs.end(); ++it) {
      it->obj.init();
   }
}

void Shape::draw(Program &prog, MatrixStack &MV, float t) {
   for (vector<Component>::iterator it = objs.begin(); it != objs.end(); ++it) {
      int frame = floor(fmod(t, 10.0f) / 10.0f * it->numFrames);
      
      MV.pushMatrix();
      glUniformMatrix4fv(prog.getUniform("MV"), 1, GL_FALSE, MV.topMatrix().data());
      it->setMaterial(prog);
      it->animate(frame);
      it->obj.draw(prog.getAttribute("vertPos"), prog.getAttribute("vertNor"), -1);
      MV.popMatrix();
   }
}

Shape::Component::Component(int matID) :
   matID(matID)
{
}

Shape::Component::~Component() {}

void Shape::Component::setMaterial(Program &prog) {
   switch (matID) {
      case 0: // shiny blue plastic
         glUniform3f(prog.getUniform("mat.dColor"), 0.0, 0.08, 0.5);
         glUniform3f(prog.getUniform("mat.sColor"), 0.14, 0.14, 0.4);
         glUniform3f(prog.getUniform("mat.aColor"), 0.02, 0.02, 0.1);
         glUniform1f(prog.getUniform("mat.shine"), 120.0f);
         break;
      case 1: // shiny green plastic
         glUniform3f(prog.getUniform("mat.dColor"), 0.0, 0.5, 0.08);
         glUniform3f(prog.getUniform("mat.sColor"), 0.14, 0.4, 0.14);
         glUniform3f(prog.getUniform("mat.aColor"), 0.02, 0.1, 0.02);
         glUniform1f(prog.getUniform("mat.shine"), 120.0f);
         break;
      case 2: // dull grey
         glUniform3f(prog.getUniform("mat.dColor"), 0.3, 0.3, 0.4);
         glUniform3f(prog.getUniform("mat.sColor"), 0.3, 0.3, 0.4);
         glUniform3f(prog.getUniform("mat.aColor"), 0.13, 0.13, 0.14);
         glUniform1f(prog.getUniform("mat.shine"), 4.0f);
         break;
   }
}

void Shape::Component::animate(int frame) {
   int k = frame;
   Eigen::Quaternionf q;
   Eigen::Vector3f p;
   Eigen::Vector4f x;  // transformed vertex
   float w;            // skinning weight
   Eigen::Matrix4f M;  // animation matrix
   Eigen::Matrix4f M0; // bind matrix
   Eigen::Vector4f x0; // initial vertex
   Eigen::Vector4f n0; // initial normal vector
   Eigen::Vector4f n;  // transformed normal vector
   
   for (int i = 0; i < numVerts; i++) {
      x << 0,0,0,0;
      n << 0,0,0,0;
      for (int b = 0; b < bones[i].size(); b++) {
         int j = bones[i][b];
         w = weights[i*numBones + j];
         q.vec() << aFrames[k*numBones*7 + j*7 + 0], aFrames[k*numBones*7 + j*7 + 1], aFrames[k*numBones*7 + j*7 + 2];
         q.w() = aFrames[k*numBones*7 + j*7 + 3];
         p << aFrames[k*numBones*7 + j*7 + 4], aFrames[k*numBones*7 + j*7 + 5], aFrames[k*numBones*7 + j*7 + 6];
         M = Eigen::Matrix4f::Identity();
         q.normalize();
         M.block<3,3>(0,0) = q.toRotationMatrix();
         M.block<3,1>(0,3) = p;
         
         q.vec() << bindPose[j*7 + 0], bindPose[j*7 + 1], bindPose[j*7 + 2];
         q.w() = bindPose[j*7 + 3];
         p << bindPose[j*7 + 4], bindPose[j*7 + 5], bindPose[j*7 + 6];
         M0 = Eigen::Matrix4f::Identity();
         q.normalize();
         M0.block<3,3>(0,0) = q.toRotationMatrix();
         M0.block<3,1>(0,3) = p;
         x0 << pos[i*3+0], pos[i*3+1], pos[i*3+2], 1;
         n0 << nor[i*3+0], nor[i*3+1], nor[i*3+2], 0;

         x += w * (M * (M0.inverse() * x0));
         n += w * (M * (M0.inverse() * n0));
      }
      obj.shapes[0].mesh.positions[i*3+0] = x(0);
      obj.shapes[0].mesh.positions[i*3+1] = x(1);
      obj.shapes[0].mesh.positions[i*3+2] = x(2);
      obj.shapes[0].mesh.normals[i*3+0] = n(0);
      obj.shapes[0].mesh.normals[i*3+1] = n(1);
      obj.shapes[0].mesh.normals[i*3+2] = n(2);
   }
}
