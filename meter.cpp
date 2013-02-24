#include <stdio.h>
#include <opencv2/core/core.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <vector>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include "meter.h"
#include <iostream>

using namespace std;
using namespace cv;

///--Constructor function to initialize private data
meter::meter( string img_scene, string name_feature, string name_descriptor,
                          string name_matcher, string img_object )
{
  setObject( img_scene, name_feature, name_descriptor, name_matcher, img_object );
}

///--Set object, feature, descriptor, matcher and scene
void meter::setObject( string sceneName, string featureDetName, string findDescriptorsName,
          string matcherName, string objectName )
{
  setImageObject( objectName );
  setImageScene( sceneName );
  setFeatureDetector( featureDetName );
  setFindDescriptors( findDescriptorsName );
  setMatcher( matcherName );
  processData();

  if( !failure ){
    cropDials();
  }
}

///--Reads object
void meter::setImageObject( const string name_object )
{
  //--Reads image
	img_object = imread( name_object, 0);
}

///--Reads Scene
void meter::setImageScene( const string name_scene )
{
  //--Reads image
	img_scene = imread( name_scene, 0);
	img_sceneColor = imread( name_scene);
}

///--Reads feature detector
void meter::setFeatureDetector( const string name_featureD )
{
   name_feature = name_featureD;
}

///--Reads descriptor finder
void meter::setFindDescriptors( const string name_Fdescriptor )
{
   name_descriptor = name_Fdescriptor;
}

///--Reads matcher
void meter::setMatcher( const string matcherN )
{
   name_matcher = matcherN;
}

///--Process available data in order to returns dials
void meter::processData( void )
{
  //--Checks if the object exists
  if( img_object.empty() ){
    printf("Failure: Cannot read image");
    failure = true;
    return;
  }

  //--Checks if the scene exists
  if( img_scene.empty() ){
    printf("Failure: Cannot read image");
    failure = true;
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

  //--Checks if segmentation is good
  failure = checkSegmentation();
  if( failure ){
    printf("Failure: bad segmentation");
    return;
  }
}

///--Crops dials
void meter::cropDials( void )
{
  int xInit = round(scene_corner[0].x);
  int xFin = round(scene_corner[2].x);
  int yInit = round(scene_corner[0].y);
  int yFin = round(scene_corner[2].y);

  dials.push_back( img_scene( Range( yInit, yFin ), Range( xInit, (xFin + 4 * xInit)/5) ) );
  dials.push_back( img_scene( Range( yInit, yFin ), Range( ( xFin + 4 * xInit ) / 5, ( 2 * xFin + 3 * xInit) / 5) ) );
  dials.push_back( img_scene( Range( yInit, yFin ), Range(( 2  * xFin + 3 * xInit) / 5, ( 3 * xFin + 2 * xInit ) / 5 ) ) );
  dials.push_back( img_scene( Range( yInit, yFin ), Range((3 * xFin + 2 * xInit)/5, ( 4 * xFin + 1 * xInit ) / 5 ) ) );
  dials.push_back( img_scene( Range( yInit, yFin ), Range((4 * xFin + 1 * xInit)/5, ( 5 * xFin + 0 * xInit ) / 5) ) );

  dials.push_back( img_sceneColor( Range( yInit, yFin ), Range( xInit, (xFin + 4 * xInit)/5) ) );
  dials.push_back( img_sceneColor( Range( yInit, yFin ), Range( ( xFin + 4 * xInit ) / 5, ( 2 * xFin + 3 * xInit) / 5) ) );
  dials.push_back( img_sceneColor( Range( yInit, yFin ), Range(( 2  * xFin + 3 * xInit) / 5, ( 3 * xFin + 2 * xInit ) / 5 ) ) );
  dials.push_back( img_sceneColor( Range( yInit, yFin ), Range((3 * xFin + 2 * xInit)/5, ( 4 * xFin + 1 * xInit ) / 5 ) ) );
  dials.push_back( img_sceneColor( Range( yInit, yFin ), Range((4 * xFin + 1 * xInit)/5, ( 5 * xFin + 0 * xInit ) / 5) ) );
}

///--Shows a meter image with lines surroundig dials
Mat meter::showSegmentation( void ){

  //--Creates a new color image
  Mat img_seg( img_sceneColor );

  //--Draw lines between the corners (the mapped object in the scene - image_2 )
  line( img_seg, scene_corner[0], scene_corner[1], Scalar( 0, 255, 0, 255), 2 );
  line( img_seg, scene_corner[1], scene_corner[2], Scalar( 0, 255, 0, 255), 2 );
  line( img_seg, scene_corner[2], scene_corner[3], Scalar( 0, 255, 0, 255), 2 );
  line( img_seg, scene_corner[3], scene_corner[0], Scalar( 0, 255, 0, 255), 2 );

  //--Resize image
  Mat result;
  Size_<int> dsize = Size( round( img_seg.cols / 2 ) , round( img_seg.rows / 2 ) );
  resize( img_seg, result, dsize );
  imshow( "Good Matches", result );
  moveWindow("Good Matches", 500, 0 );

  char s = ' ';
  while ((s = waitKey(0)) != 'q');  // Keep window there until user presses 'q' to quit.

  destroyAllWindows();
  return result;
}

/*int matcher::match(const string matcher_tool, const string objeto, const string escena, float desv)
{


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
bool meter::checkSegmentation( void )
{
  //--Checks if there is a segmentation
  if( ( scene_corner[2].x - scene_corner[3].x ) == 0  ){
    return true;
  }

  //--Checks if the segmentation form is a rectangle
  if(((scene_corner[1].x - scene_corner[0].x) / (scene_corner[2].x - scene_corner[3].x)) < 0.9 ||
        ((scene_corner[1].x - scene_corner[0].x) / (scene_corner[2].x - scene_corner[3].x)) > 1.1){
    return true;
  }

  //--Checks if the segmentation form is a rectangle
  if(((scene_corner[3].y - scene_corner[0].y) / (scene_corner[2].y - scene_corner[1].y)) < 0.9 ||
        ((scene_corner[3].y - scene_corner[0].y) / (scene_corner[2].y - scene_corner[1].y)) > 1.1){
    return true;
  }

  //--Checks if corner points are inside the image
  if(scene_corner[1].x > img_scene.cols || scene_corner[0].x < 0 || scene_corner[3].x < 0){
    return true;
  }

  //--Checks if corner points are inside the image
  if(scene_corner[3].y > img_scene.rows || scene_corner[0].y < 0 || scene_corner[1].y < 0){
    return true;
  }

  return false;
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



///--Returns variable segmentation
bool meter::getFailure()
{
  return failure;
}

///-Returns vector dials
vector<Mat> meter::getDials( void )
{
  return dials;
}








