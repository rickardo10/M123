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

	void setImageObject( const string img_object );
	void setImageScene( const string img_scene );
	void setFeatureDetector( const string name_featureD );
  void setFindDescriptors( const string name_Fdescriptor );
  void setMatcher( const string matcherN );

	void writeKeypoints( void );
	void featureDetector( void );

	void findDescriptors( const string descriptor_tool );
	Mat getDescriptors( void );
	vector<KeyPoint> getKeypoints( void );
  bool checkSegmentation( void );
  void cropDials( void );

  int match( const string matcher_tool, const string objeto, const string escena, float desv );
	void setInitialData( vector<KeyPoint> keypoints_obj, vector<KeyPoint> keypoints_sce, Mat ob, Mat sc );
	Point test(Mat inputImg, int x, int y);
	Mat thresholding(Mat inputImg);
	Mat centerFinding(Mat inputImg, int tipoDial);


private:

   Mat img_object;
   Mat img_scene;
   Mat img_matches;
   Mat result;
   Mat descriptors_object;
   Mat descriptors_scene;
   Mat dial1;
   Mat dial2;
   Mat dial3;
   Mat dial4;
   Mat dial5;
   Mat H;

   string name_feature;
   string name_descriptor;
   string name_matcher;

   bool segmentation;

   vector<KeyPoint> keypoints_object;
   vector<KeyPoint> keypoints_scene;
   vector<DMatch> matches;
   vector<DMatch > good_matches;
   vector<Point2f> obj;
   vector<Point2f> scene;
   vector<Point2f> object_corner;
   vector<Point2f> scene_corner;
};

#endif
