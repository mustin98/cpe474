#include <iostream>
#include <fstream>
#include <Eigen/Dense>
#include "ShapeObj.h"
#include "GLSL.h"

using namespace std;

ShapeObj::ShapeObj() :
	posBufID(0),
	norBufID(0),
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
	vector<tinyobj::material_t> objMaterials;
	string err = tinyobj::LoadObj(shapes, objMaterials, meshName.c_str());
	if(!err.empty()) {
		cerr << err << endl;
	}
}

void ShapeObj::loadBones(const char* bonefile) {
   ifstream in;
   in.open(bonefile);
   if (!in.good()) {
      cout << "Cannot read " << bonefile << endl;
      return;
   }
   string line;
   getline(in, line);
   getline(in, line);
   in >> numVerts;
   in >> numBones;
   bones0.resize(numVerts);
   weights0.resize(numVerts);
   bonesBuf.resize(numVerts * 16);
   weightsBuf.resize(numVerts * 16);
   int vert = 0;
   int bone = 0;

   while(1) {
      getline(in, line);
      if (in.eof()) {
         break;
      }
      // Skip empty lines
      if(line.size() < 2) {
         continue;
      }
      // Skip comments
      if(line.at(0) == '#') {
         continue;
      }
      // Parse line
      float weight;
      stringstream ss(line);
      for (bone = 0; bone < numBones; bone++) {
         ss >> weight;
         if (weight > 0.0) {
            weights0[vert].push_back(weight);
            bones0[vert].push_back(bone);
         }
      }
      vert++;
   }
   // Fill in bones buffer with trailing 0s using bones0 array
   for (vert = 0; vert < numVerts; vert++) {
   	numBonesBuf.push_back(bones0[vert].size());
   	for (bone = 0; bone < numBonesBuf[vert]; bone++) {
   		bonesBuf[vert*16+ bone] = bones0[vert][bone];
         weightsBuf[vert*16+ bone] = weights0[vert][bone];
   	}
   	for (; bone < 16; bone++) {
   		bonesBuf[vert*16+ bone] = 0;
   		weightsBuf[vert*16+ bone] = 0;
   	}
   }
   in.close();
}

void ShapeObj::loadAnimation(const char* animationfile) {
   ifstream in;
   in.open(animationfile);
   if (!in.good()) {
      cout << "Cannot read " << animationfile << endl;
      return;
   }
   string line;
   getline(in, line);
   getline(in, line);
   getline(in, line);
   in >> numFrames;
   in >> numBones;
   float qx, qy, qz, qw, px, py, pz;
   for (int bones = 0; bones < numBones; bones++) {
      in >> qx;
      in >> qy;
      in >> qz;
      in >> qw;
      in >> px;
      in >> py;
      in >> pz;
      bindPose.push_back(qx);
      bindPose.push_back(qy);
      bindPose.push_back(qz);
      bindPose.push_back(qw);
      bindPose.push_back(px);
      bindPose.push_back(py);
      bindPose.push_back(pz);
   }

   while(1) {
      getline(in, line);
      if (in.eof()) {
         break;
      }
      // Skip empty lines
      if(line.size() < 2) {
         continue;
      }
      // Skip comments
      if(line.at(0) == '#') {
         continue;
      }
      // Parse line
      stringstream ss(line);
      // First line is bind pose
      for (int bones = 0; bones < numBones; bones++) {
         ss >> qx;
         ss >> qy;
         ss >> qz;
         ss >> qw;
         ss >> px;
         ss >> py;
         ss >> pz;
         aFrames.push_back(qx);
         aFrames.push_back(qy);
         aFrames.push_back(qz);
         aFrames.push_back(qw);
         aFrames.push_back(px);
         aFrames.push_back(py);
         aFrames.push_back(pz);
      }
   }
   in.close();
}

void ShapeObj::init()
{
	// Send list of bones per vertex
	glGenBuffers(1, &bonesBufID);
	glBindBuffer(GL_ARRAY_BUFFER, bonesBufID);
	glBufferData(GL_ARRAY_BUFFER, bonesBuf.size()*sizeof(float), &bonesBuf[0], GL_STATIC_DRAW);
	
	// Send list of weights per bone per vertex
	glGenBuffers(1, &weightsBufID);
	glBindBuffer(GL_ARRAY_BUFFER, weightsBufID);
	glBufferData(GL_ARRAY_BUFFER, weightsBuf.size()*sizeof(float), &weightsBuf[0], GL_STATIC_DRAW);

	// Send list of number of bones affecting each vertex
	glGenBuffers(1, &numBonesBufID);
	glBindBuffer(GL_ARRAY_BUFFER, numBonesBufID);
	glBufferData(GL_ARRAY_BUFFER, numBonesBuf.size()*sizeof(float), &numBonesBuf[0], GL_STATIC_DRAW);

	// Send the position array to the GPU
	posBuf = shapes[0].mesh.positions;
	glGenBuffers(1, &posBufID);
	glBindBuffer(GL_ARRAY_BUFFER, posBufID);
	glBufferData(GL_ARRAY_BUFFER, posBuf.size()*sizeof(float), &posBuf[0], GL_STATIC_DRAW);
	
	// Send the normal array (if it exists) to the GPU
	norBuf = shapes[0].mesh.normals;
	if(!norBuf.empty()) {
		glGenBuffers(1, &norBufID);
		glBindBuffer(GL_ARRAY_BUFFER, norBufID);
		glBufferData(GL_ARRAY_BUFFER, norBuf.size()*sizeof(float), &norBuf[0], GL_STATIC_DRAW);
	} else {
		norBufID = 0;
	}
	
	// Send the texture coordinates array (if it exists) to the GPU
	const vector<float> &texBuf = shapes[0].mesh.texcoords;
	if(!texBuf.empty()) {
		glGenBuffers(1, &texBufID);
		glBindBuffer(GL_ARRAY_BUFFER, texBufID);
		glBufferData(GL_ARRAY_BUFFER, texBuf.size()*sizeof(float), &texBuf[0], GL_STATIC_DRAW);
	} else {
		texBufID = 0;
	}
	
	// Send the index array to the GPU
	const vector<unsigned int> &indBuf = shapes[0].mesh.indices;
	glGenBuffers(1, &indBufID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indBufID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indBuf.size()*sizeof(unsigned int), &indBuf[0], GL_STATIC_DRAW);
	
	// Unbind the arrays
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	
	assert(glGetError() == GL_NO_ERROR);
}


void ShapeObj::draw(int h_bon0, int h_bon1, int h_bon2, int h_bon3,
	                 int h_wei0, int h_wei1, int h_wei2, int h_wei3,
	                 int h_nBon, int h_pos, int h_nor, int h_tex) const
{
	unsigned stride = 16*sizeof(float);

	// bones
	GLSL::enableVertexAttribArray(h_bon0);
	GLSL::enableVertexAttribArray(h_bon1);
	GLSL::enableVertexAttribArray(h_bon2);
	GLSL::enableVertexAttribArray(h_bon3);
	glBindBuffer(GL_ARRAY_BUFFER, bonesBufID);
	glVertexAttribPointer(h_bon0, 4, GL_FLOAT, GL_FALSE, stride, (const void *)( 0*sizeof(float)));
	glVertexAttribPointer(h_bon1, 4, GL_FLOAT, GL_FALSE, stride, (const void *)( 4*sizeof(float)));
	glVertexAttribPointer(h_bon2, 4, GL_FLOAT, GL_FALSE, stride, (const void *)( 8*sizeof(float)));
	glVertexAttribPointer(h_bon3, 4, GL_FLOAT, GL_FALSE, stride, (const void *)(12*sizeof(float)));

	// weights
	GLSL::enableVertexAttribArray(h_wei0);
	GLSL::enableVertexAttribArray(h_wei1);
	GLSL::enableVertexAttribArray(h_wei2);
	GLSL::enableVertexAttribArray(h_wei3);
	glBindBuffer(GL_ARRAY_BUFFER, weightsBufID);
	glVertexAttribPointer(h_wei0, 4, GL_FLOAT, GL_FALSE, stride, (const void *)( 0*sizeof(float)));
	glVertexAttribPointer(h_wei1, 4, GL_FLOAT, GL_FALSE, stride, (const void *)( 4*sizeof(float)));
	glVertexAttribPointer(h_wei2, 4, GL_FLOAT, GL_FALSE, stride, (const void *)( 8*sizeof(float)));
	glVertexAttribPointer(h_wei3, 4, GL_FLOAT, GL_FALSE, stride, (const void *)(12*sizeof(float)));

	GLSL::enableVertexAttribArray(h_nBon);
	glBindBuffer(GL_ARRAY_BUFFER, numBonesBufID);
	glVertexAttribPointer(h_nBon, 1, GL_FLOAT, GL_FALSE, 0, 0);

	// Enable and bind position array for drawing
	GLSL::enableVertexAttribArray(h_pos);
	glBindBuffer(GL_ARRAY_BUFFER, posBufID);
	glVertexAttribPointer(h_pos, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Enable and bind normal array (if it exists) for drawing
	if(norBufID && h_nor >= 0) {
		GLSL::enableVertexAttribArray(h_nor);
		glBindBuffer(GL_ARRAY_BUFFER, norBufID);
		glVertexAttribPointer(h_nor, 3, GL_FLOAT, GL_FALSE, 0, 0);
	}
	
	// Enable and bind texcoord array (if it exists) for drawing
	if(texBufID && h_tex >= 0) {
		GLSL::enableVertexAttribArray(h_tex);
		glBindBuffer(GL_ARRAY_BUFFER, texBufID);
		glVertexAttribPointer(h_tex, 2, GL_FLOAT, GL_FALSE, 0, 0);
	}
	
	// Bind index array for drawing
	int nIndices = (int)shapes[0].mesh.indices.size();
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indBufID);

	// Draw
	glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, 0);
	
	// Disable and unbind
	if(texBufID && h_tex >= 0) {
		GLSL::disableVertexAttribArray(h_tex);
	}
	if(norBufID && h_nor >= 0) {
		GLSL::disableVertexAttribArray(h_nor);
	}
	GLSL::disableVertexAttribArray(h_pos);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void ShapeObj::animate(float t, float animationDuration, int h_m, int h_m0) {
	int k = floor(fmod(t, animationDuration) / animationDuration * numFrames);

   Eigen::Quaternionf q;
   Eigen::Vector3f p;
   vector<Eigen::Matrix4f> MArr;  // animation matrix
   vector<Eigen::Matrix4f> M0Arr; // bind matrix

   for (int j = 0; j < 18; j++) {
   	Eigen::Matrix4f M;
      q.vec() << aFrames[k*numBones*7 + j*7 + 0], aFrames[k*numBones*7 + j*7 + 1], aFrames[k*numBones*7 + j*7 + 2];
      q.w() = aFrames[k*numBones*7 + j*7 + 3];
      p << aFrames[k*numBones*7 + j*7 + 4], aFrames[k*numBones*7 + j*7 + 5], aFrames[k*numBones*7 + j*7 + 6];
      M = Eigen::Matrix4f::Identity();
      q.normalize();
      M.block<3,3>(0,0) = q.toRotationMatrix();
      M.block<3,1>(0,3) = p;
      MArr.push_back(M);

   	Eigen::Matrix4f M0;      
      q.vec() << bindPose[j*7 + 0], bindPose[j*7 + 1], bindPose[j*7 + 2];
      q.w() = bindPose[j*7 + 3];
      p << bindPose[j*7 + 4], bindPose[j*7 + 5], bindPose[j*7 + 6];
      M0 = Eigen::Matrix4f::Identity();
      q.normalize();
      M0.block<3,3>(0,0) = q.toRotationMatrix();
      M0.block<3,1>(0,3) = p;
      M0Arr.push_back(M0.inverse());
   }
	glUniformMatrix4fv(h_m, MArr.size(), GL_FALSE, MArr[0].data());
   glUniformMatrix4fv(h_m0, M0Arr.size(), GL_FALSE, M0Arr[0].data());
}

void ShapeObj::setMaterial(int id, int h_dif, int h_spec, int h_amb, int h_shine) {
   matID = id;
   switch (matID) {
      case 0: // shiny blue plastic
         glUniform3f(h_dif, 0.0, 0.08, 0.5);
         glUniform3f(h_spec, 0.14, 0.14, 0.4);
         glUniform3f(h_amb, 0.02, 0.02, 0.1);
         glUniform1f(h_shine, 120.0f);
         break;
      case 1: // shiny green plastic
         glUniform3f(h_dif, 0.0, 0.5, 0.08);
         glUniform3f(h_spec, 0.14, 0.4, 0.14);
         glUniform3f(h_amb, 0.02, 0.1, 0.02);
         glUniform1f(h_shine, 120.0f);
         break;
      case 2: // dull grey
         glUniform3f(h_dif, 0.3, 0.3, 0.4);
         glUniform3f(h_spec, 0.3, 0.3, 0.4);
         glUniform3f(h_amb, 0.13, 0.13, 0.14);
         glUniform1f(h_shine, 4.0f);
         break;
   }
}
