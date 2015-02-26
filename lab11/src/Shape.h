#pragma once
#ifndef _SHAPE_H_
#define _SHAPE_H_

#include <vector>

class Program;

class Shape
{
public:
	Shape();
	virtual ~Shape();
	void loadMesh(const std::string &meshName);
	void init();
	void draw(const Program &p) const;
	
private:
	std::vector<unsigned int> eleBuf;
	std::vector<float> posBuf;
	std::vector<float> norBuf;
	std::vector<float> texBuf;
	unsigned eleBufID;
	unsigned posBufID;
	unsigned norBufID;
	unsigned texBufID;
};

#endif
