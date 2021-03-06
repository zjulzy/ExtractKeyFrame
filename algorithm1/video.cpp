// video.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include <iostream>
#include "cv.h"
#include "opencv2/opencv.hpp"

using namespace std;
using namespace cv;

// 描述：将视频帧转成图片输出
void main()
{
	// 获取视频文件
	VideoCapture cap("C:\\Users\\3160105367\\Desktop\\video\\movie.mp4");//set the url of your own video

	// 获取视频总帧数
	long total_frame_number = cap.get(CV_CAP_PROP_FRAME_COUNT);
	cout << "total frames: " << total_frame_number << endl;

	Mat frame;
	bool flags = true;
	long currentFrame = 0;
	char hash_previous[64];
	while (flags) {
		// 读取视频每一帧
		cap.read(frame);
		stringstream str;
		str << currentFrame << ".jpg";
		//cout << "正在处理第" << currentFrame << "帧" << endl;
		//printf("\n");
		int pixel_average=0, arr[64], num_not_zero=0;
		for (int i = 0;i < 64;i++) {
			arr[i] = 0;
		}
		// 设置每30帧获取一次帧
		if (currentFrame % 30 == 0) {
			Mat frame_small, frame_grey;
			resize(frame, frame_small, Size(8, 8), 0, 0, INTER_LINEAR);
			cvtColor(frame_small, frame_grey, CV_BGR2GRAY);
			
			for (int i = 0; i < 8; i++) {
				uchar* data = frame_grey.ptr<uchar>(i);
				int tmp = i * 8;
				for (int j = 0; j < 8; j++) {
					int tmp1 = tmp + j;
					arr[tmp1] = data[j] / 4 * 4;
					pixel_average += arr[tmp1];
					if (arr[tmp1] != 0) {
						num_not_zero++;
					}
				}
			}
			//cout << pixel_average << " ";
			
			cout << endl;
			pixel_average /= 64;
			char hash[64];
			for (int i = 0; i < 64; i++) {
				hash[i] = (arr[i] >= pixel_average) ? '1' : '0';
			}
			int diff_num=0;
			for (int i = 0;i < 64;i++) {
				if (hash[i] != hash_previous[i]) {
					diff_num++;
				}
			}
			if (diff_num > 10 && num_not_zero>30) {
				imwrite("C:\\Users\\3160105367\\Desktop\\video\\result\\" + str.str(), frame);//set the folder to save the keyframe
			}
			for (int i = 0;i < 64;i++) {
				hash_previous[i] = hash[i];
			}
			//cout << str.str() << " " << hash << endl;
		}
		// 结束条件
		if (currentFrame >= total_frame_number) {
			flags = false;
		}
		currentFrame++;
	}

	system("pause");
}


