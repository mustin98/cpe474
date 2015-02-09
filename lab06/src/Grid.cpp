#ifdef __APPLE__
#include <GLUT/glut.h>
#endif
#ifdef __unix__
#include <GL/glut.h>
#endif
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include "Grid.h"

using namespace std;

Grid::Grid() :
	nrows(2),
	ncols(2),
	closest(-1)
{
	
}

Grid::~Grid()
{
	
}

void Grid::setSize(int nrows, int ncols)
{
	this->nrows = nrows;
	this->ncols = ncols;
	
	cps.resize(nrows*ncols);
	reset();
}

void Grid::reset()
{
	for(int col = 0; col < ncols; ++col) {
		float x = -1.0f + col/(ncols-1.0f)*2.0f;
		for(int row = 0; row < nrows; ++row) {
			float y = -1.0f + row/(nrows-1.0f)*2.0f;
			cps[indexAt(row, col)] << x, y;
		}
	}
}

int Grid::indexAt(int row, int col) const
{
	return row*ncols + col;
}

void Grid::moveCP(const Eigen::Vector2f &p)
{
	if(closest != -1) {
		cps[closest] = p;
	}
}

void Grid::findClosest(const Eigen::Vector2f &p)
{
	closest = -1;
	float dmin = 0.1f;
	for(int col = 0; col < ncols; ++col) {
		for(int row = 0; row < nrows; ++row) {
			Eigen::Vector2f cp = cps[indexAt(row, col)];
			float d = (p - cp).norm();
			if(d < dmin) {
				closest = indexAt(row, col);
				dmin = d;
			}
		}
	}
}

void Grid::draw() const
{
	// Draw closest control point
	glPointSize(6.0f);
	glColor3f(0.5f, 0.5f, 0.5f);
	if(closest != -1) {
		glBegin(GL_POINTS);
		Eigen::Vector2f cp = cps[closest];
		glVertex3f(cp(0), cp(1), 0.01f); // offset slightly above the shape
		glEnd();
	}
	// Draw grid
	glLineWidth(1.0f);
	for(int col = 0; col < ncols; ++col) {
		glBegin(GL_LINE_STRIP);
		for(int row = 0; row < nrows; ++row) {
			Eigen::Vector2f cp = cps[indexAt(row, col)];
			glVertex3f(cp(0), cp(1), 0.01f); // offset slightly above the shape
		}
		glEnd();
	}
	for(int row = 0; row < nrows; ++row) {
		glBegin(GL_LINE_STRIP);
		for(int col = 0; col < ncols; ++col) {
			Eigen::Vector2f cp = cps[indexAt(row, col)];
			glVertex3f(cp(0), cp(1), 0.01f); // offset slightly above the shape
		}
		glEnd();
	}
}

vector<Eigen::Vector2f> Grid::getTileCPs(int index) const
{
	// Return the 4 cps corresponding to the index
	// 2---3
	// |   |
	// 0---1
	assert(index >= 0);
	assert(index + ncols + 1 < cps.size());
	vector<Eigen::Vector2f> cps_;
	cps_.push_back(cps[index]);
	cps_.push_back(cps[index + 1]);
	cps_.push_back(cps[index + ncols]);
	cps_.push_back(cps[index + ncols + 1]);
	return cps_;
}

const vector<Eigen::Vector2f> & Grid::getAllCPs() const
{
	return cps;
}

void Grid::save(const char *filename) const
{
	ofstream out(filename);
	if(!out.good()) {
		cout << "Could not open " << filename << endl;
		return;
	}
	out << nrows << endl;
	out << ncols << endl;
	for(int k = 0; k < (int)cps.size(); ++k) {
		Eigen::Vector2f cp = cps[k];
		out << cp(0) << " " << cp(1) << endl;
	}
	out << "##################################" << endl;
	out.close();
}

void Grid::load(const char *filename)
{
	ifstream in;
	in.open(filename);
	if(!in.good()) {
		std::cout << "Cannot read " << filename << endl;
		return;
	}
	in >> nrows;
	in >> ncols;
	cps.resize(nrows*ncols);
	cps.clear();
	string line;
	// Discard rest of the first line.
	getline(in, line);
	while(1) {
		getline(in, line);
		if(in.eof()) {
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
		Eigen::Vector2f cp;
		ss >> cp(0);
		ss >> cp(1);
		cps.push_back(cp);
	}
	in.close();
}
