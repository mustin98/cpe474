#pragma once
#ifndef __A1__HUD__
#define __A1__HUD__

#include <stdio.h>

class HUD
{
private:
	HUD() {}
	
public:
	static void setWidthHeight(int w, int h);
	static int getWidth() { return width; }
	static int getHeight() { return height; };
	static void drawString(int x, int y, const char *str);
	
	static int width;
	static int height;
};

#endif /* defined(__A1__HUD__) */
