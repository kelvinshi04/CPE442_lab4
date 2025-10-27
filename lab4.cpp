#include <iostream>
#include <stdlib.h>
#include <string>
#include <pthread.h>
#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/core/mat.hpp>

using namespace std;
using namespace cv;

void *thread_routine(void *);
Mat to_sobel(Mat); 


int main(int argc, char** argv){
    //ensure number of arguments are correct
    if (argc != 2){
        cout << "Error: Invalid input" << endl;
        return 0;
    }
    string videoPath = argv[1];
    VideoCapture cap(videoPath);

    if (!cap.isOpened()){ 
        perror("Error: Could not open video file.");
        return 0;
    }

    //create threads
    pthread_t thread[4];
    for (int i = 0; i < 4; i++){
        // check for thread creation, calling pthread_create starts thread
        // TODO: create args for the threads to pass each quadrant of image for sobel function
        if (pthread_create(&thread[i], NULL, &thread_routine, NULL) != 0) {
            perror("Error: Failed to create thread");
            return 0;
        }
    }

    for (int i = 0; i < 4; i++){
        // wait for threads to finish
        if (pthread_join(thread[i], NULL) != 0){

        }
    }

    /* Implementation: Due to stitching issues, make each frame one pixel larger 
       on all ends when broken into 4ths and then trim and sitch back together*/

    //  TODO: move to thread
    Mat frame;
    while (cap.read(frame)){
        flip(frame, frame, -1);
        Mat sobel = to_sobel(frame);
        imshow("gImage", sobel);
        waitKey(1);
    }

    return 0;
}

void *thread_routine(void *) {
    
}




// Perform grayscale and sobel operations
Mat to_sobel(Mat frame){
    //Given a colored image, return a grayscale version using ITU-R (BT.709) algorithm
    Mat sobel(frame.rows, frame.cols, CV_8UC1, Scalar(0));
    for (int r = 0; r < frame.rows; r++){
        Vec3b* row = frame.ptr<Vec3b>(r);
        uchar* gRow = sobel.ptr<uchar>(r);
        for (int c = 0; c < frame.cols; c++){
            Vec3b pixel = row[c];
            gRow[c] = pixel[0]*0.0722 + pixel[1]*0.7152 + pixel[2]*0.2126;
        }
    }

    // Apply the sobel filter on a grayscale image
    int16_t xtotal, ytotal, total;
    for (int r = 1; r < frame.rows-1; r++){
        uchar* topRow = frame.ptr<uchar>(r-1);
        uchar* midRow = frame.ptr<uchar>(r);
        uchar* botRow = frame.ptr<uchar>(r+1);
        uchar* sRow = sobel.ptr<uchar>(r);
        for (int c = 1; c < frame.cols-1; c++){
            // Apply sobel filter
            xtotal = -topRow[c-1] + topRow[c+1] - midRow[c-1]*2 + midRow[c+1]*2 - botRow[c-1] + botRow[c+1];
            ytotal = topRow[c-1] + topRow[c]*2 +topRow[c+1] - botRow[c-1] - botRow[c]*2 - botRow[c+1];
            total = abs(xtotal) + abs(ytotal);
            total = (total > 255) ? 255 : ((total < 0) ? 0 : total);
            sRow[c] = total;
        }
    }
    return sobel;
}


