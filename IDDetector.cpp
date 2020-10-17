#include <opencv2/opencv.hpp>
#include <iostream>
#include <time.h>
#include <chrono>
#include <ctime>  
#include <stdio.h>

using namespace cv;
using namespace std;

// Get the current date & time for naming the card save.
const std::string currentDateTime() {
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);
    // Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
    // for more information about date/time format
    strftime(buf, sizeof(buf), "%Y-%m-%d @ %Hh%Mm%Ss", &tstruct);

    return buf;
}

// Resize, blur, and filter by white and green HSV for computational efficiency.
Mat preprocessFrame(Mat frame) {
    resize(frame, frame, Size(640, 480), 0, 0);
    Mat blurred;
    GaussianBlur(frame, blurred, Size(11, 11), 0, 0);
    Mat hsv;
    cvtColor(blurred, hsv, COLOR_BGR2HSV);
    Mat maskWG;
    inRange(hsv, Scalar(91, 64, 44), Scalar(237, 220, 201), maskWG);
    return maskWG;
}

// Filters out contours that are noise by asserting certain conditions. Conditions include size and 
// height/width ratio requirements.
bool falseContour(size_t i, vector<vector<Point> > contours, vector<Rect> boundRect) {
    if (contourArea(contours[i]) <= 200.0) return true;
    if (((boundRect[(int)i].width) / 1.4) < (boundRect[(int)i].height)) return true;
    if (((boundRect[(int)i].width) * (boundRect[(int)i].height)) < 1000) return true;
    if ((((boundRect[(int)i].width) * (boundRect[(int)i].height)) * 0.5) > contourArea(contours[i])) return true;
    return false;
}

// Save the region of the image containing the detected student card with the corresponding date and
// time of capture.
void saveCard(Mat frame, vector<vector<Point> > contours, vector<Rect> boundRect, Mat drawing,
              vector<vector<Point> > contours_poly, size_t i) {
    drawContours( drawing, contours_poly, (int)i, Scalar(0, 255, 0) );
    rectangle( drawing, boundRect[i].tl(), boundRect[i].br(), Scalar(0, 255, 0), 2 );
    int X = (int)(boundRect[(int)i].x);
    int Y = (int)(boundRect[(int)i].y);
    int W = ((int)(boundRect[(int)i]).width);
    int H = (int)(boundRect[(int)i].height);
    Rect crop(X, abs(Y - 55), W + 80, H + 60);
    Mat cardFrame = frame(crop);

    string cDate = currentDateTime();
    string filepath = "../../IdentifiedCards/" + cDate + ".jpg";
    cout << "Student Card Identified @: " << cDate << endl;
    imwrite(filepath, cardFrame);
}

int main() {

    VideoCapture cap(0);
    if (!(cap.open(0))) {
        return 0;
    }

    while(true) {
        Mat frame;
        cap.read(frame);
        Mat maskWG = preprocessFrame(frame);
        
        vector<vector<Point> > contours;
        vector<Vec4i> hierarchy;
        findContours(maskWG, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
        vector<vector<Point> > contours_poly( contours.size() );
        vector<Rect> boundRect( contours.size() );

        Mat drawing = Mat::zeros(maskWG.size(), CV_8UC3 );
        for (size_t i = 0; i< contours.size(); i++) {
            approxPolyDP( contours[i], contours_poly[i], 3, true );
            boundRect[i] = boundingRect( contours_poly[i] );
            if (falseContour(i, contours, boundRect)) continue;
            saveCard(frame, contours, boundRect, drawing, contours_poly, i);
        }

        imshow("Feed", drawing);
        if (waitKey(1) >= 0) {
            break;
        }
    }
}