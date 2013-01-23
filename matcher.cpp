#include <iostream>
#include <stdio.h>
#include <opencv2/core/core.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <vector>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include "matcher.h"
#include <math.h>

using namespace std;
using namespace cv;


int matcher::match(const string matcher_tool, const string objeto, const string escena, float desv)
{
  Mat img_object = imread(objeto, 0);
  Mat img_scene  = imread(escena, 0);

  if( img_scene.empty() ){
      return -1;
  }

  Ptr<DescriptorMatcher> featureMatcher = DescriptorMatcher::create( matcher_tool );
  featureMatcher->match( descriptors_object, descriptors_scene, matches );

  double max_dist = 0; double min_dist = 100;

  for(int i = 0; i < descriptors_object.rows; i++)
  { double dist = matches[i].distance;
    if(dist < min_dist) min_dist = dist;
    if(dist > max_dist) max_dist = dist;
  }

  for(int i = 0; i < descriptors_object.rows; i++)
  { if(matches[i].distance <= desv*min_dist)
     { good_matches.push_back( matches[i]); }
  }

//  drawMatches( img_object, keypoints_object, img_scene, keypoints_scene,
//            good_matches, img_matches, Scalar::all(-1), Scalar::all(-1),
//            vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS );

  //-- Localize the object
  for(int i = 0; i < good_matches.size(); i++)
  {
    //-- Get the keypoints from the good matches
    obj.push_back(keypoints_object[ good_matches[i].queryIdx ].pt);
    scene.push_back(keypoints_scene[ good_matches[i].trainIdx ].pt);
  }

  H = findHomography(obj, scene, CV_RANSAC);

  vector<Point2f> object_corners(4);
  vector<Point2f> scene_corners(4);
  //-- Get the corners from the image_1 ( the object to be "detected" )
  object_corners[0] = cvPoint(0,0);
  object_corners[1] = cvPoint(img_object.cols, 0);
  object_corners[2] = cvPoint(img_object.cols, img_object.rows);
  object_corners[3] = cvPoint(0, img_object.rows);

  perspectiveTransform(object_corners, scene_corners, H);

  if( ( scene_corners[2].x - scene_corners[3].x ) == 0  ){
    return -2;
  }

  if(((scene_corners[1].x - scene_corners[0].x) / (scene_corners[2].x - scene_corners[3].x)) < 0.9 ||
        ((scene_corners[1].x - scene_corners[0].x) / (scene_corners[2].x - scene_corners[3].x)) > 1.1){
    return -2;
  }

  if(((scene_corners[3].y - scene_corners[0].y) / (scene_corners[2].y - scene_corners[1].y)) < 0.9 ||
        ((scene_corners[3].y - scene_corners[0].y) / (scene_corners[2].y - scene_corners[1].y)) > 1.1){
    return -2;
  }

  if(scene_corners[1].x > img_scene.cols || scene_corners[0].x < 0 || scene_corners[3].x < 0){
    return -2;
  }

  if(scene_corners[3].y > img_scene.rows || scene_corners[0].y < 0 || scene_corners[1].y < 0){
    return -2;
  }

//  //-- Draw lines between the corners (the mapped object in the scene - image_2 )
//  line( img_scene, scene_corners[0], scene_corners[1], Scalar( 0, 255, 0, 255), 2 );
//  line( img_scene, scene_corners[1], scene_corners[2], Scalar( 0, 255, 0, 255), 2 );
//  line( img_scene, scene_corners[2], scene_corners[3], Scalar( 0, 255, 0, 255), 2 );
//  line( img_scene, scene_corners[3], scene_corners[0], Scalar( 0, 255, 0, 255), 2 );
//
//  //-- Resize image
//  Size_<int> dsize = Size( round( img_scene.cols / 2 ) , round( img_scene.rows / 2 ) );
//  resize( img_scene, result, dsize );
//  imshow( "Good Matches", result );
//  moveWindow("Good Matches", 500, 0 );
//
//  char s = ' ';
//  while ((s = waitKey(0)) != 'q');  // Keep window there until user presses 'q' to quit.
//
//  destroyWindow("Good Matches");

  int xInit = round(scene_corners[0].x);
  int xFin = round(scene_corners[2].x);
  int yInit = round(scene_corners[0].y);
  int yFin = round(scene_corners[2].y);

  Mat dial1 = img_scene( Range( yInit, yFin ), Range(xInit, (xFin + 4 * xInit)/5));
  Mat dial2 = img_scene( Range( yInit, yFin ), Range((xFin + 4 * xInit)/5, (2 * xFin + 3 * xInit)/5));
  Mat dial3 = img_scene( Range( yInit, yFin ), Range((2 * xFin + 3 * xInit)/5, (3 * xFin + 2 * xInit)/5));
  Mat dial4 = img_scene( Range( yInit, yFin ), Range((3 * xFin + 2 * xInit)/5, (4 * xFin + 1 * xInit)/5));
  Mat dial5 = img_scene( Range( yInit, yFin ), Range((4 * xFin + 1 * xInit)/5, (5 * xFin + 0 * xInit)/5));


  Mat dial1T, dial2T, dial3T, dial4T, dial5T;

  threshold(dial1, dial1T, 0, 255,  THRESH_OTSU);
  threshold(dial2, dial2T, 0, 255,  THRESH_OTSU);
  threshold(dial3, dial3T, 0, 255,  THRESH_OTSU);
  threshold(dial4, dial4T, 0, 255,  THRESH_OTSU);
  threshold(dial5, dial5T, 0, 255,  THRESH_OTSU);

  Mat erode1, erode2, erode3, erode4, erode5;

  Mat element = getStructuringElement(MORPH_CROSS,
                        Size(30, 30));

  dilate(dial1T, erode1, element, Point(-1,-1), 0);
  dilate(dial2T, erode2, element, Point(-1,-1), 0);
  dilate(dial3T, erode3, element, Point(-1,-1), 0);
  dilate(dial4T, erode4, element, Point(-1,-1), 0);
  dilate(dial5T, erode5, element, Point(-1,-1), 0);

  try{
    floodFill(erode1, Point(erode1.rows / 2, erode1.cols / 2), 150);
    floodFill(erode2, Point(erode2.rows / 2, erode2.cols / 2), 150);
    floodFill(erode3, Point(erode3.rows / 2, erode3.cols / 2), 150);
    floodFill(erode4, Point(erode4.rows / 2, erode4.cols / 2), 150);
    floodFill(erode5, Point(erode5.rows / 2, erode5.cols / 2), 150);
  }
  catch( Exception )
  {
    return 2;
  }

  imshow("Ero1", erode1);
  waitKey(0);
  imshow("Ero2", erode2);
  waitKey(0);
  imshow("Ero3", erode3);
  waitKey(0);
  imshow("Ero4", erode4);
  waitKey(0);
  imshow("Ero5", erode5);
  waitKey(0);
  destroyAllWindows();
  }



void matcher::setInitialData(vector<KeyPoint> keypoints_obj, vector<KeyPoint> keypoints_sce, Mat ob, Mat sc)
{
  keypoints_object = keypoints_obj;
  keypoints_scene = keypoints_sce;
  descriptors_object = ob;
  descriptors_scene = sc;
}


