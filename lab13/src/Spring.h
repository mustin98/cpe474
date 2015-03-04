#pragma once
#ifndef __L12__Spring__
#define __L12__Spring__

#include <stdio.h>

class Particle;

class Spring
{
public:
	Spring(Particle *p0, Particle *p1);
	virtual ~Spring();
	
	Particle *p0;
	Particle *p1;
	double K;
	double L;
};

#endif /* defined(__L12__Spring__) */
