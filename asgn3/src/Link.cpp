#include <iostream>

#include <Eigen/Dense>
#include "Link.h"

using namespace std;

Link::Link() :
   mesh(NULL),
   parent(NULL),
   child(NULL)
{}

Link::~Link(){}

void Link::init(Shape *mesh, Link *parent, float dist) {
   Em = Eigen::Matrix4f::Identity();
   Ep = Eigen::Matrix4f::Identity();
   angle = restAngle = 0;

   this->dist = dist;
   this->mesh = mesh;
   this->parent = parent;

   Eigen::Matrix4f E = Eigen::Matrix4f::Identity();
   if (parent) {
      parent->child = this;
   }
   E(0,3) = dist;
   Ep *= E;
   E(0,3) = 0.5;
   Em *= E;
}

void Link::draw(MatrixStack &M, Program *prog) {
   M.pushMatrix();
   M.multMatrix(Ep);
   M.rotate(angle, Eigen::Vector3f::UnitZ());
   M.pushMatrix();
   M.multMatrix(Em);
   glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, M.topMatrix().data());
   mesh->draw(prog);
   M.popMatrix();
   if (child) {
      child->draw(M, prog);
   }
   M.popMatrix();
}

Eigen::Matrix4f Link::endTransform(Eigen::Matrix4f transMat) {
   Eigen::Matrix4f rot = Eigen::Matrix4f::Identity();
   rot.block<3,3>(0,0) = Eigen::AngleAxisf(angle, Eigen::Vector3f::UnitZ()).toRotationMatrix();
   transMat *= Ep * rot;

   return child ? child->endTransform(transMat) : transMat * Ep;
}