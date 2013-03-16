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

  dial( const meter, int dialN );
  void setDialNumber( int d );
  void setDial( vector<Mat> dialI );
  void test( void );
  void setDialImage( void );
  void setRReading( int rReading );
  void dialProcessing( void );
  void checkFailure( Point test, Mat img_scene );
  Point seekDial(Mat inputImg, int x, int y);
  Mat binarization( Mat inputImg );
  void dialReading( Mat inputImg );
  bool evenDialType( void );
  bool getFailure( void );
  int getReading( void );
  Mat complement( Mat inputImg );
  Mat filtering( Mat img_orig );


private:

  meter Meter;
  vector<Mat> Dial;
  int dialNumber;
  int reading;
  int rightReading;
  Mat imageDial;
  Mat imageDialColor;
  Point centroid;
  Point tip;
  bool failure;

};

#endif
