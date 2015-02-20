#ifdef __APPLE__
#include <GLUT/glut.h>
#endif
#ifdef __unix__
#include <GL/glut.h>
#endif
#include "HUD.h"
#include "MatrixStack.h"

int HUD::width = 1;
int HUD::height = 1;

void HUD::setWidthHeight(int w, int h)
{
	width = w;
	height = h;
}

void HUD::drawString(int x, int y, const char *str)
{
	MatrixStack P;
	P.ortho(0, width, 0, height, -1, 1);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadMatrixf(P.topMatrix().data());
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glRasterPos3f(x, y, 1.0);
	for(int i = 0; i < strlen(str); ++i) {
		glutBitmapCharacter(GLUT_BITMAP_8_BY_13, str[i]);
	}
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
}
