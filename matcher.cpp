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
                        Size(10, 10));

  dilate(dial1T, erode1, element, Point(-1,-1), 1);
  dilate(dial2T, erode2, element, Point(-1,-1), 1);
  dilate(dial3T, erode3, element, Point(-1,-1), 1);
  dilate(dial4T, erode4, element, Point(-1,-1), 1);
  dilate(dial5T, erode5, element, Point(-1,-1), 1);

  Point Black1 = test(erode1, erode1.cols / 2, erode1.rows / 2);
  Point Black2 = test(erode2, erode2.cols / 2, erode2.rows / 2);
  Point Black3 = test(erode3, erode3.cols / 2, erode3.rows / 2);
  Point Black4 = test(erode4, erode4.cols / 2, erode4.rows / 2);
  Point Black5 = test(erode5, erode5.cols / 2, erode5.rows / 2);

  try{
    floodFill(erode1, Black1, 150);
    floodFill(erode2, Black2, 150);
    floodFill(erode3, Black3, 150);
    floodFill(erode4, Black4, 150);
    floodFill(erode5, Black5, 150);

  }
  catch( Exception ){
    return -2;
  }

  Mat threshold1 = thresholding( erode1 );
  Mat threshold2 = thresholding( erode2 );
  Mat threshold3 = thresholding( erode3 );
  Mat threshold4 = thresholding( erode4 );
  Mat threshold5 = thresholding( erode5 );

  try{
    floodFill(threshold1, Point(0,0), 150);
    floodFill(threshold2, Point(0,0), 150);
    floodFill(threshold3, Point(0,0), 150);
    floodFill(threshold4, Point(0,0), 150);
    floodFill(threshold5, Point(0,0), 150);
  }
  catch( Exception ){
    return -2;
  }

  Mat drawCircle1 = centerFinding( threshold1, 1);
  Mat drawCircle2 = centerFinding( threshold2, 2 );
  Mat drawCircle3 = centerFinding( threshold3, 1 );
  Mat drawCircle4 = centerFinding( threshold4, 2 );
  Mat drawCircle5 = centerFinding( threshold5, 1 );

  imshow("Foto1", dial1);
  moveWindow("Foto1", 500, 0);
  waitKey(0);
  imshow("Ero1", drawCircle1);
  moveWindow("Ero1", 800, 0);
  waitKey(0);

  imshow("Foto2", dial2);
  moveWindow("Foto2", 500, 200);
  waitKey(0);
  imshow("Ero2", drawCircle2);
  moveWindow("Ero2", 800, 200);
  waitKey(0);

  imshow("Foto3", dial3);
  moveWindow("Foto3", 500, 400);
  waitKey(0);
  imshow("Ero3", drawCircle3);
  moveWindow("Ero3", 800, 400);
  waitKey(0);

  imshow("Foto4", dial4);
  moveWindow("Foto4", 500, 600);
  waitKey(0);
  imshow("Ero4", drawCircle4);
  moveWindow("Ero4", 800, 600);
  waitKey(0);

  imshow("Foto5", dial5);
  moveWindow("Foto5", 500, 800);
  waitKey(0);
  imshow("Ero5", drawCircle5);
  moveWindow("Ero5", 800, 800);
  waitKey(0);
  destroyAllWindows();
  puts("----------------");
}



void matcher::setInitialData(vector<KeyPoint> keypoints_obj, vector<KeyPoint> keypoints_sce, Mat ob, Mat sc)
{
  keypoints_object = keypoints_obj;
  keypoints_scene = keypoints_sce;
  descriptors_object = ob;
  descriptors_scene = sc;
}

Point matcher::test(Mat inputImg, int x, int y){
  while(inputImg.at<uchar>(x, y) == 255){
    y += 1;
  }
  Point black(y,x);
  return black;
}

Mat matcher::thresholding(Mat inputImg){
  for( int i = 0; i < inputImg.rows; i++ ){
    for( int j = 0; j < inputImg.cols; j++ ){
      if( inputImg.at<uchar>(i, j) == 150 ){
        inputImg.at<uchar>(i, j) = 255;
      } else {
        inputImg.at<uchar>(i, j) = 0;
      }
    }
  }
  return inputImg;
}

Mat matcher::centerFinding(Mat inputImg, int tipoDial){
  int noPuntosHor = 0, noPuntosVer = 0, noPuntosHorMax = 0, noPuntosVerMax = 0;
  int x, y;
  double pi = 3.1415926535;

  for( int i = 0; i < inputImg.rows; i++ ){
      noPuntosHor = 0;
    for( int j = 0; j < inputImg.cols; j++ ){
      if(inputImg.at<uchar>(Point( j, i )) == 0){
        noPuntosHor += 1;
      }
    }

    if( noPuntosHor > noPuntosHorMax ){
      noPuntosHorMax = noPuntosHor;
      y = i;
    }
  }

  for( int i = 0; i < inputImg.cols; i++ ){
    noPuntosVer = 0;
    for( int j = 0; j < inputImg.rows; j++ ){
      if(inputImg.at<uchar>(Point( i, j )) == 0){
        noPuntosVer += 1;
      }
    }

    if( noPuntosVer > noPuntosVerMax ){
      noPuntosVerMax = noPuntosVer;
      x = i;
    }
  }

  double euclideanDistance = 0, maxED = 0;
  int x1 = 0, y1 = 0;
  for( int i = 0; i < inputImg.rows; i++ ){
    for( int j = 0; j < inputImg.cols; j++ ){
      if( inputImg.at<uchar>( Point( j, i ) ) == 255 ){
        euclideanDistance = sqrt( pow( ( j - x ), 2 ) + pow( ( i - y ), 2 ) );
        if( euclideanDistance > maxED ){
          maxED = euclideanDistance;
          x1 = j;
          y1 = i;
        }
      }
    }
  }

  double numbers[ 6 ] = { pi / 2, 2 * pi / 5, pi / 5, - pi / 5, - 2 * pi / 5, - pi / 2 };
  double hyp = sqrt( pow( ( x1 - x ), 2 ) + pow( ( y - y1 ), 2 ) );

  double teta1 = atan( (double)( y - y1 ) / ( x1 - x ) );
  double teta2 = asin( (double)( y - y1 ) / hyp );
  double teta3 = acos( (double)( x1 - x ) / hyp );

  int lectura = 0;

  for( int i = 0; i <= 5; i ++){
    if( teta1 <= numbers[ i ] && teta1 > numbers[ i + 1 ]){
      if( teta1 > 0 && teta2 < 0 ){
        lectura = i + 5;
        continue;
      }
      if( teta1 < 0 && teta2 > 0){
        lectura = i + 5;
        continue;
      }
      lectura = i;
      continue;
    }
  }

  if( tipoDial == 2 ){
    lectura = 9 - lectura;
  }

  printf("%d\n", lectura);

  circle( inputImg, Point(x, y), 3, Scalar( 150, 150, 150, 150), 1, 8, 0);
  circle( inputImg, Point(x1, y1), 3, Scalar( 0, 0, 0, 0), 1, 8, 0);
  puts("");
  return inputImg;
}

//Mat matcher::centerFinding(Mat inputImg){
//  int r = 0, noPuntosNegro =0, maxPuntosNegro = 0, noPuntosBlanco = 0, maxPuntosBlanco = 0;
//  Point init, fin, center, initMax, finMax;
//  for( int i = 0; i < inputImg.rows; i++ ){
//    r = 0;
//    noPuntosNegro = 0;
//    noPuntosBlanco = 0;
//    for( int j = 0; j < inputImg.cols; j++ ){
//      if( inputImg.at<uchar>(Point( j, i )) == 0 && inputImg.at<uchar>(Point( j + 1, i )) == 255 && r == 0){
//        r += 1;
//      }
//      if( inputImg.at<uchar>(Point( j, i )) == 255 && inputImg.at<uchar>(Point( j + 1, i )) == 0 && r == 1){
//        r += 1;
//        init = Point( j, i );
//      }
//      if( inputImg.at<uchar>(Point( j, i )) == 0 && inputImg.at<uchar>(Point( j + 1, i )) == 255 && r == 2){
//        r += 1;
//        fin = Point( j, i );
//      }
//      if( inputImg.at<uchar>(Point( j, i )) == 255 && inputImg.at<uchar>(Point( j + 1, i )) == 0 && r == 3){
//        r += 1;
//      }
//      if( r == 2 ){
//        noPuntosNegro += 1;
//      }
//      if( r == 1 || r == 3 ){
//        noPuntosBlanco += 1;
//      }
//    }
//    if( r == 4 ){
//      if( noPuntosNegro < maxPuntosNegro && noPuntosBlanco < maxPuntosBlanco ){
//        continue;
//      }else {
//        initMax = init;
//        finMax = fin;
//        maxPuntosNegro = noPuntosNegro;
//        maxPuntosBlanco = noPuntosBlanco;
//        center = Point(( init.x + fin.x ) / 2, ( init.y + fin.y ) / 2);
//      }
//    }
//  }
//  circle( inputImg, initMax, 3, Scalar( 150, 150, 150, 150), 1, 8, 0);
//  circle( inputImg, finMax, 3, Scalar( 150, 150, 150, 150), 1, 8, 0);
//  circle( inputImg, center, 3, Scalar( 150, 150, 150, 150), 1, 8, 0);
//  return inputImg;
//}
