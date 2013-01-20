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


void matcher::match( const string matcher_tool, const string objeto, const string escena, float desv)
{
  Mat img_object = imread(objeto);
  Mat img_scene  = imread(escena);

  Ptr<DescriptorMatcher> featureMatcher = DescriptorMatcher::create( matcher_tool );
  featureMatcher->match( descriptors_object, descriptors_scene, matches );

  double max_dist = 0; double min_dist = 100;

  for( int i = 0; i < descriptors_object.rows; i++ )
  { double dist = matches[i].distance;
    if( dist < min_dist ) min_dist = dist;
    if( dist > max_dist ) max_dist = dist;
  }

  for( int i = 0; i < descriptors_object.rows; i++ )
  { if( matches[i].distance <= desv*min_dist )
     { good_matches.push_back( matches[i]); }
  }

  drawMatches( img_object, keypoints_object, img_scene, keypoints_scene,
            good_matches, img_matches, Scalar::all(-1), Scalar::all(-1),
            vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS );

  //-- Localize the object
  for( int i = 0; i < good_matches.size(); i++ )
  {
    //-- Get the keypoints from the good matches
    obj.push_back( keypoints_object[ good_matches[i].queryIdx ].pt );
    scene.push_back( keypoints_scene[ good_matches[i].trainIdx ].pt );
  }

  H = findHomography( obj, scene, CV_RANSAC );

  vector<Point2f> object_corners(4);
  vector<Point2f> scene_corners(4);
  //-- Get the corners from the image_1 ( the object to be "detected" )
  object_corners[0] = cvPoint(0,0);
  object_corners[1] = cvPoint( img_object.cols, 0 );
  object_corners[2] = cvPoint( img_object.cols, img_object.rows );
  object_corners[3] = cvPoint( 0, img_object.rows );

  perspectiveTransform( object_corners, scene_corners, H);

  //-- Draw lines between the corners (the mapped object in the scene - image_2 )
  line( img_scene, scene_corners[0], scene_corners[1], Scalar( 0, 255, 0), 2 );
  line( img_scene, scene_corners[1], scene_corners[2], Scalar( 0, 255, 0), 2 );
  line( img_scene, scene_corners[2], scene_corners[3], Scalar( 0, 255, 0), 2 );
  line( img_scene, scene_corners[3], scene_corners[0], Scalar( 0, 255, 0), 2 );

  //-- Resize image
  Size_<int> dsize = Size( round( img_scene.cols / 2 ) , round( img_scene.rows / 2 ) );
  resize( img_scene, result, dsize );
  imshow( "Good Matches", result );

  char s = ' ';
  while ((s = waitKey(0)) != 'q');  // Keep window there until user presses 'q' to quit.

  destroyWindow("Good Matches");

//  int xInit = round(scene_corners[0].x);
//  int xFin = round(scene_corners[2].x);
//  int yInit = round(scene_corners[0].y);
//  int yFin = round(scene_corners[2].y);
//
//  Mat dial1 = img_scene( Range( yInit, yFin ), Range(xInit, (xFin + 4 * xInit)/5));
//  Mat dial2 = img_scene( Range( yInit, yFin ), Range((xFin + 4 * xInit)/5, (2 * xFin + 3 * xInit)/5));
//  Mat dial3 = img_scene( Range( yInit, yFin ), Range((2 * xFin + 3 * xInit)/5, (3 * xFin + 2 * xInit)/5));
//  Mat dial4 = img_scene( Range( yInit, yFin ), Range((3 * xFin + 2 * xInit)/5, (4 * xFin + 1 * xInit)/5));
//  Mat dial5 = img_scene( Range( yInit, yFin ), Range((4 * xFin + 1 * xInit)/5, (5 * xFin + 0 * xInit)/5));
//
//  imshow("test", dial1);
//  waitKey(0);
//
//  imshow("test", dial2);
//  waitKey(0);
//
//  imshow("test", dial3);
//  waitKey(0);
//
//  imshow("test", dial4);
//  waitKey(0);
//
//  imshow("test", dial5);
//  waitKey(0);
}


void matcher::setInitialData( vector<KeyPoint> keypoints_obj, vector<KeyPoint> keypoints_sce, Mat ob, Mat sc )
{
  keypoints_object = keypoints_obj;
  keypoints_scene = keypoints_sce;
  descriptors_object = ob;
  descriptors_scene = sc;
}


