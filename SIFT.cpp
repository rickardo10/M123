/*
Date: 04/01/2013
Creator: Ricardo Antonio Ocampo Vega
Description: Detects the keypoints of an image. After, extracts the
keypoints descriptors. Finally, compares the keypoints of the scene
and the object and find matches.
*/

#include <stdio.h>
#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <vector>

using namespace std;
using namespace cv;

int main(int argc, char *argv[])
{
  Mat img_object = imread("diales2.png", 0);
  Mat img_scene = imread("27.jpg", 0);

  if(img_object.empty() || img_scene.empty())
  {
      printf("Can't read one of the images\n");
      return -1;
  }

  // Create smart pointer for feature detector.
  Ptr<FeatureDetector> featureDetector = FeatureDetector::create("SIFT");
  vector<KeyPoint> keypoints_object, keypoints_scene;

  // Detect the keypoints
  featureDetector->detect(img_object, keypoints_object); // NOTE: featureDetector is a pointer hence the '->'.
  featureDetector->detect(img_scene, keypoints_scene);

  //Similarly, we create a smart pointer to the extractor.
  Ptr<DescriptorExtractor> featureExtractor = DescriptorExtractor::create("SIFT");

  // Compute the 128 dimension descriptor at each keypoint.
  // Each row in "descriptors" correspond to the descriptor for each keypoint
  Mat descriptors_object, descriptors_scene;
  featureExtractor->compute(img_object, keypoints_object, descriptors_object);
  featureExtractor->compute(img_scene, keypoints_scene, descriptors_scene);

//  // If you would like to draw the detected keypoint just to check
//  Mat output_object, output_scene;
//
//  Scalar keypointColor1 = Scalar(255, 0, 0);     // Blue keypoints.
//  Scalar keypointColor2 = Scalar(0, 0, 255);     // Blue keypoints.
//
//  drawKeypoints(img_object, keypoints_object, output_object, keypointColor1, DrawMatchesFlags::DEFAULT);
//  drawKeypoints(img_scene, keypoints_scene, output_scene, keypointColor2, DrawMatchesFlags::DEFAULT);
//
//  namedWindow("Output1");
//  imshow("Output1", output_object);
//
//  char c = ' ';
//  while ((c = waitKey(0)) != 'q');  // Keep window there until user presses 'q' to quit.
//
//  destroyWindow("Output1");
//  namedWindow("Output2");
//  imshow("Output2", output_scene);
//  char r = ' ';
//  while ((r = waitKey(0)) != 'q');  // Keep window there until user presses 'q' to quit.
//  destroyWindow("Output1");

  //matching descriptors
  Ptr<DescriptorMatcher> featureMatcher = DescriptorMatcher::create("FlannBased");
  vector<DMatch> matches;
  featureMatcher->match( descriptors_object, descriptors_scene, matches );

//  //drawing the resulting matches
//  namedWindow("matches", 1);
  Mat img_matches;
//  drawMatches(img_object, keypoints_object, img_scene, keypoints_scene, matches, img_matches);
//  imshow("matches", img_matches);
//
//  char r = ' ';
//  while ((r = waitKey(0)) != 'q');  // Keep window there until user presses 'q' to quit.
//
//  destroyWindow("Output1");
//  waitKey(0);

   //-- Quick calculation of max and min distances between keypoints
  double max_dist = 0; double min_dist = 100;

  for( int i = 0; i < descriptors_object.rows; i++ )
  { double dist = matches[i].distance;
    if( dist < min_dist ) min_dist = dist;
    if( dist > max_dist ) max_dist = dist;
  }

  printf("-- Max dist : %f \n", max_dist );
  printf("-- Min dist : %f \n", min_dist );

  //-- Draw only "good" matches (i.e. whose distance is less than 3*min_dist )
  vector< DMatch > good_matches;

  for( int i = 0; i < descriptors_object.rows; i++ )
  { if( matches[i].distance <= 3*min_dist )
     { good_matches.push_back( matches[i]); }
  }

  drawMatches( img_object, keypoints_object, img_scene, keypoints_scene,
               good_matches, img_matches, Scalar::all(-1), Scalar::all(-1),
               vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS );

//  namedWindow("matches", 1);
//  imshow("matches", img_matches);
//
//  char s = ' ';
//  while ((s = waitKey(0)) != 'q');  // Keep window there until user presses 'q' to quit.
//
//  destroyWindow("Output1");
//
  //-- Localize the object
  vector<Point2f> obj;
  vector<Point2f> scene;

  for( int i = 0; i < good_matches.size(); i++ )
  {
    //-- Get the keypoints from the good matches
    obj.push_back( keypoints_object[ good_matches[i].queryIdx ].pt );
    scene.push_back( keypoints_scene[ good_matches[i].trainIdx ].pt );
  }

  Mat H = findHomography( obj, scene, CV_RANSAC );

  //-- Get the corners from the image_1 ( the object to be "detected" )
  vector<Point2f> obj_corners(4);
  obj_corners[0] = cvPoint(0,0);
  obj_corners[1] = cvPoint( img_object.cols, 0 );
  obj_corners[2] = cvPoint( img_object.cols, img_object.rows );
  obj_corners[3] = cvPoint( 0, img_object.rows );
  vector<Point2f> scene_corners(4);

  perspectiveTransform( obj_corners, scene_corners, H);

  //-- Draw lines between the corners (the mapped object in the scene - image_2 )
  //Mat img_matches = imread("7.jpg");//Reads the scene in order to draw on the lines that segment the objects found
  line( img_matches, scene_corners[0] + Point2f( img_object.cols, 0), scene_corners[1] + Point2f( img_object.cols, 0), Scalar(0, 255, 0), 1 );
  line( img_matches, scene_corners[1] + Point2f( img_object.cols, 0), scene_corners[2] + Point2f( img_object.cols, 0), Scalar( 0, 255, 0), 1 );
  line( img_matches, scene_corners[2] + Point2f( img_object.cols, 0), scene_corners[3] + Point2f( img_object.cols, 0), Scalar( 0, 255, 0), 1 );
  line( img_matches, scene_corners[3] + Point2f( img_object.cols, 0), scene_corners[0] + Point2f( img_object.cols, 0), Scalar( 0, 255, 0), 1 );

  //-- Show detected matches
  imshow( "Good Matches & Object detection", img_matches );

  waitKey(0);
  return 0;

}
