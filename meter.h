//meter.hpp
//printImage Class Definition
//Member functions defined in printImage.h

#ifndef METER_H
#define METER_H

using namespace std;
using namespace cv;

class meter
{
public:

  meter( string = "1.jpg", string = "SIFT", string = "SIFT",
               string = "BruteForce", string = "diales.jpg" );
  void setObject( string sceneName, string featureDetName, string findDescriptorsName,
          string matcherName, string objectName );
  void setImageObject( const string img_object );
  void setImageScene( const string img_scene );
  void setFeatureDetector( const string name_featureD );
  void setFindDescriptors( const string name_Fdescriptor );
  void setMatcher( const string matcherN );
  void showKeypoints( void );
  void processData( void );
  bool getFailure( void );
  bool checkDials( Mat inputImg );
  float getAngle( void );
  void checkSegmentation( void );
  void cropDials( void );
  Mat showSegmentation( void );
  vector<Mat> getDials( void );

private:

   Mat img_object;
   Mat img_scene;
   Mat img_sceneColor;
   Mat img_matches;
   Mat descriptors_object;
   Mat descriptors_scene;
   Mat H;
   string name_feature;
   string name_descriptor;
   string name_matcher;
   bool failure;
   float angleR;
   vector<KeyPoint> keypoints_object;
   vector<KeyPoint> keypoints_scene;
   vector<DMatch> matches;
   vector<DMatch > good_matches;
   vector<Point2f> obj;
   vector<Point2f> scene;
   vector<Point2f> object_corner;
   vector<Point2f> scene_corner;
   vector<Mat> dials;
};

#endif
