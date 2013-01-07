//descriptors.hpp
//printImage Class Definition
//Member functions defined in printImage.h

#ifndef DESCRIPTORS_H
#define DESCRIPTORS_H

using namespace std;
using namespace cv;

class descriptors
{
public:

	void setImage( const string img_source );
	void writeKeypoints( void );
	void featureDetector( const string detector_tool );
	void findDescriptors( const string descriptor_tool );
	Mat getDescriptors( void );
	vector<KeyPoint> getKeypoints( void );


private:

   Mat img_source;
   Mat descriptors_source;
   Mat output_source;
   Mat result;
   vector<KeyPoint> keypoints_source;
};

#endif
