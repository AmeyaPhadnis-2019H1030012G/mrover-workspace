#include "perception.hpp"


using namespace std;
using namespace cv;

static Mat HSV;
static Mat DEPTH;
/* For debug use: print the HSV values at mouseclick locations */
void onMouse(int event, int x, int y, int flags, void* userdata){
  

    if(event == EVENT_LBUTTONUP){

        Vec3b p = HSV.at<Vec3b>(y,x);
        float d = DEPTH.at<float>(y,x);
        printf("Get mouse click at (%d, %d), HSV value is H: %d, S: %d, V:%d, depth is %.2f meters \n", y, x,
	                        p.val[0], p.val[1], p.val[2], d);
    }
}


Mat greenFilter(const Mat& src){
    assert(src.type() == CV_8UC3);

    Mat greenOnly;
    // 36 170 80
    Scalar lowerb = Scalar(30, 170, 80);
    Scalar upperb = Scalar(43, 226, 220);
    inRange(src, lowerb, upperb, greenOnly);

    return greenOnly;
}


vector<Point2f> findTennisBall(Mat &src, Mat & depth_src){
  
    Mat hsv;

    cvtColor(src, hsv, COLOR_BGR2HSV);

    Mat mask = greenFilter(hsv);

    #if PERCEPTION_DEBUG
    DEPTH = depth_src;
    HSV = hsv;
    DEPTH = depth_src;
    setMouseCallback("image", onMouse);    
    imshow("mask", mask);
    #endif
    
    // smoothing
    //medianBlur(mask, mask, 11);
    Size ksize(5,5);
    GaussianBlur(mask, mask, ksize, 1, 1, BORDER_DEFAULT );

    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;

    /// Find contours
    findContours( mask, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );

    vector<vector<Point> > contours_poly( contours.size() );
    vector<Point2f>center( contours.size() );
    vector<float>radius( contours.size() );

    for( unsigned i = 0; i < contours.size(); i++ ){
        approxPolyDP( Mat(contours[i]), contours_poly[i], 3, true );
        minEnclosingCircle( (Mat)contours_poly[i], center[i], radius[i] );
    }

    
    #if PERCEPTION_DEBUG
    // Draw polygonal contour + bonding rects + circles
    Mat drawing = Mat::zeros( mask.size(), CV_8UC3);
    for( unsigned i = 0; i< contours.size(); i++ ){
        Scalar color = Scalar(0, 0, 255);
        drawContours( src, contours_poly, i, color, 1, 8, vector<Vec4i>(), 0, Point() );
        //circle( src, center[i], (int)radius[i], color, 2, 8, 0 );
    }
    #endif
    
    if(center.size() > 0){
        vector<Point2f>biggestCircle( 1 );
        double biggestRadius =0;
        unsigned index= 0;
        
        for(unsigned i = 0; i< center.size(); i++ ){
            if(radius[i] > biggestRadius){
                biggestRadius = radius[i];
                biggestCircle[0] =center[i];
                index = i;
            }
        }
                Scalar color = Scalar(0, 0, 255);

        circle( src, center[index], (int)radius[index], color, 2, 8, 0 );
        return biggestCircle;
    }
    return center;
}
