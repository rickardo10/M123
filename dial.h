//dial.hpp
//printImage Class Definition
//Member functions defined in printImage.h

#ifndef DIAL_H
#define DIAL_H

#include "meter.h"
using namespace std;
using namespace cv;

class dial
{
public:

  dial( const meter, int );
  void setDialNumber( int d );
  void setDial( vector<Mat> dialI );
  void test( void );

private:

  meter Meter;
  vector<Mat> Dial;
  int dialNumber;

};

#endif
