#include <iostream>
#include <Eigen/Dense>
#include "Shape.h"
#include "GLSL.h"

using namespace std;

Shape::Shape() {}

Shape::~Shape() {}

void Shape::addObj(
  const string &meshName,
  const string &mtlBasePath) {
   Component toAdd;

   toAdd.obj.load(meshName, mtlBasePath);
   objs.push_back(toAdd);
}

void Shape::addObj(
  const string &meshName,
  const string &mtlBasePath,
  Eigen::Vector3f center,
  Eigen::Vector3f axis) {
   Component toAdd(center, axis);

   toAdd.obj.load(meshName, mtlBasePath);
   objs.push_back(toAdd);
}

void Shape::addCP(Eigen::Vector3f pt, Eigen::AngleAxisf rot) {
   Eigen::Quaternionf q;
   q = rot;
   if (frames.size() == 0) {
      for (int i = 0; i < 4; i++) {
         frames.push_back(KeyFrame(pt, q));
      }
   }
   else {
      vector<KeyFrame>::iterator it = frames.end();
      if (q.dot((it-3)->q) < 0) {
         q.w() *= -1;
         q.vec() *= -1;
      }
      frames.insert(it-2, KeyFrame(pt, q));
   }
}

void Shape::init() {
   for (vector<Component>::iterator it = objs.begin(); it != objs.end(); ++it) {
      it->obj.init();
   }
}

void Shape::rescale(float scale) {
   this->scale = scale;
}

void Shape::center(Eigen::Vector3f center) {
   offset = center;
}

void Shape::drawSpline() {
   int ncps = (int)frames.size();

   glPointSize(3.0f);
   glColor3f(0.0f, 0.0f, 0.0f);
   glBegin(GL_POINTS);
   for(int i = 0; i < ncps; ++i) {
      Eigen::Vector3f cp = frames[i].pos;
      glVertex3f(cp(0), cp(1), cp(2));
   }
   glEnd();
   
   glLineWidth(1.0f);
   if(ncps >= 4) {
      Eigen::Matrix4f B;
      Eigen::MatrixXf G(3,4);
      B = getCatmullMatrix();
      for (int i = 0; i <= ncps - 4 && ncps >= 4; i++) {
         for (int idx = i; idx < i + 4; idx++) {
            G(0,idx - i) = frames[idx].pos(0);
            G(1,idx - i) = frames[idx].pos(1);
            G(2,idx - i) = frames[idx].pos(2);
         }
         glBegin(GL_LINE_STRIP);
         glColor3f(1.0f, 0.0f, 1.0f);
         for (float u = 0; u < 1; u += 0.01) {
            Eigen::Vector3f p = G*B*getUVec(u);
            
            glVertex3fv(p.data());
         }
         glEnd();
      }
   }
}

void Shape::drawKeyFrames(Program &prog, MatrixStack &MV) {
   for (vector<KeyFrame>::iterator it = frames.begin(); it != frames.end(); ++it) {
      MV.pushMatrix();
      MV.translate(it->pos);
      MV.scale(scale);
      Eigen::Matrix4f R = Eigen::Matrix4f::Identity();
      R.block<3,3>(0,0) = it->q.toRotationMatrix();
      MV.multMatrix(R);
      MV.translate(-offset);
      for (vector<Component>::iterator it2 = objs.begin(); it2 != objs.end(); ++it2) {
         glUniformMatrix4fv(prog.getUniform("MV"), 1, GL_FALSE, MV.topMatrix().data());
         it2->obj.draw(prog.getAttribute("vertPos"), prog.getAttribute("vertNor"), -1);
      }
      MV.popMatrix();
   }
}

void Shape::draw(Program &prog, MatrixStack &MV, float t) {
   buildTable();
   float angle = fmod(t*10.0f, (float)M_PI*2.0f);

   float uu = s2u(t2s(t));
   // Convert from concatenated u to the usual u between 0 and 1.
   float kfloat;
   float u = std::modf(uu, &kfloat);
   int k = (int)std::floor(kfloat);
   Eigen::Matrix4f B;
   Eigen::MatrixXf G(3,4), qG(4,4);
   Eigen::Vector4f uVec;
   B = getCatmullMatrix();
   uVec = getUVec(u);
   
   for (int idx = k; idx < k + 4; idx++) {
      G(0, idx - k) = frames[idx].pos(0);
      G(1, idx - k) = frames[idx].pos(1);
      G(2, idx - k) = frames[idx].pos(2);

      qG(0, idx - k) = frames[idx].q.w();
      qG(1, idx - k) = frames[idx].q.x();
      qG(2, idx - k) = frames[idx].q.y();
      qG(3, idx - k) = frames[idx].q.z();
   }
   Eigen::Vector3f p = G*B*uVec;
   Eigen::Vector4f qVec = (qG*(B*uVec));
   Eigen::Quaternionf q;
   q.w() = qVec(0);
   q.vec() = qVec.segment<3>(1);
   q.normalize();

   for (vector<Component>::iterator it = objs.begin(); it != objs.end(); ++it) {
      glUniform3f(prog.getUniform("dif"), 0.2, 0.2, 0.3);
      glUniform3f(prog.getUniform("spec"), 0.1, 0.2, 0.8);
      glUniform3f(prog.getUniform("amb"), 0.15, 0.15, 0.15);
      glUniform1f(prog.getUniform("shine"), 20);
      // Last transform done first
      MV.pushMatrix();
      MV.translate(p);
      MV.scale(scale);
      Eigen::Matrix4f R = Eigen::Matrix4f::Identity();
      R.block<3,3>(0,0) = q.toRotationMatrix();
      MV.multMatrix(R);
      MV.translate(-offset);
      if (it->spinning) {
         MV.translate(it->center);
         MV.rotate(angle, it->axis);
         MV.translate(-it->center);
      }
      glUniformMatrix4fv(prog.getUniform("MV"), 1, GL_FALSE, MV.topMatrix().data());
      it->obj.draw(prog.getAttribute("vertPos"), prog.getAttribute("vertNor"), -1);
      MV.popMatrix();
   }
}

void Shape::buildTable() {
   usTable.clear();
   int ncps = (int)frames.size();
   Eigen::MatrixXf G(3,4);
   float totalLen = 0.0f;
   Eigen::Matrix4f B = getCatmullMatrix();

   if(ncps >= 4) {
      //BUILD TABLE
      usTable.push_back(make_pair(0.0f, totalLen));
      for (int cp = 0; cp < ncps-3; cp++) {
         for (int idx = cp; idx < cp+4; idx++) {
            G(0, idx-cp) = frames[idx].pos(0);
            G(1, idx-cp) = frames[idx].pos(1);
            G(2, idx-cp) = frames[idx].pos(2);
         }
         for (float u = 0.2f; u < 1.0001f; u += 0.2f) {
            float uA = u - 0.2, uB = u;
            Eigen::Vector4f uVecA = getUVec(uA);
            Eigen::Vector4f uVecB = getUVec(uB);
            Eigen::Vector3f pPrime;

            float dx = sqrt(3.0f/5.0f);
            float dw = 3.0f/9.0f;
            float x = -dx, w = 5.0f/9.0f;
            float sum = 0;
            for (int i = 0; i < 3; i++) {
               float pParam = (uB - uA) / 2 * x + (uA + uB) / 2;
               Eigen::Vector4f uVecP = getUVecP(pParam);
               pPrime = G*B*uVecP;
               sum += w * pPrime.norm();
               
               x += dx;
               w += dw;
               dw = -dw;
            }
            float s = (uB - uA) / 2 * sum;

            totalLen += s;
            usTable.push_back(make_pair(cp + u, totalLen));
         }
      }
   }
}

float Shape::t2s(float t) {
  float tNorm = fmod(t, (float)T_MAX) / T_MAX;
   float sNorm = tNorm;
   float s = usTable.back().second * sNorm;

   return s;
}

float Shape::s2u(float s) {
   pair<float, float> start = usTable[0], end = usTable[1];
   for (int i = 0; i < (int)usTable.size() - 1; i++) {
      pair<float, float> row1 = usTable[i];
      pair<float, float> row2 = usTable[i+1];
      if (s > row1.second && s < row2.second) {
         start = usTable[i];
         end = usTable[i+1];
         break;
      }
   }
   float a = (s - start.second) / (end.second - start.second);
   float u = (1 - a) * start.first + a * end.first;
   return u;
}

Shape::Component::Component() : 
   spinning(false)
{
}
Shape::Component::Component(Eigen::Vector3f center, Eigen::Vector3f axis) :
   spinning(true),
   center(center),
   axis(axis)
{
}
Shape::Component::~Component() {}

Shape::KeyFrame::KeyFrame(Eigen::Vector3f pos, Eigen::Quaternionf q) :
   pos(pos),
   q(q)
{
}
Shape::KeyFrame::~KeyFrame() {}
