/*
Date: 04/01/2013
Creator: Ricardo Antonio Ocampo Vega
Description: Detects the keypoints of an image. After, extracts the
keypoints descriptors. Finally, draw the detected keypoints on the image
*/

#include <stdio.h>
#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <vector>

using namespace std;
using namespace cv;

int main(int argc, char *argv[])
{
  Mat image1 = imread("dial.png");
  Mat image2 = imread("10.jpg");

  if(image1.empty() || image2.empty())
  {
      printf("Can't read one of the images\n");
      return -1;
  }

  // Create smart pointer for feature detector.
  Ptr<FeatureDetector> featureDetector = FeatureDetector::create("ORB");
  vector<KeyPoint> keypoints1, keypoints2;

  // Detect the keypoints
  featureDetector->detect(image1, keypoints1); // NOTE: featureDetector is a pointer hence the '->'.
  featureDetector->detect(image2, keypoints2);

  //Similarly, we create a smart pointer to the extractor.
  Ptr<DescriptorExtractor> featureExtractor = DescriptorExtractor::create("SIFT");

  // Compute the 128 dimension descriptor at each keypoint.
  // Each row in "descriptors" correspond to the descriptor for each keypoint
  Mat descriptors1, descriptors2;
  featureExtractor->compute(image1, keypoints1, descriptors1);
  featureExtractor->compute(image2, keypoints2, descriptors2);

//  // If you would like to draw the detected keypoint just to check
//  Mat outputImage1, outputImage2;
//
//  Scalar keypointColor1 = Scalar(255, 0, 0);     // Blue keypoints.
//  Scalar keypointColor2 = Scalar(0, 0, 255);     // Blue keypoints.
//
//  drawKeypoints(image1, keypoints1, outputImage1, keypointColor1, DrawMatchesFlags::DEFAULT);
//  drawKeypoints(image2, keypoints2, outputImage2, keypointColor2, DrawMatchesFlags::DEFAULT);
//
//  namedWindow("Output1");
//  imshow("Output1", outputImage1);
//
//  char c = ' ';
//  while ((c = waitKey(0)) != 'q');  // Keep window there until user presses 'q' to quit.
//
//  destroyWindow("Output1");
//  namedWindow("Output2");
//  imshow("Output2", outputImage2);
//  char r = ' ';
//  while ((r = waitKey(0)) != 'q');  // Keep window there until user presses 'q' to quit.
//  destroyWindow("Output1");

  //matching descriptors
  Ptr<DescriptorMatcher> featureMatcher = DescriptorMatcher::create("BruteForce");
  vector<DMatch> matches;
  featureMatcher->match( descriptors1, descriptors2, matches );

  // drawing the resulting matches
  namedWindow("matches", 1);
  Mat img_matches;
  drawMatches(image1, keypoints1, image2, keypoints2, matches, img_matches);
  imshow("matches", img_matches);
  waitKey(0);

  return 0;

}
