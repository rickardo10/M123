#include <stdio.h>
#include <opencv2/core/core.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <vector>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include "descriptors.h"
#include <iostream>

using namespace std;
using namespace cv;

///--Constructor function to initialize private data
descriptors::descriptors( string img_scene, string name_feature, string name_descriptor,
                          string name_matcher, string img_object )
{
  setObject( img_scene, name_feature, name_descriptor, name_matcher, img_object );
}

///--Set object, feature, descriptor, matcher and scene
void descriptors::setObject( string sceneName, string featureDetName, string findDescriptorsName,
          string matcherName, string objectName )
{
  setImageObject( objectName );
  setImageScene( sceneName );
  setFeatureDetector( featureDetName );
  setFindDescriptors( findDescriptorsName );
  setMatcher( matcherName );
  featureDetector();
}

///--Reads object
void descriptors::setImageObject( const string name_object )
{
  //--Reads image
	img_object = imread( name_object, 0);
}

///--Reads Scene
void descriptors::setImageScene( const string name_scene )
{
  //--Reads image
	img_scene = imread( name_scene, 0);
}

///--Reads feature detector
void descriptors::setFeatureDetector( const string name_featureD )
{
   name_feature = name_featureD;
}

///--Reads descriptor finder
void descriptors::setFindDescriptors( const string name_Fdescriptor )
{
   name_descriptor = name_Fdescriptor;
}

///--Reads matcher
void descriptors::setMatcher( const string matcherN )
{
   name_matcher = matcherN;
}

///--Process available data in order to returns dials
void descriptors::featureDetector( void )
{
  //--Checks if the object exists
  if( img_object.empty() ){
    printf("Failure: Cannot read image");
    return;
  }

  //--Checks if the scene exists
  if( img_scene.empty() ){
    printf("Failure: Cannot read image");
    return;
  }

  //--Creates pointers with feature detector, find descriptor and matcher tools
  Ptr<FeatureDetector> featureDetector = FeatureDetector::create( name_feature );
  Ptr<DescriptorExtractor> featureExtractor = DescriptorExtractor::create( name_descriptor );
  Ptr<DescriptorMatcher> featureMatcher = DescriptorMatcher::create( name_matcher );

  //--Detects features of object and scene
  featureDetector->detect(img_object, keypoints_object);
  featureDetector->detect(img_scene, keypoints_scene);

  //--Computes descriptors
  featureExtractor->compute(img_object, keypoints_object, descriptors_object);
  featureExtractor->compute(img_scene, keypoints_scene, descriptors_scene);

  //--Matches descriptors
  try{
  featureMatcher->match( descriptors_object, descriptors_scene, matches );
  }
  catch( Exception ){
    printf("Failure: there are no keypoints to match\n");
    return;
  }

  //--Initialize max and min distances between descriptors
  double max_dist = 0; double min_dist = 100;

  //--Finds max and min distances between descriptors
  for(int i = 0; i < descriptors_object.rows; i++)
  { double dist = matches[i].distance;
    if(dist < min_dist) min_dist = dist;
    if(dist > max_dist) max_dist = dist;
  }

  //--Finds good matches
  for(int i = 0; i < descriptors_object.rows; i++)
  { if(matches[i].distance <= 3.4 * min_dist)
     { good_matches.push_back( matches[i]); }
  }

  //--Stores good matches' points of object and scene
  for(int i = 0; i < good_matches.size(); i++)
  {
    //--Get points from good matches
    obj.push_back(keypoints_object[ good_matches[i].queryIdx ].pt);
    scene.push_back(keypoints_scene[ good_matches[i].trainIdx ].pt);
  }

  //--Gets perspective from scene
  H = findHomography(obj, scene, CV_RANSAC);

  //--Get the corners from the object
  object_corner.push_back( cvPoint(0,0) );
  object_corner.push_back( cvPoint(img_object.cols, 0) );
  object_corner.push_back( cvPoint(img_object.cols, img_object.rows) );
  object_corner.push_back( cvPoint(0, img_object.rows) );

  //--Moves scene corners to fit dials
  perspectiveTransform(object_corner, scene_corner, H);

  //-- Draw lines between the corners (the mapped object in the scene - image_2 )
  line( img_scene, scene_corner[0], scene_corner[1], Scalar( 0, 255, 0, 255), 2 );
  line( img_scene, scene_corner[1], scene_corner[2], Scalar( 0, 255, 0, 255), 2 );
  line( img_scene, scene_corner[2], scene_corner[3], Scalar( 0, 255, 0, 255), 2 );
  line( img_scene, scene_corner[3], scene_corner[0], Scalar( 0, 255, 0, 255), 2 );

  //-- Resize image
  Size_<int> dsize = Size( round( img_scene.cols / 2 ) , round( img_scene.rows / 2 ) );
  resize( img_scene, result, dsize );
  imshow( "Good Matches", result );
  moveWindow("Good Matches", 500, 0 );

  char s = ' ';
  while ((s = waitKey(0)) != 'q');  // Keep window there until user presses 'q' to quit.

  destroyWindow("Good Matches");

  //--Checks if segmentation is good
  segmentation = checkSegmentation();
  if( !segmentation ){
    printf("Failure: bad segmentation");
    return;
  }

  //Crops each dial
  cropDials();
}

///--Crops dials
void descriptors::cropDials( void )
{
  int xInit = round(scene_corner[0].x);
  int xFin = round(scene_corner[2].x);
  int yInit = round(scene_corner[0].y);
  int yFin = round(scene_corner[2].y);

  Mat dial1 = img_scene( Range( yInit, yFin ), Range(xInit, (xFin + 4 * xInit)/5));
  Mat dial2 = img_scene( Range( yInit, yFin ), Range((xFin + 4 * xInit)/5, (2 * xFin + 3 * xInit)/5));
  Mat dial3 = img_scene( Range( yInit, yFin ), Range((2 * xFin + 3 * xInit)/5, (3 * xFin + 2 * xInit)/5));
  Mat dial4 = img_scene( Range( yInit, yFin ), Range((3 * xFin + 2 * xInit)/5, (4 * xFin + 1 * xInit)/5));
  Mat dial5 = img_scene( Range( yInit, yFin ), Range((4 * xFin + 1 * xInit)/5, (5 * xFin + 0 * xInit)/5));

  imshow("Intento", dial1);
  waitKey(0);
}

/*int matcher::match(const string matcher_tool, const string objeto, const string escena, float desv)
{
  Mat dial1T, dial2T, dial3T, dial4T, dial5T;

  threshold(dial1, dial1T, 0, 255,  THRESH_OTSU);
  threshold(dial2, dial2T, 0, 255,  THRESH_OTSU);
  threshold(dial3, dial3T, 0, 255,  THRESH_OTSU);
  threshold(dial4, dial4T, 0, 255,  THRESH_OTSU);
  threshold(dial5, dial5T, 0, 255,  THRESH_OTSU);

  Mat erode1, erode2, erode3, erode4, erode5;

  Mat element = getStructuringElement(MORPH_CROSS,
                        Size(10, 10));

  dilate(dial1T, erode1, element, Point(-1,-1), 1);
  dilate(dial2T, erode2, element, Point(-1,-1), 1);
  dilate(dial3T, erode3, element, Point(-1,-1), 1);
  dilate(dial4T, erode4, element, Point(-1,-1), 1);
  dilate(dial5T, erode5, element, Point(-1,-1), 1);

  Point Black1 = test(erode1, erode1.cols / 2, erode1.rows / 2);
  Point Black2 = test(erode2, erode2.cols / 2, erode2.rows / 2);
  Point Black3 = test(erode3, erode3.cols / 2, erode3.rows / 2);
  Point Black4 = test(erode4, erode4.cols / 2, erode4.rows / 2);
  Point Black5 = test(erode5, erode5.cols / 2, erode5.rows / 2);

  try{
    floodFill(erode1, Black1, 150);
    floodFill(erode2, Black2, 150);
    floodFill(erode3, Black3, 150);
    floodFill(erode4, Black4, 150);
    floodFill(erode5, Black5, 150);
  }
  catch( Exception ){
    return -2;
  }

  Mat threshold1 = thresholding( erode1 );
  Mat threshold2 = thresholding( erode2 );
  Mat threshold3 = thresholding( erode3 );
  Mat threshold4 = thresholding( erode4 );
  Mat threshold5 = thresholding( erode5 );

  printf("[Dial 1]: ");
  Mat drawCircle1 = centerFinding( threshold1, 1);
  printf("[Dial 2]: ");
  Mat drawCircle2 = centerFinding( threshold2, 2 );
  printf("[Dial 3]: ");
  Mat drawCircle3 = centerFinding( threshold3, 1 );
  printf("[Dial 4]: ");
  Mat drawCircle4 = centerFinding( threshold4, 2 );
  printf("[Dial 5]: ");
  Mat drawCircle5 = centerFinding( threshold5, 1 );


  imshow("Foto1", dial1);
  moveWindow("Foto1", 500, 0);
  waitKey(0);
  imshow("Ero1", drawCircle1);
  moveWindow("Ero1", 800, 0);
  waitKey(0);

  imshow("Foto2", dial2);
  moveWindow("Foto2", 500, 200);
  waitKey(0);
  imshow("Ero2", drawCircle2);
  moveWindow("Ero2", 800, 200);
  waitKey(0);

  imshow("Foto3", dial3);
  moveWindow("Foto3", 500, 400);
  waitKey(0);
  imshow("Ero3", drawCircle3);
  moveWindow("Ero3", 800, 400);
  waitKey(0);

  imshow("Foto4", dial4);
  moveWindow("Foto4", 500, 600);
  waitKey(0);
  imshow("Ero4", drawCircle4);
  moveWindow("Ero4", 800, 600);
  waitKey(0);

  imshow("Foto5", dial5);
  moveWindow("Foto5", 500, 800);
  waitKey(0);
  imshow("Ero5", drawCircle5);
  moveWindow("Ero5", 800, 800);
  waitKey(0);
  destroyAllWindows();
  puts("");

}*/

///--Checks if segmentation is good
bool descriptors::checkSegmentation( void )
{
  //--Checks if there is a segmentation
  if( ( scene_corner[2].x - scene_corner[3].x ) == 0  ){
    return false;
  }

  //--Checks if the segmentation form is a rectangle
  if(((scene_corner[1].x - scene_corner[0].x) / (scene_corner[2].x - scene_corner[3].x)) < 0.9 ||
        ((scene_corner[1].x - scene_corner[0].x) / (scene_corner[2].x - scene_corner[3].x)) > 1.1){
    return false;
  }

  //--Checks if the segmentation form is a rectangle
  if(((scene_corner[3].y - scene_corner[0].y) / (scene_corner[2].y - scene_corner[1].y)) < 0.9 ||
        ((scene_corner[3].y - scene_corner[0].y) / (scene_corner[2].y - scene_corner[1].y)) > 1.1){
    return false;
  }

  //--Checks if corner points are inside the image
  if(scene_corner[1].x > img_scene.cols || scene_corner[0].x < 0 || scene_corner[3].x < 0){
    return false;
  }

  //--Checks if corner points are inside the image
  if(scene_corner[3].y > img_scene.rows || scene_corner[0].y < 0 || scene_corner[1].y < 0){
    return false;
  }

  return true;
}


/*void descriptors::writeKeypoints( void )
{
  Scalar keypointColor1 = Scalar(0, 0, 255);// Green keypoints.

  drawKeypoints( img_source, keypoints_source, output_source, keypointColor1, DrawMatchesFlags::DEFAULT);

  namedWindow("Output1");

  Size_<int> dsize = Size( round( output_source.cols / 2 ) , round( output_source.rows / 2 ) );
  resize( output_source, result, dsize );

  imshow("Output1", result);
  char c = ' ';
  while ((c = waitKey(0)) != 'q');  // Keep window there until user presses 'q' to quit.
  destroyWindow("Output1");
}*/


/*void matcher::setInitialData(vector<KeyPoint> keypoints_obj, vector<KeyPoint> keypoints_sce, Mat ob, Mat sc)
{
  keypoints_object = keypoints_obj;
  keypoints_scene = keypoints_sce;
  descriptors_object = ob;
  descriptors_scene = sc;
}

Point matcher::test(Mat inputImg, int x, int y){
  while(inputImg.at<uchar>(x, y) == 255){
    y += 1;
  }
  Point black(y,x);
  return black;
}

Mat matcher::thresholding(Mat inputImg){
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
}*/

/*Mat matcher::centerFinding(Mat inputImg, int tipoDial){
  int noPuntosHor = 0, noPuntosVer = 0, noPuntosHorMax = 0, noPuntosVerMax = 0;
  double x, y;
  double pi = 3.1415926535;

  Moments momentos1 =  moments( inputImg, true );

  x = momentos1.m10 / momentos1.m00;
  y = momentos1.m01 / momentos1.m00;

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

  double numbers[ 6 ] = { pi / 2, 3 * pi / 10, pi / 10, - pi / 10, - 3 * pi / 10, - pi / 2 };
  double hyp = sqrt( pow( ( x1 - x ), 2 ) + pow( ( y - y1 ), 2 ) );

  double teta1 = atan( (double)( y - y1 ) / ( x1 - x ) );
  double teta2 = asin( (double)( y - y1 ) / hyp );
  double teta3 = acos( (double)( x1 - x ) / hyp );

  int lectura = 0;

  for( int i = 0; i <= 5; i ++){
    if( teta1 <= numbers[ i ] && teta1 > numbers[ i + 1 ]){
      if( teta1 > 0 && teta2 < 0 ){
        lectura = i + 5;
        continue;
      }
      if( teta1 < 0 && teta2 > 0){
        lectura = i + 5;
        continue;
      }
      lectura = i;
      continue;
    }
  }

  if( tipoDial == 2 ){
    lectura = 9 - lectura;
  }

  printf("%d \n", lectura);

  circle( inputImg, Point(x, y), 3, Scalar( 150, 150, 150, 150), 1, 8, 0);
  circle( inputImg, Point(x1, y1), 3, Scalar( 0, 0, 0, 0), 1, 8, 0);
  return inputImg;
}*/








