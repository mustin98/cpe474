#pragma once
#ifndef _SHAPEOBJ_H_
#define _SHAPEOBJ_H_

#include "tiny_obj_loader.h"

class ShapeObj
{
public:
	ShapeObj();
	virtual ~ShapeObj();
	void load(const std::string &meshName);
   void loadBones(const char* bonefile);
   void loadAnimation(const char* animationfile);
	void init();
	void animate(float t, float animationDuration, int h_m, int h_m0);
	void draw(int h_bon0, int h_bon1, int h_bon2, int h_bon3,
		       int h_wei0, int h_wei1, int h_wei2, int h_wei3,
		       int h_nBon, int h_pos, int h_nor, int h_tex) const;
	void setMaterial(int id, int h_dif, int h_spec, int h_amb, int h_shine);

private:
	int numVerts;
   int numBones;
   int numFrames;
   std::vector<float> posBuf;
   std::vector<float> norBuf;
   std::vector< std::vector<float> > weights0;
   std::vector<float> weightsBuf;
   std::vector< std::vector<float> > bones0;
   std::vector<float> bonesBuf;
   std::vector<float> numBonesBuf;
   std::vector<float> aFrames;
   std::vector<float> bindPose;
   int matID;

	std::vector<tinyobj::shape_t> shapes;
	unsigned posBufID;
	unsigned norBufID;
	unsigned texBufID;
	unsigned indBufID;
	unsigned bonesBufID;
	unsigned weightsBufID;
	unsigned numBonesBufID;
};

#endif
