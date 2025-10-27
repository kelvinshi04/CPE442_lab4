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

struct args{
    Mat *source;
    Mat *dest;
    int startIndex;

};

void* graySobel(void*);
pthread_barrier_t barrierA, barrierB;

int main(int argc, char **argv){
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

    Mat frame;
    while (cap.read(frame)){
        flip(frame, frame, -1);
        
        // imshow("gImage", );
        // waitKey(1);
    }

    return 0;
}

void* graySobel(void *arg){
    struct args *arguments = static_cast<struct args*>(arg);
    int16_t xTotal, yTotal, g11, g12, g13, g21, g23, g31, g32, g33, total;
    for (int r = arguments->startIndex; r < arguments->source->rows; r++){
        Vec3b *tRow, *mRow, *bRow;
        uchar *sRow;
        if ((r-1) < 0)
            tRow = NULL;
        else 
            tRow = arguments->source->ptr<Vec3b>(r-1);
        
        if ((r+1) > arguments->source->rows-1)
            bRow = NULL;
        else 
            bRow = arguments->source->ptr<Vec3b>(r+1);
        mRow = arguments->source->ptr<Vec3b>(r);
        sRow = arguments->dest->ptr<uchar>(r);

        for (int c = 0; c < arguments->source->rows; c++){
            // just perform grayscale on edge cases
            if (tRow == NULL || bRow == NULL || c == 0 || c == arguments->source->cols){
                sRow[c] = mRow[c][0]*0.0722 + mRow[c][1]*0.7152 + mRow[c][2]*0.2126;
            }
            else{
                g11 = tRow[c-1][0]*0.0722 + tRow[c-1][1]*0.7152 + tRow[c-1][2]*0.2126;
                g12 = tRow[c][0]*0.0722 + tRow[c][1]*0.7152 + tRow[c][2]*0.2126;
                g13 = tRow[c+1][0]*0.0722 + tRow[c+1][1]*0.7152 + tRow[c+1][2]*0.2126;
                g21 = mRow[c-1][0]*0.0722 + mRow[c-1][1]*0.7152 + mRow[c-1][2]*0.2126;
                g23 = mRow[c+1][0]*0.0722 + mRow[c+1][1]*0.7152 + mRow[c+1][2]*0.2126;
                g31 = bRow[c-1][0]*0.0722 + bRow[c-1][1]*0.7152 + bRow[c-1][2]*0.2126;
                g32 = bRow[c][0]*0.0722 + bRow[c][1]*0.7152 + bRow[c][2]*0.2126;
                g33 = bRow[c+1][0]*0.0722 + bRow[c+1][1]*0.7152 + bRow[c+1][2]*0.2126;

                xTotal = -g11 + g13 - g21*2 + g23*2 - g31 + g33;
                yTotal = g11 + g12*2 + g13 - g31 - g32*2 - g33;

                total = abs(xTotal) + abs(yTotal);
                total = (total > 255) ? 255 : ((total < 0) ? 0 : total);
                sRow[c] = total;
            }
            


        }
    }

};
