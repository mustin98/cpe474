#include <iostream>
#include <Eigen/Dense>
#include "Shape.h"
#include "GLSL.h"

using namespace std;

Shape::Shape() :
   vMin(2.0),
   v(0.0),
   m(10.0),
   g(-9.8),
   tangent(Eigen::Vector3f(0,0,-1)),
   axis(Eigen::Vector3f(0,1,0)),
   point(Eigen::Vector3f(0,0,-1)),
   tangentAngle(0),
   activeCB(0),
   ncps(0)
{
}

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

// First and last ControlBox should have two=true
void Shape::addCB(Eigen::Vector3f center, Eigen::Vector3f dim, bool two) {
   cbs.push_back(ControlBox(center, dim, two));
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

void Shape::switchCB(int change) {
   activeCB += change;
   activeCB = activeCB < 0 ? cbs.size() - 1 : activeCB % cbs.size();
}

void Shape::drawSpline() {
   fillCPs();

   glPointSize(3.0f);
   glColor3f(0.0f, 0.0f, 0.0f);
   glBegin(GL_POINTS);
   for(int i = 0; i < ncps; ++i) {
      Eigen::Vector3f cp = cps[i];
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
            G(0,idx - i) = cps[idx](0);
            G(1,idx - i) = cps[idx](1);
            G(2,idx - i) = cps[idx](2);
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

void Shape::drawCPs() {
   fillCPs();

   glPointSize(3.0f);
   glColor3f(0.0f, 0.0f, 0.0f);
   glBegin(GL_POINTS);
   for(int i = 0; i < ncps; ++i) {
      Eigen::Vector3f cp = cps[i];
      glVertex3f(cp(0), cp(1), cp(2));
   }
   glEnd();
   
   glColor3f(1.0f, 0.5f, 0.5f);
   glBegin(GL_LINE_STRIP);
   for(int i = 0; i < ncps; ++i) {
      Eigen::Vector3f cp = cps[i];
      // You can also specify an array by using glVertex3fv()
      glVertex3fv(cp.data());
   }
   glEnd();
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
   Eigen::MatrixXf G(3,4);
   Eigen::Vector4f uVec, uVecP;
   B = getCatmullMatrix();
   uVec = getUVec(u);
   uVecP = getUVecP(u);
   fillCPs();
   
   for (int idx = k; idx < k + 4; idx++) {
      G(0, idx - k) = cps[idx](0);
      G(1, idx - k) = cps[idx](1);
      G(2, idx - k) = cps[idx](2);
   }
   Eigen::Vector3f p = G*B*uVec;
   tangent = G*B*uVecP;
   tangent.normalize();

   for (vector<Component>::iterator it = objs.begin(); it != objs.end(); ++it) {
      // Really should have put this somewhere else...
      glUniform3f(prog.getUniform("dif"), 0.2, 0.2, 0.3);
      glUniform3f(prog.getUniform("spec"), 0.1, 0.2, 0.8);
      glUniform3f(prog.getUniform("amb"), 0.15, 0.15, 0.15);
      glUniform1f(prog.getUniform("shine"), 20);
      // Last transform done first
      MV.pushMatrix();
      MV.translate(p);
      MV.scale(scale);
      axis = tangent.cross(point);
      axis.normalize();
      tangentAngle = acos(tangent.dot(point));
      Eigen::Quaternionf q;
      q = Eigen::AngleAxisf(tangentAngle, axis);
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
   fillCPs();
   Eigen::MatrixXf G(3,4);
   float totalLen = 0.0f;
   Eigen::Matrix4f B = getCatmullMatrix();

   if(ncps >= 4) {
      //BUILD TABLE
      usTable.push_back(make_pair(0.0f, totalLen));
      for (int cp = 0; cp < ncps-3; cp++) {
         for (int idx = cp; idx < cp+4; idx++) {
            G(0, idx-cp) = cps[idx](0);
            G(1, idx-cp) = cps[idx](1);
            G(2, idx-cp) = cps[idx](2);
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

void Shape::fillCPs() {
   cps.clear();
   for (int i = 0; i < cbs.size(); i++) {
      cps.insert(cps.end(), cbs[i].cps.begin(), cbs[i].cps.end());
   }
   ncps = (int)cps.size();
}

Shape::ControlBox::ControlBox(Eigen::Vector3f center, Eigen::Vector3f dimensions, bool two) :
   pos(center),
   dimensions(dimensions)
{
   cps.push_back(center);
   if (two) {
      cps.push_back(center);
   }
}
Shape::ControlBox::~ControlBox() {}

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
