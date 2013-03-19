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
dial::dial( const meter rMeter, int dialN, int rReading )
: Meter( rMeter )
{
  setDial( Meter.getDials() );
  setDialNumber( dialN );
  setDialImage();
  setRReading( rReading );
  dialProcessing();
}

///--Reads the vector with all the dials
void dial::setDial( vector<Mat> dialI )
{
  Dial = dialI;
}

///--Reads the reading of the next dial
void dial::setRReading( int rReading )
{
   rightReading = rReading;
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
                        Size(3, 3));

  Mat element2 = getStructuringElement(MORPH_CROSS,
                        Size(3, 3));

  Mat original = complement( img_thrshld );
  //--Applies an opening operation to the image
  Mat img_ero;
  erode( original, img_ero, element, Point( 1, 1 ), 2 );

  img_ero = filtering( img_ero );

//  imshow( "Filtered", img_ero );

  Mat img_dil;
  dilate( img_ero, img_dil, element2, Point( 1, 1 ), 6 );

  Mat img_ero2;
  erode( img_dil, img_ero2, element, Point( 1, 1 ), 10 );

  img_ero2 = filtering( img_ero2 );

//  imshow( "Original", img_thrshld);
//  imshow( "Erosion", img_ero );
//  imshow( "Erosion2", img_ero2 );
//  imshow( "Dilate", img_dil );
//  moveWindow( "Erosion", 200, 200 );
//  moveWindow( "Original", 0, 200 );
//  moveWindow( "Dilate", 400, 200 );
//  moveWindow( "Filtered", 600, 200 );
//  moveWindow( "Erosion2", 800, 200 );
//  waitKey(0);

  //--Reads dial
  dialReading( img_ero2 );
}

///--Seeks dial place
Point dial::seekDial(Mat inputImg, int x, int y)
{
  while(inputImg.at<uchar>(x, y) == 0){
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
        inputImg.at<uchar>(i, j) = 255;
      } else {
        inputImg.at<uchar>(i, j) = 0;
      }
    }
  }

  return inputImg;
}

///--Reads dial
void dial::dialReading( Mat inputImg ){
  double x, y;
  double pi = 3.1415926535;
  int cpyReading = 0;

  //--Calculates moments and finds centroid
  Moments moments1 =  moments( inputImg, true );
  x = moments1.m10 / moments1.m00;
  y = moments1.m01 / moments1.m00;

  //--Finds the dial's tip
  double euclideanDistance = 0, maxED = 0;
  int x1 = 0, y1 = 0;

  for( int i = 0; i < inputImg.rows; i++ ){
    for( int j = 0; j < inputImg.cols; j++ ){

      if( inputImg.at<uchar>( Point( j, i ) ) == 255 ){
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

   //--Adds the image inclination
   teta1 += -Meter.getAngle();
   reading = -1;

  //--Verifies in which interval the actual angle fits and saves the reading
  for( int i = 0; i <= 5; i ++){
    //--Checks if the angle is in the current interval
    if( teta1 <= numbers[ i ] && teta1 > numbers[ i + 1 ] ){
      //--If tan is positive and sin es negative then the dial is pointing at the other
      //half of the circle
      if( teta1 > 0 && teta2 < 0 ){
        //--Creates a copy of variable reading before it modifies it
        cpyReading = i;
        reading = i + 5;
        break;
      }

      //--If tan is negative and sin is positive then the dial is pointing at the other
      //half of the circle
      if( teta1 < 0 && teta2 > 0){
         //--Creates a copy of variable reading before it modifies it
        cpyReading = i;
        reading = i + 5;
        break;
      }
      //--Creates a copy of variable reading before it modifies it
      cpyReading = i;
      reading = i;
      break;
    }
  }

  //--If the dial is even then its reading is differnt
  if( evenDialType() ){
    reading = 9 - reading;
  }

  //--Checks if the dial reads counterclockwise and if it is near the previous interval
  if( evenDialType() )
  {
    if( teta1  < ( numbers[ cpyReading ] ) && teta1 > ( numbers[ cpyReading  ] ) - pi / 25 )
    {
      if( rightReading < 5 )
      {
         reading += 1;
      }
    }
  }

  //--Checks if the dial reads clockwise and if it is near the previous interval
  if( !evenDialType() )
  {
    if( teta1  < ( numbers[ cpyReading + 1 ] + pi / 25 ) && teta1 > ( numbers[ cpyReading + 1 ] ) )
    {
      if( rightReading < 5 )
      {
         reading += 1;
      }
    }
  }

  //--Checks if dial is equal to 10, if it is, it will change it to 0
  if( reading == 10 ){
    reading = 0;
  }

  //--Saves centroid and tip points
  centroid = Point( x1, y1 );
  tip = Point( x, y );

  Mat img_new( imageDialColor);

  circle( img_new, centroid, 4, Scalar( 0, 0, 255 ), 1 );
  circle( img_new, tip, 4, Scalar( 0, 0, 255 ), 1  );
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

///--Complement of Image
Mat dial::complement( Mat inputImg ){

  for( int i = 0; i < inputImg.rows; i++ ){
    for( int j = 0; j < inputImg.cols; j++ ){
      if( inputImg.at<uchar>(i, j) == 255 ){
        inputImg.at<uchar>(i, j) = 0;
      } else {
        inputImg.at<uchar>(i, j) = 255;
      }
    }
  }

  return inputImg;
}

///Filters an image in order to erase all the noise
Mat dial::filtering( Mat img_orig )
{
    //--Seeks dial place
    Point Black = seekDial( img_orig, img_orig.cols / 2, img_orig.rows / 2 );

    //--Checks if point is inside the image
    checkFailure( Black, img_orig );
    if( failure ){
      printf("Failure: fails floodfill in dial %d ", dialNumber );
      return img_orig;
    }

    //--Filters image
    try{
      floodFill( img_orig, Black, 150);
    }
    catch( cv::Exception ){
      printf("Failure: floodfill\n");
      return img_orig;
    }

    img_orig = binarization( img_orig );

    return img_orig;
}
