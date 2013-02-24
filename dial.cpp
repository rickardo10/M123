#include <iostream>
#include <stdio.h>
#include <opencv2/core/core.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <vector>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include "dial.h"
#include "meter.h"

using namespace std;
using namespace cv;

dial::dial( const meter rDial, int dialNumber )
: Meter( rDial )
{
  setDial( Meter.getDials() );
  setDialNumber( dialNumber );
}

void dial::setDial( vector<Mat> dialI )
{
  Dial = dialI;
}

void dial::setDialNumber( int d )
{
  dialNumber = d;
}

void dial::test( void )
{
  imshow("test", Dial[1] );
  waitKey(0);
}



