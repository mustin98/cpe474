#include <iostream>
#include <Eigen/Dense>
#include "ShapeObj.h"
#include "GLSL.h"
#include "Grid.h"

using namespace std;

ShapeObj::ShapeObj() :
	tileIndexBufID(0),
	posLocalBufID(0),
	texBufID(0),
	indBufID(0)
{
}

ShapeObj::~ShapeObj()
{
}

void ShapeObj::load(const string &meshName)
{
	// Load geometry
	// Some obj files contain material information.
	// We'll ignore them for this assignment.
	vector<tinyobj::shape_t> shapes;
	vector<tinyobj::material_t> objMaterials;
	string err = tinyobj::LoadObj(shapes, objMaterials, meshName.c_str());
	if(!err.empty()) {
		cerr << err << endl;
	} else {
		posBuf = shapes[0].mesh.positions;
		norBuf = shapes[0].mesh.normals;
		texBuf = shapes[0].mesh.texcoords;
		indBuf = shapes[0].mesh.indices;
	}
}

void ShapeObj::toLocal()
{
	// Find which tile each vertex belongs to.
	// Store (row, col) into tileBuf.
	int nVerts = (int)posBuf.size()/3;
	posLocalBuf.resize(nVerts*2);
	tileIndexBuf.resize(nVerts);
	int nrows = grid->getRows();
	int ncols = grid->getCols();
	

	for (int col = 0; col < ncols-1; col++) {
		for (int row = 0; row < nrows-1; row++) {
			int tileIndex = grid->indexAt(row, col);
			vector<Eigen::Vector2f> cps = grid->getTileCPs(tileIndex);
			float minX = cps[0](0), maxX = cps[1](0);
			float minY = cps[0](1), maxY = cps[2](1);
			for(int k = 0; k < nVerts; ++k) {
				float x = posBuf[3*k+0];
				float y = posBuf[3*k+1];
				if (x >= minX && x <= maxX && y >= minY && y <= maxY) {
					x = (x - minX) / (maxX - minX);
					y = (y - minY) / (maxY - minY);
					posLocalBuf[2*k+0] = x;
					posLocalBuf[2*k+1] = y;	
					tileIndexBuf[k] = tileIndex;
				}
			}
		}
	}
}

/*void ShapeObj::toWorld()
{
	int nVerts = (int)posBuf.size()/3;
	for(int k = 0; k < nVerts; ++k) {
		vector<Eigen::Vector2f> cps = grid->getTileCPs(tileIndexBuf[k]);
		float u = posLocalBuf[2*k+0];
		float v = posLocalBuf[2*k+1];
		Eigen::Vector2f p = (1-v) * ((1-u)*cps[0]+u*cps[1]) + v * ((1-u)*cps[2]+u*cps[3]);
		posBuf[3*k+0] = p(0);
		posBuf[3*k+1] = p(1);
	}
	// Send updated position array to the GPU
	glBindBuffer(GL_ARRAY_BUFFER, posBufID);
	glBufferData(GL_ARRAY_BUFFER, posBuf.size()*sizeof(float), &posBuf[0], GL_DYNAMIC_DRAW);
}*/

void ShapeObj::init()
{
	// Send the position array to the GPU
	glGenBuffers(1, &tileIndexBufID);
	glBindBuffer(GL_ARRAY_BUFFER, tileIndexBufID);
	glBufferData(GL_ARRAY_BUFFER, tileIndexBuf.size()*sizeof(float), &tileIndexBuf[0], GL_DYNAMIC_DRAW);
	
	// Send the position array to the GPU
	glGenBuffers(1, &posLocalBufID);
	glBindBuffer(GL_ARRAY_BUFFER, posLocalBufID);
	glBufferData(GL_ARRAY_BUFFER, posLocalBuf.size()*sizeof(float), &posLocalBuf[0], GL_DYNAMIC_DRAW);
	
	// Send the texture coordinates array (if it exists) to the GPU
	if(!texBuf.empty()) {
		glGenBuffers(1, &texBufID);
		glBindBuffer(GL_ARRAY_BUFFER, texBufID);
		glBufferData(GL_ARRAY_BUFFER, texBuf.size()*sizeof(float), &texBuf[0], GL_STATIC_DRAW);
	} else {
		texBufID = 0;
	}
	
	// Send the index array to the GPU
	glGenBuffers(1, &indBufID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indBufID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indBuf.size()*sizeof(unsigned int), &indBuf[0], GL_STATIC_DRAW);
	
	// Unbind the arrays
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	
	assert(glGetError() == GL_NO_ERROR);
}

void ShapeObj::draw(int h_tile, int h_pos, int h_tex) const
{
	GLSL::enableVertexAttribArray(h_tile);
	glBindBuffer(GL_ARRAY_BUFFER, tileIndexBufID);
	glVertexAttribPointer(h_tile, 1, GL_FLOAT, GL_FALSE, 0, 0);

	// Enable and bind position array for drawing
	GLSL::enableVertexAttribArray(h_pos);
	glBindBuffer(GL_ARRAY_BUFFER, posLocalBufID);
	glVertexAttribPointer(h_pos, 2, GL_FLOAT, GL_FALSE, 0, 0);
	
	// Enable and bind texcoord array for drawing
	GLSL::enableVertexAttribArray(h_tex);
	glBindBuffer(GL_ARRAY_BUFFER, texBufID);
	glVertexAttribPointer(h_tex, 2, GL_FLOAT, GL_FALSE, 0, 0);
	
	// Bind index array for drawing
	int nIndices = (int)indBuf.size();
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indBufID);
	
	// Draw
	glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, 0);
	
	// Disable and unbind
	GLSL::disableVertexAttribArray(h_tex);
	GLSL::disableVertexAttribArray(h_pos);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
