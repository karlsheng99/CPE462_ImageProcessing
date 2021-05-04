#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <cmath>
#include <iostream>
#include <vector>

using namespace std;
using namespace cv;

vector<Point2f> srcPoints;
vector<Point2f> dstPoints;
int   j, k, width, height;
int ** img_in, ** img_out;
Mat image_in, image_in_copy;
double ratio_copy, ratio_trans;

void mousePoints(int event, int x, int y, int flag, void* userdata) {
    if (event==EVENT_LBUTTONUP) {
        circle(image_in_copy, Point2f(x, y), 8, Scalar(0, 255, 0), FILLED);
        imshow("Input Image", image_in_copy);
        cout << "Points position: (" << x << ", " << y << ")" << endl;
        srcPoints.push_back(Point2f(x, y));
        if (srcPoints.size() == 1) {
            cout << "Click on the top-right point." << endl;
        }
        else if (srcPoints.size() == 2) {
            cout << "Click on the bottom-right point." << endl;
        }
        else if (srcPoints.size() == 3) {
            cout << "Click on the bottom-left point." << endl;
        }
    }
    else if (srcPoints.size() == 4) {
        destroyWindow("Input Image");
    }
}

void set_srcPoints() {
    for (int i=0; i<4; i++) {
        srcPoints.at(i).x /= ratio_copy;
        srcPoints.at(i).y /= ratio_copy;
    }
}

void set_dstPoints() {
    double x1, x2, y1, y2;
    x1 = sqrt(pow(srcPoints.at(0).x - srcPoints.at(1).x, 2) + pow(srcPoints.at(0).y - srcPoints.at(1).y, 2));
    x2 = sqrt(pow(srcPoints.at(2).x - srcPoints.at(3).x, 2) + pow(srcPoints.at(2).y - srcPoints.at(3).y, 2));
    y1 = sqrt(pow(srcPoints.at(0).x - srcPoints.at(3).x, 2) + pow(srcPoints.at(0).y - srcPoints.at(3).y, 2));
    y2 = sqrt(pow(srcPoints.at(1).x - srcPoints.at(2).x, 2) + pow(srcPoints.at(1).y - srcPoints.at(2).y, 2));
    width = (int)((x1 + x2) / 2);
    height = (int)((y1 + y2) / 2);
    dstPoints.push_back(Point2f(0, 0));
    dstPoints.push_back(Point2f(width, 0));
    dstPoints.push_back(Point2f(width, height));
    dstPoints.push_back(Point2f(0, height));
}

int global_threshold() {
    double old_T = 255.0/2;
	double new_T = 0.0;
	double delta_T= 0.0;
	double sum1 = 0.0, sum2 = 0.0;
	double count1 = 0.0, count2 = 0.0;
	int a = 1;

	do {
		for (j=0; j<height; j++) {
			for (k=0; k<width; k++) {
				if (img_out[j][k] < old_T) {
					sum1 += img_out[j][k];
					count1 += 1;
				}
				else {
					sum2 += img_out[j][k];
					count2 += 1;
				}
			}
		}

        if (count1 == 0) {
            new_T = (old_T / 2 + sum2 / count2) /2;
        }
        else if (count2 == 0) {
            new_T = (sum1 / count1 + old_T / 2) / 2;
        }
        else {
		    new_T = (sum1 / count1 + sum2 / count2) / 2;
        }
		printf("Updated threshold T: %.2f, previous threshold T: %.2f\n", new_T, old_T);

		delta_T = abs(new_T - old_T);
		old_T = new_T;
	} while (delta_T >= a);

	return (int)new_T;
}

void edge_detection() {
    int num = 3;
    int p1[num][num] = {{-1, -1, -1}, {0, 0, 0}, {1, 1, 1}};
    int p2[num][num] = {{-1, 0, 1}, {-1, 0, 1}, {-1, 0, 1}};

    for (j=0; j<height; j++) {
		for (k=0; k<width; k++) {
			if (j==0 || k==0 || j==height-1 || k==width-1) {
				img_out[j][k] = 0;
			}
            else {
                int Gx = 0, Gy = 0;
                for (int m=0; m<num; m++) {
					for (int n=0; n<num; n++) {
						Gx += img_in[j-1+m][k-1+n] * p1[m][n];
                        Gy += img_in[j-1+m][k-1+n] * p2[m][n];
					}
				}
                img_out[j][k] = abs(Gx) + abs(Gy);
            }
        }
    }

    int t = global_threshold();
    for (j=0; j<height; j++) {
		for (k=0; k<width; k++) {
			if (img_out[j][k] >= t){
				img_out[j][k] = 255;
			}
			else {
				img_out[j][k] = 0;
			}
		}
	}
}

bool initialize_img() {
    img_in = (int**) calloc(height, sizeof(int*));
	if(!img_in)
	{
		return(false);
	}

	img_out = (int**) calloc(height, sizeof(int*));
	if(!img_out)
	{
		return(false);
	}

	for (j=0; j<height; j++)
	{
		img_in[j] = (int *) calloc(width, sizeof(int));
		if(!img_in[j])
		{
			return(false);
		}

		img_out[j] = (int *) calloc(width, sizeof(int));
		if(!img_out[j])
		{
			return(false);
		}
	}
    return true;
}

void delete_img() {
    for (j=0; j<height; j++)
	{
		free(img_in[j]);
		free(img_out[j]);
	}
	free(img_in);
	free(img_out);
}

int main(int argc, char *argv[]) {

/********************************************************************/
/*  Read image                                                      */
/********************************************************************/
    image_in = imread(argv[1]);
    image_in_copy = imread(argv[1]);
    ratio_copy = min(800.0/image_in.rows, 800.0/image_in.cols);
    resize(image_in_copy, image_in_copy, Size(0,0), ratio_copy, ratio_copy);
    namedWindow("Input Image");
    imshow("Input Image", image_in_copy);
    setMouseCallback("Input Image", mousePoints, NULL);
    cout << "Click on the top-left point." << endl;
    waitKey(0);
    set_dstPoints();
    set_srcPoints();

/********************************************************************/
/*  Perspective transform                                           */
/********************************************************************/

    Mat image_trans(Size(width, height), CV_64FC1);
    Mat transMat = getPerspectiveTransform(srcPoints, dstPoints);
    warpPerspective(image_in, image_trans, transMat, image_trans.size());

    if (max(width, height) < 500) {
        ratio_trans = min(500.0/image_trans.rows, 500.0/image_trans.cols);
        resize(image_trans, image_trans, Size(0,0), ratio_trans, ratio_trans);
        width = image_trans.cols;
        height = image_trans.rows;
    }

    namedWindow("Perspective Transform Image");
    imshow("Perspective Transform Image", image_trans);
    waitKey(0);
    destroyWindow("Perspective Transform Image");

/********************************************************************/
/*  Convert to 2D array                                             */
/********************************************************************/
    Mat_<uchar> image_gray(width, height);
	cvtColor(image_trans, image_gray, COLOR_BGR2GRAY);
    namedWindow("Grayscale Image");
    imshow("Grayscale Image", image_gray);
    waitKey(0);
    destroyWindow("Grayscale Image");

    while (!initialize_img()) {
        cerr << "Error: Can't allocate memmory!" << endl;
        return 1;
    }

	for (j=0; j<height; j++) {
		for (k=0; k<width; k++) {
			img_in[j][k] = image_gray(j,k);
		}
	}

/********************************************************************/
/*  Image processing                                                */
/********************************************************************/

    edge_detection();

/********************************************************************/
/*  Save image                                                      */
/********************************************************************/
    Mat_<uchar> image_out(height, width);
	for (j=0; j<height; j++) {
		for (k=0; k<width; k++) {
			image_out(j, k) = img_out[j][k];
		}
	}

	namedWindow("Output Image");
	imshow("Output Image", image_out);
	waitKey(0);
	destroyWindow("Output Image");

	bool isSuccess = imwrite(argv[2], image_out);
	if (!isSuccess) {
		cout << "Failed to save the image" << endl;
		return 1;
	}

    delete_img();
    return 0;
}
