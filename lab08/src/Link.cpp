#include <iostream>

#include <Eigen/Dense>
#include "Link.h"

using namespace std;

Link::Link() :
   mesh(NULL),
   parent(NULL)
{}

Link::~Link(){}

void Link::init(Shape *mesh, Link *parent, float dist) {
   Em = Eigen::Matrix4f::Identity();
   Ep = Eigen::Matrix4f::Identity();

   this->dist = dist;
   this->mesh = mesh;
   this->parent = parent;

   if (parent) {
      parent->children.push_back(this);
      Eigen::Matrix4f E = Eigen::Matrix4f::Identity();
      E(0,3) = dist;
      Ep *= E;
      E(0,3) = dist > 0 ? dist/dist/2 : dist/dist/-2;
      Em *= E;
   }
}

void Link::draw(MatrixStack &M, Program *prog) {
   M.pushMatrix();
   M.multMatrix(Ep);
   M.rotate(angle, Eigen::Vector3f(0,0,1));
   M.pushMatrix();
   M.multMatrix(Em);
   glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, M.topMatrix().data());
   mesh->draw(prog);
   M.popMatrix();
   for (int i = 0; i < children.size(); i++) {
      children[i]->draw(M, prog);
   }
   M.popMatrix();
}