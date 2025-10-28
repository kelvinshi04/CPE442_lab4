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
    int endIndex;
    uint8_t *status;
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
    pthread_t threads[4];
    struct args thr_args[4];
    Mat src, dest;
    uint8_t dest_create = 0, pthread_init = 0, status = 0;
    pthread_barrier_init(&barrierA, NULL, 5);
    pthread_barrier_init(&barrierB, NULL, 5);


    while (cap.read(src)){
        flip(src, src, -1);
        if (!dest_create){
            dest.create(src.rows, src.cols, CV_8UC1);
            dest_create = 1;
        }
        if (!pthread_init){
            // create pthread + arguments
            for (int i = 0; i < 4; i++){
                thr_args[i].source = &src;
                thr_args[i].dest = &dest;
                thr_args[i].startIndex = src.rows*i/4;
                thr_args[i].endIndex = src.rows*(i+1)/4;
                thr_args[i].status = &status;
                pthread_create(&threads[i], NULL, graySobel, (void *) &thr_args[i]);
            }
            pthread_init = 1;
        }
        // ready
        status = 1;
        pthread_barrier_wait(&barrierB);
        pthread_barrier_wait(&barrierA);
        imshow("sImage", dest);
        waitKey(1);
    }
    status = 0;
    pthread_barrier_wait(&barrierB);

    for (int i = 0; i < 4; i++){
        pthread_join(threads[i], NULL);
    }

    return 0;
}

void* graySobel(void *arg){
    while(true){
        pthread_barrier_wait(&barrierB);
        struct args *arguments = static_cast<struct args*>(arg);
        if (!(*(arguments->status))){
            pthread_exit(0);
        }
        int16_t xTotal, yTotal, g11, g12, g13, g21, g23, g31, g32, g33, total;
        for (int r = arguments->startIndex; r < arguments->endIndex; r++){
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

            for (int c = 0; c < arguments->source->cols; c++){
                // just perform grayscale on edge cases
                if (tRow == NULL || bRow == NULL || c == 0 || c == arguments->source->cols-1){
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
        pthread_barrier_wait(&barrierA);
    }
}
