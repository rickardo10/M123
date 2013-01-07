#include <iostream>
#include <stdio.h>
#include <opencv2/core/core.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <vector>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include "matcher.h"

using namespace std;
using namespace cv;


void matcher::match( const string matcher_tool, const string objeto, const string escena )
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

  float desv;
  cout << endl << "Escriba un valor para la desviación:\n? ";
  cin >> desv;

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

  namedWindow("matches", 1);

  //-- Draw lines between the corners (the mapped object in the scene - image_2 )
  line( img_matches, scene_corners[0] + Point2f( img_object.cols, 0), scene_corners[1] + Point2f( img_object.cols, 0), Scalar( 0, 255, 0), 2 );
  line( img_matches, scene_corners[1] + Point2f( img_object.cols, 0), scene_corners[2] + Point2f( img_object.cols, 0), Scalar( 0, 255, 0), 2 );
  line( img_matches, scene_corners[2] + Point2f( img_object.cols, 0), scene_corners[3] + Point2f( img_object.cols, 0), Scalar( 0, 255, 0), 2 );
  line( img_matches, scene_corners[3] + Point2f( img_object.cols, 0), scene_corners[0] + Point2f( img_object.cols, 0), Scalar( 0, 255, 0), 2 );

  //-- Resize image
  Size_<int> dsize = Size( round( img_matches.cols / 2 ) , round( img_matches.rows / 2 ) );
  resize( img_matches, result, dsize );
  imshow( "Good Matches", result );

  char s = ' ';
  while ((s = waitKey(0)) != 'q');  // Keep window there until user presses 'q' to quit.

  destroyWindow("Output1");
}


void matcher::setInitialData( vector<KeyPoint> keypoints_obj, vector<KeyPoint> keypoints_sce, Mat ob, Mat sc )
{
  keypoints_object = keypoints_obj;
  keypoints_scene = keypoints_sce;
  descriptors_object = ob;
  descriptors_scene = sc;
}


