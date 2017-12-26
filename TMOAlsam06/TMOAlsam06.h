#include "TMO.h"
#include <iostream>
#include <math.h>



class TMOAlsam06 : public TMO  
{
public:
	TMOAlsam06();
	virtual ~TMOAlsam06();
	virtual int Transform();

protected:
	TMODouble dParameter;
};
