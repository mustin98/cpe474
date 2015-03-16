#include <iostream>
#include <Eigen/Dense>
#include "Obstacle.h"
#include "GLSL.h"

using namespace std;

Obstacle::Obstacle() {}

Obstacle::~Obstacle() {}

void Obstacle::mesh(const std::string &meshName,
               Eigen::Vector3f pos) {
   obj.load(meshName);
}
void Obstacle::init() {
   obj.init();
}

void Obstacle::draw(Program &prog, MatrixStack &MV) {
   MV.pushMatrix();
   MV.translate(pos);
   glUniformMatrix4fv(prog.getUniform("MV"), 1, GL_FALSE, MV.topMatrix().data());
   obj.draw(prog.getAttribute("vertPos"), -1, prog.getAttribute("vertTex"));
   MV.popMatrix();
}
