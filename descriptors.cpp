#include <stdio.h>
#include <opencv2/core/core.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <vector>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include "descriptors.h"

using namespace std;
using namespace cv;

void descriptors::setImage( const string name_source )
{
	img_source = imread( name_source, 0);
	if(img_source.empty() )
  {
    printf("Can't read the image\n");
    exit;
  }
}


void descriptors::featureDetector( const string detector_tool )
{
  Ptr<FeatureDetector> featureDetector = FeatureDetector::create( detector_tool );
  featureDetector->detect(img_source, keypoints_source);
}


void descriptors::writeKeypoints( void )
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
}


void descriptors::findDescriptors( const string descriptor_tool )
{
  Ptr<DescriptorExtractor> featureExtractor = DescriptorExtractor::create( descriptor_tool );
  featureExtractor->compute(img_source, keypoints_source, descriptors_source);
}


Mat descriptors::getDescriptors( void )
{
  return descriptors_source;
}

vector<KeyPoint> descriptors::getKeypoints( void )
{
  return keypoints_source;
}










