//matcher.hpp
//printImage Class Definition
//Member functions defined in printImage.h

#ifndef MATCHER_H
#define MATCHER_H

using namespace std;
using namespace cv;

class matcher
{
public:

	void match( const string matcher_tool, const string objeto, const string escena );
	void setInitialData( vector<KeyPoint> keypoints_obj, vector<KeyPoint> keypoints_sce, Mat ob, Mat sc );

private:

   Mat img_object;
   Mat img_scene;
   Mat img_matches;
   Mat result;
   Mat descriptors_object;
   Mat descriptors_scene;
   Mat H;
   vector<KeyPoint> keypoints_object;
   vector<KeyPoint> keypoints_scene;
   vector<DMatch> matches;
   vector< DMatch > good_matches;
   vector<Point2f> obj;
   vector<Point2f> scene;
   vector<Point2f> object_corners;
   vector<Point2f> scene_corners;
};

#endif
