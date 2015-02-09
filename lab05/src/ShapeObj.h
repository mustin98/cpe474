#pragma once
#ifndef _SHAPEOBJ_H_
#define _SHAPEOBJ_H_

#include "tiny_obj_loader.h"

class Grid;

class ShapeObj
{
public:
	ShapeObj();
	virtual ~ShapeObj();
	void load(const std::string &meshName);
	void setGrid(Grid *g) { grid = g; }
	void toLocal();
	void toWorld();
	void init();
	void draw(int h_pos, int h_tex) const;
	
private:
	std::vector<unsigned int> indBuf;
	std::vector<float> posBuf;
	std::vector<float> norBuf;
	std::vector<float> texBuf;
	std::vector<float> posLocalBuf;
	std::vector<float> tileIndexBuf;
	unsigned posBufID;
	unsigned texBufID;
	unsigned indBufID;
	Grid *grid;
};

#endif
