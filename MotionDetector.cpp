#include <opencv2/opencv.hpp>
#include <iostream>
#include <time.h>
#include <chrono>
#include <ctime>  
#include <stdio.h>

using namespace cv;
using namespace std;

// Get the current date & time for naming the motion capture image.
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

// Resize, grayscale, and blur the frame for computational efficiency.
Mat preprocessFrame(Mat currentFrame) {
    resize(currentFrame, currentFrame, Size(640, 480), 0, 0);
    cvtColor(currentFrame, currentFrame, COLOR_BGR2GRAY);
    GaussianBlur(currentFrame, currentFrame, Size(11, 11), 0, 0);
    return currentFrame;
}

// Count the pixel differences greater than a grayscale value of
// 10 between image segments of the current and previous frame.
int countDifferences(Mat siPrev, Mat siCurr) {
    int numDif = 0;
    for (int x = 1; x <= 159; x++) {
        for (int y = 1; y <= 159; y++) {
            if (abs((int)siCurr.at<uchar>(x, y) - (int)siPrev.at<uchar>(x, y)) > 10) {
                numDif++;
            }
        } 
    }
    return numDif;
}

// Write the current frames to a MotionCapture file with the date
// and time stamps if the given pixel difference is above 3,000.
void detectMotion(int numDif, Mat frame) {
    if (numDif > 3000) {
        cout << "Motion Detected!" << endl;
        cout << "Difference: " << numDif << endl;
        string cDate = currentDateTime();
        string filepath = "../../MotionCaptures/" + cDate + ".jpg";
        cout << filepath << endl;
        imwrite(filepath, frame);
    }
}

// Breaks the current frame and previous frame into image segments
// and checks if motion is detected within that segment.
// Each point represents a segment of the image.
// Each image segment is 160x160.
// (1, 1)  (2, 1)  (3, 1)  (4, 1)
// (1, 2)  (2, 2)  (3, 2)  (4, 2)
// (1, 3)  (2, 3)  (3, 3)  (4, 4)
void segmentImage(Mat prevFrame, Mat frame) {
    for (int i = 0; i <= 480; i += 160) {
        for (int j = 0; j <= 320; j += 160) {
            Rect crop(i, j, 160, 160);
            Mat segmentPrev = prevFrame(crop);
            Mat segmentCurr = frame(crop);
            detectMotion(countDifferences(segmentPrev, segmentCurr), frame);
        }
    }
}

int main() {

    VideoCapture cap(0);
    if (!(cap.open(0))) {
        return 0;
    }

    Mat prevFrame = Mat::zeros(Size(640, 480), CV_8UC1);
    int numFrames = 0;

    while (true) {
        Mat frame;
        cap.read(frame);
        frame = preprocessFrame(frame);

        numFrames++;
        if (numFrames > 1) {
            segmentImage(prevFrame, frame);
        }
        prevFrame = frame;
    }
    cap.release();
    return 0;
}