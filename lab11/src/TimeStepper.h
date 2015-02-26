#ifndef __L11__TimeStepper__
#define __L11__TimeStepper__

#include <stdio.h>
#include <vector>

class Particle;

class TimeStepper
{
public:
	TimeStepper();
	virtual ~TimeStepper();
	
	void step(std::vector<Particle*> &particles);
	void reset(std::vector<Particle*> &particles);
	
	double t;
	double h;
	int type;
};

#endif /* defined(__L11__TimeStepper__) */
