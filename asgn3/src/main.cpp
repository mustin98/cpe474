#ifdef __APPLE__
#include <GLUT/glut.h>
#endif
#ifdef __unix__
#include <GL/glut.h>
#endif
#ifdef _WIN32
#define GLFW_INCLUDE_GLCOREARB
#include <GL/glew.h>
#include <cstdlib>
#include <glut.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <Eigen/Dense>
#include "GLSL.h"
#include "Program.h"
#include "Camera.h"
#include "MatrixStack.h"
#include "Shape.h"
#include "Texture.h"
#include "Link.h"

#include "ceres/ceres.h"
#include "glog/logging.h"


using ceres::AutoDiffCostFunction;
using ceres::CostFunction;
using ceres::Problem;
using ceres::Solver;
using ceres::Solve;

using namespace std;

int width = 1;
int height = 1;
bool keyToggles[256] = {false};
Camera camera;
Program progSimple;
Program progTex;
Shape shape;
Link links[5];
Texture texture;
Eigen::Vector2f mouse(-1.0f, -1.0f);
int modifiers;
int IKType = 0;

struct CostFunctor {
   template <typename T> bool operator()(const T* const x, T* residual) const {
      Eigen::Matrix<T,4,4> transform;
      transform << T(1),T(0),T(0),T(0),
                   T(0),T(1),T(0),T(0),
                   T(0),T(0),T(1),T(0),
                   T(0),T(0),T(0),T(1);
      // 1 unit in x-pos
      Eigen::Matrix<T,4,4> trans;
      trans << T(1),T(0),T(0),T(1),
               T(0),T(1),T(0),T(0),
               T(0),T(0),T(1),T(0),
               T(0),T(0),T(0),T(1);
      // rotation about z-axis
      Eigen::Matrix<T,4,4> rot;
      for (int joint = 0; joint < 5; joint++) {
         rot << T(cos(x[joint])),T(-sin(x[joint])),T(0),T(0),
                T(sin(x[joint])),T(cos(x[joint])),T(0),T(0),
                T(0),T(0),T(1),T(0),
                T(0),T(0),T(0),T(1);
         if (joint == 0) {
            transform *= rot;
         } else {
            transform *= trans * rot;
         }
      }
      // One last translation to reach goal point
      transform *= trans;
      Eigen::Matrix<T,4,1> result;
      result << T(0),T(0),T(0),T(1);
      result = transform * result;

      residual[0] = T(mouse(0)) - result(0);
      residual[1] = T(mouse(1)) - result(1);
      return true;
   }
};

class AngleCostFunctor {
   public:
   AngleCostFunctor(int jointIdx, double restAngle) {
      alpha = 0.05;
      this->jointIdx = jointIdx;
      angle = restAngle;
   }
   double alpha;
   double angle;
   int jointIdx;

   template <typename T> bool operator()(const T* const x, T* residual) const {
      residual[0] = T(alpha) * (T(angle) - x[jointIdx]);
      return true;
   }
};

void solveJointAngles() {
   double x[5];
   for (int link = 0; link < 5; link++) {
      x[link] = links[link].angle;
   }

   Problem problem;
   CostFunction* cost_function =
    new AutoDiffCostFunction<CostFunctor, 2, 5>(new CostFunctor);
   problem.AddResidualBlock(cost_function, NULL, &x[0]);
   Solver::Options options;
   options.linear_solver_type = ceres::DENSE_QR;
   Solver::Summary summary;

   if (IKType > 0) {
      int neg = -1;
      for (int link = 1; link < 5; link++) {
         problem.AddResidualBlock(
          new AutoDiffCostFunction<AngleCostFunctor, 1, 5>(new AngleCostFunctor(link, IKType == 1 ? 0 : neg*180)),
          NULL, &x[0]);
         neg *= -1;
      }
   }
   Solve(options, &problem, &summary);

   for (int link = 0; link < 5; link++) {
      links[link].angle = x[link];
   }
}

void loadScene()
{
   camera.setTranslations(Eigen::Vector3f(0.0f, 0.0f, 1.0f));
   shape.loadMesh("link.obj");
   texture.setFilename("metal_texture_15_by_wojtar_stock.jpg");
   progSimple.setShaderNames("simple_vert.glsl", "simple_frag.glsl");
   progTex.setShaderNames("tex_vert.glsl", "tex_frag.glsl");
   links[0].init(&shape, NULL, 0);
   for (int link = 1; link < 5; link++) {
      links[link].init(&shape, &links[link-1], 1);
   }
}

void initGL()
{
   // Set background color
   glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
   // Disable z-buffer test
   glEnable(GL_DEPTH_TEST);
   // Antialiasing
   glEnable(GL_LINE_SMOOTH);
   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
   glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
   
   shape.init();
   
   texture.init();
   
   progSimple.init();
   progSimple.addUniform("P");
   progSimple.addUniform("MV");
   
   progTex.init();
   progTex.addUniform("P");
   progTex.addUniform("MV");
   progTex.addAttribute("vertPos");
   progTex.addAttribute("vertNor");
   progTex.addAttribute("vertTex");
   progTex.addUniform("texture0");
   
   GLSL::checkVersion();
}

void reshapeGL(int w, int h)
{
   width = w;
   height = h;
   glViewport(0, 0, (GLsizei)w, (GLsizei)h);
   float aspect = w/(float)h;
   float s = 5.2f;
   camera.setOrtho(-s*aspect, s*aspect, -s, s, 0.1f, 10.0f);
}

void drawGL()
{
   // Clear buffers
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   
   // Apply camera transforms
   MatrixStack P, MV;
   camera.applyProjectionMatrix(&P);
   camera.applyViewMatrix(&MV);
   
   // Draw grid
   progSimple.bind();
   glUniformMatrix4fv(progSimple.getUniform("P"), 1, GL_FALSE, P.topMatrix().data());
   glUniformMatrix4fv(progSimple.getUniform("MV"), 1, GL_FALSE, MV.topMatrix().data());
   glLineWidth(2.0f);
   float x0 = -5.0f;
   float x1 = 5.0f;
   float y0 = -5.0f;
   float y1 = 5.0f;
   glColor3f(0.2f, 0.2f, 0.2f);
   glBegin(GL_LINE_LOOP);
   glVertex2f(x0, y0);
   glVertex2f(x1, y0);
   glVertex2f(x1, y1);
   glVertex2f(x0, y1);
   glEnd();
   int gridSize = 10;
   glLineWidth(1.0f);
   glBegin(GL_LINES);
   for(int i = 1; i < gridSize; ++i) {
      if(i == gridSize/2) {
         glColor3f(0.2f, 0.2f, 0.2f);
      } else {
         glColor3f(0.8f, 0.8f, 0.8f);
      }
      float x = x0 + i / (float)gridSize * (x1 - x0);
      glVertex2f(x, y0);
      glVertex2f(x, y1);
   }
   for(int i = 1; i < gridSize; ++i) {
      if(i == gridSize/2) {
         glColor3f(0.2f, 0.2f, 0.2f);
      } else {
         glColor3f(0.8f, 0.8f, 0.8f);
      }
      float y = y0 + i / (float)gridSize * (y1 - y0);
      glVertex2f(x0, y);
      glVertex2f(x1, y);
   }
   glEnd();
   progSimple.unbind();
   
   // Draw shape
   progTex.bind();
   texture.bind(progTex.getUniform("texture0"), 0);
   glUniformMatrix4fv(progTex.getUniform("P"), 1, GL_FALSE, P.topMatrix().data());
   MV.pushMatrix();
   glUniformMatrix4fv(progTex.getUniform("MV"), 1, GL_FALSE, MV.topMatrix().data());
   links[0].draw(MV, &progTex);
   MV.popMatrix();
   texture.unbind();
   progTex.unbind();

   // Double buffer
   glutSwapBuffers();
}

void mouseGL(int button, int state, int x, int y)
{
   modifiers = glutGetModifiers();
}

Eigen::Vector2f window2world(int x, int y)
{
   // Convert from window coords to world coords
   // (Assumes orthographic projection)
   Eigen::Vector4f p;
   // Inverse viewing transform
   p(0) = x / (float)width;
   p(1) = (height - y) / (float)height;
   p(0) = 2.0f * (p(0) - 0.5f);
   p(1) = 2.0f * (p(1) - 0.5f);
   p(2) = 0.0f;
   p(3) = 1.0f;
   // Inverse model-view-projection transform
   MatrixStack P, MV;
   camera.applyProjectionMatrix(&P);
   camera.applyViewMatrix(&MV);
   p = (P.topMatrix() * MV.topMatrix()).inverse() * p;
   return p.segment<2>(0);
}

void mouseMotionGL(int x, int y)
{
   if(modifiers & GLUT_ACTIVE_ALT) {
      mouse = window2world(x, y);
      solveJointAngles();
      Eigen::Matrix4f transMat = links[0].endTransform(Eigen::Matrix4f::Identity());
      Eigen::Vector4f endEff = transMat * Eigen::Vector4f(0,0,0,1);
   }
}

void keyboardGL(unsigned char key, int x, int y)
{
   keyToggles[key] = !keyToggles[key];
   switch(key) {
      case 27:
         // ESCAPE
         exit(0);
         break;
      case 115:
         IKType = (IKType+1) % 3;
         break;
   }
}

void timerGL(int value)
{
   glutPostRedisplay();
   glutTimerFunc(20, timerGL, 0);
}

int main(int argc, char **argv)
{
   glutInit(&argc, argv);
   glutInitWindowSize(600, 600);
   glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
   glutCreateWindow("Justin Fujikawa - Assignment 3");
   glutMouseFunc(mouseGL);
   glutMotionFunc(mouseMotionGL);
   glutKeyboardFunc(keyboardGL);
   glutReshapeFunc(reshapeGL);
   glutDisplayFunc(drawGL);
   glutTimerFunc(20, timerGL, 0);
   loadScene();
   initGL();
   glutMainLoop();
   return 0;
}
