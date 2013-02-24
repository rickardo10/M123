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

///--Constructor
dial::dial( const meter rMeter, int dialNumber )
: Meter( rMeter )
{
  setDial( Meter.getDials() );
  setDialNumber( dialNumber );
  setDialImage();
  dialProcessing();
}

///--Reads the vector with all the dials
void dial::setDial( vector<Mat> dialI )
{
  Dial = dialI;
}

///--Reads the dial to be analyzed
void dial::setDialNumber( int d )
{
  dialNumber = d;
}

///--Sets the dial
void dial::setDialImage( void )
{
  imageDial = Dial[ dialNumber ];
  imageDialColor = Dial[ dialNumber + 5 ];
}

///--Gets the reading
int dial::getReading( void )
{
  return reading;
}

///--Process the image
void dial::dialProcessing( void )
{
  //--Threshold the dial
  Mat img_thrshld;
  threshold( imageDial, img_thrshld, 0, 255,  THRESH_OTSU );

  //--Creates an Structuring Element in order to apply morphology
  Mat element = getStructuringElement(MORPH_CROSS,
                        Size(4, 4));

  Mat element2 = getStructuringElement(MORPH_CROSS,
                        Size(2, 2));

  //--Applies an opening operation to the image
  Mat img_dil;
  dilate( img_thrshld, img_dil, element, Point( 1, 1 ), 3 );

  Mat img_ero;
  erode( img_dil, img_ero, element2, Point( 1, 1 ), 4 );

  //--Seeks dial place
  Point Black = seekDial( img_ero, img_ero.cols / 2, img_ero.rows / 2 );

  //--Checks if point is inside the image
  checkFailure( Black, img_ero );
  if( failure ){
    printf("Failure: fails floodfill in dial %d ", dialNumber );
    return;
  }

  //--Filters image
  try{
    floodFill( img_ero, Black, 150);
  }
  catch( cv::Exception ){
    printf("Failure: floodfill\n");
    return;
  }

  //--Binarizes image
  Mat img_bin = binarization( img_ero );

  //--Reads dial
  dialReading( img_ero );
}

///--Seeks dial place
Point dial::seekDial(Mat inputImg, int x, int y)
{
  while(inputImg.at<uchar>(x, y) == 255){
    y += 1;
  }
  Point black(y,x);
  return black;
}

///--Binarize Image
Mat dial::binarization( Mat inputImg ){

  for( int i = 0; i < inputImg.rows; i++ ){
    for( int j = 0; j < inputImg.cols; j++ ){
      if( inputImg.at<uchar>(i, j) == 150 ){
        inputImg.at<uchar>(i, j) = 0;
      } else {
        inputImg.at<uchar>(i, j) = 255;
      }
    }
  }

  return inputImg;
}

///--Reads dial
void dial::dialReading( Mat inputImg ){
  double x, y;
  double pi = 3.1415926535;

  //--Calculates moments and finds centroid
  Moments moments1 =  moments( inputImg, true );
  x = moments1.m10 / moments1.m00;
  y = moments1.m01 / moments1.m00;

  //--Finds the dial's tip
  double euclideanDistance = 0, maxED = 0;
  int x1 = 0, y1 = 0;

  for( int i = 0; i < inputImg.rows; i++ ){
    for( int j = 0; j < inputImg.cols; j++ ){

      if( inputImg.at<uchar>( Point( j, i ) ) == 0 ){
        euclideanDistance = sqrt( pow( ( j - x ), 2 ) + pow( ( i - y ), 2 ) );
        if( euclideanDistance > maxED ){
          maxED = euclideanDistance;
          x1 = j;
          y1 = i;
        }
      }

    }
  }

  //--Creates an array with angle intervals
  double numbers[ 6 ] = { pi / 2, 3 * pi / 10, pi / 10, - pi / 10, - 3 * pi / 10, - pi / 2 };

  //--Calculates hypotenuse
  double hyp = sqrt( pow( ( x1 - x ), 2 ) + pow( ( y - y1 ), 2 ) );

  //--Calculates the angle using atan, asin, acos
  double teta1 = atan( (double)( y - y1 ) / ( x1 - x ) );
  double teta2 = asin( (double)( y - y1 ) / hyp );
  double teta3 = acos( (double)( x1 - x ) / hyp );

  reading = 0;

  //--Verifies in which interval the actual angle fits and saves the reading
  for( int i = 0; i <= 5; i ++){
    if( teta1 <= numbers[ i ] && teta1 > numbers[ i + 1 ]){
      if( teta1 > 0 && teta2 < 0 ){
        reading = i + 5;
        continue;
      }
      if( teta1 < 0 && teta2 > 0){
        reading = i + 5;
        continue;
      }
      reading = i;
      continue;
    }
  }

  //--If the dial is even then it's reading is differnt
  if( evenDialType() ){
    reading = 9 - reading;
  }

  //--Saves centroid and tip points
  centroid = Point( x1, y1 );
  tip = Point( x, y );
}

///--Returns true if dialNumber is even
bool dial::evenDialType( void )
{
  if( dialNumber == 1 || dialNumber == 3 ) {
    return true;
  }
  else {
    return false;
  }
}

///--Returns variable failure
bool dial::getFailure()
{
  return failure;
}

///--Checks if segmentation is good
void dial::checkFailure( Point test, Mat img_scene )
{
  failure = false;
  //--Checks if points are inside the image
  if(test.x > img_scene.cols || test.x < 0 ){
    failure = true;
    return;
  }

  //--Checks if points are inside the image
  if(test.y > img_scene.rows || test.y < 0 ){
    failure = true;
    return;
  }
}

