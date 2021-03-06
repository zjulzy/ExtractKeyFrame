// video2.cpp: 定义控制台应用程序的入口点。
//
/*
 * 由一个类专门表示聚类，成员变量有centroid（string）表示质心成员变量还有一个hash数组，表示这个聚类中的一帧。包含计算centroid的方法和添加成员帧的方法
 * 有一个专门的类表示Hash，其中包含比较两个hash的函数
 */

#include "stdafx.h"

#include <iostream>
#include "cv.h"
#include "opencv2/opencv.hpp"

const int THRESHOLD=10;
using namespace std;
using namespace cv;

class ImageHash {
private:
	bool value[64];
	ImageHash *next;
	int index;
public:
	//
	void Printvalue() {
		for (int i = 0;i < 64;i++) {
			printf("%d", value[i]);
		}
		printf("\n");
	}
	//constructor
	ImageHash() {
		for (int i = 0;i < 64;i++) {
			value[i] = true;
		}
		next = nullptr;
	}
	//constructor with a picture
	ImageHash(Mat frame, int _index) {
		index = _index;
		int pixel_average = 0, arr[64], num_not_zero = 0;
		Mat frame_small, frame_grey;
		//调整图片的大小与灰度
		resize(frame, frame_small, Size(8, 8), 0, 0, INTER_LINEAR);
		cvtColor(frame_small, frame_grey, CV_BGR2GRAY);
		for (int i = 0; i < 8; i++) {
			//将图片信息转换为一个字符串
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
		pixel_average /= 64;
		for (int i = 0; i < 64; i++) {
			value[i] = (arr[i] >= pixel_average) ? true : false;
		}
		next = nullptr;
	}
	//calculate the number of different bit in ImageHash
	int diff(ImageHash _ImageHash) {
		int ret=0;
		for (int i = 0;i < 64;i++) {
			if (value[i] != _ImageHash.value[i]) {
				ret++;
			}
		}
		return ret;
	}
	//set and get function to manipulate the member variables
	int getIndex() {
		return index;
	}
	ImageHash* getNext() {
		return next;
	}
	void setNext(ImageHash* _next) {
		next = _next;
	}
	bool getValueByIndex(int index) {
		return value[index];
	}
	void setValueByIndex(int index, bool _value) {
		value[index] = _value;
	}
	//
	ImageHash& operator =(ImageHash& a) {
		for (int i = 0;i < 64;i++) {
			this->value[i] = a.value[i];
		}
		return *this;
	}
};

class ImageCluster {
private:
	ImageHash centroid;
	ImageHash *member;
	ImageHash *tail;
	ImageCluster *next;
	int numImageInCluster;
public:
	//constructor
	ImageCluster() {
		member = new ImageHash;
		tail = member;
		member->setNext(nullptr);
		next = nullptr;
		numImageInCluster = 0;
		setCentroid();
	}
	int getKeyFrameInCluster() {
		
		ImageHash *Ptr = member;
		ImageHash *minPtr=member->getNext();//point to the most similar frame to the Centroid
		int diff = 65;
		while (member->getNext() != nullptr) {
			int currentDiff = centroid.diff(*(member->getNext()));
			if (currentDiff < diff) {
				minPtr = member->getNext();
				diff = currentDiff;
			}
			member = member->getNext();
		}
		if (minPtr == nullptr) {
			printf("The Cluster is null\n");
			return 0;
		}
		return minPtr->getIndex();
	}
	//set and get function
	void setNext(ImageCluster* _next) {
		next = _next;
	}
	ImageCluster* getNext() {
		return next;
	}
	int getNumImageInCluster() {
		return numImageInCluster;
	}
	void setCentroid() {
		int zero[64], one[64];
		for (int i = 0;i < 64;i++) {
			zero[i] = 0; one[i] = 0;
		}
		ImageHash* Ptr = member;
		/*if (Ptr->getNext() != nullptr) {
			Ptr->getNext()->Printvalue();
		}*/
		/**************************************************************/
		while (Ptr->getNext() != nullptr) {
			for (int i = 0;i < 64;i++) {
				if (Ptr->getNext()->getValueByIndex(i) == false) {
					zero[i]++;
				}
				else if (Ptr->getNext()->getValueByIndex(i) == true) {
					one[i]++;
				}
			}
			Ptr = Ptr->getNext();
		}
		bool flag = false;
		for (int i = 0;i < 64;i++) {
			if (zero[i] > one[i]) {
				centroid.setValueByIndex(i, false);
			}
			else if(zero[i] < one[i]){
				centroid.setValueByIndex(i, true);
			}
			else {
				if (flag == false) {
					centroid.setValueByIndex(i, false);
				}
				else if (flag == true) {
					centroid.setValueByIndex(i, true);
				}
				flag = !flag;
			}
		}
		/*for (int i = 0;i < 64;i++) {
			printf("%d", centroid.getValueByIndex(i));
		}
		printf("\n");*/
	}

	//judge whether a ImageHash is in the cluster
	bool judgeInCluster(ImageHash h) {
		bool ret;
		if (h.diff(centroid) < THRESHOLD) {
			ret = true;
		}
		else {
			ret = false;
		}
		return ret;
	}
	void addHashInCluster(ImageHash* h) {
		tail->setNext(h);
		tail = h;
		setCentroid();
		numImageInCluster++;
	}
	
	void printCount() {
		printf("numInageInCluster: %d\n", numImageInCluster);
	}
};

class  ClusterList {
private:
	int ClusterNum;
	ImageCluster *Head;
	ImageCluster *tail;
public:
	ClusterList() {
		ImageCluster *cell = new ImageCluster;
		Head = cell;
		tail = cell;
		//printf("-1\n");
	}
	void addCluster(ImageCluster* c) {
		ClusterNum++;
		tail->setNext(c);
		tail = c;
	}
	ImageCluster* judgeInExitedCluster(ImageHash h) {
		ImageCluster *Ptr=Head;
		ImageCluster *ret=nullptr;
		while (Ptr->getNext()!=nullptr) {
			if (Ptr->getNext()->judgeInCluster(h)) {
				ret = Ptr->getNext();
				break;
			}
			Ptr = Ptr->getNext();
		}
		return ret;
	}
	void printClusterCount() {
		ImageCluster *Ptr=Head;
		/*while (Ptr->getNext() != nullptr) {
			Ptr->getNext()->printCount();
			Ptr - Ptr->getNext();
		}*/
		printf("ClusterNum: %d\n", ClusterNum);
	}
	int getClusterNum() {
		return ClusterNum;
	}
	ImageCluster* getHead() {
		return Head;
	}
} ;


// 描述：将视频帧转成图片输出
void main()
{
	// 获取视频文件
	VideoCapture cap("C:\\Users\\3160105367\\Desktop\\video\\movie.mp4");

	// 获取视频总帧数
	long total_frame_number = cap.get(CV_CAP_PROP_FRAME_COUNT);
	cout << "total frames: " << total_frame_number << endl;

	Mat frame[5000];
	bool flags = true;
	long currentFrame = 0, totalFrame = 0;
	//printf("0\n");
	ClusterList Clist;
	int ss = 0;
	while (flags) {
		// 读取视频每一帧
		cap.read(frame[currentFrame]);
		stringstream str;
		str << "cqh" << currentFrame << ".jpg";
		//cout << "正在处理第" << currentFrame << "帧" << endl;
		//printf("\n");

		// 设置每30帧获取一次帧
		if (currentFrame % 6 == 0) {
			totalFrame++;
			if (Clist.getClusterNum() == 18 && ss == 2) {
				printf("here\n");
			}
			ImageHash *h = new ImageHash(frame[currentFrame], currentFrame);
			//h.Printvalue();
			ImageCluster *exitedClusterReceived= Clist.judgeInExitedCluster(*h);
			if (exitedClusterReceived!=nullptr) {//whether join 
				exitedClusterReceived->addHashInCluster(h);
			}
			else {
				ImageCluster* cell=new ImageCluster;
				cell->addHashInCluster(h);
				//printf("2");
				//create new cluster
				Clist.addCluster(cell);
				//add cluster to the cluster list
			}
			//cout << str.str() << " " << hash << endl;
			Clist.printClusterCount();
		}
		// 结束条件
		

		//imwrite("C:\\Users\\3160105367\\Desktop\\video\\result2\\" + str.str(), frame);
		if (currentFrame >= total_frame_number) {
			flags = false;
		}
		currentFrame++;
	}
	printf("total frame: %ld\n", totalFrame);
	printf("total cluster: %d\n", Clist.getClusterNum());
	//find key cluster
	//stringstream name;
	ImageCluster* Ptr=Clist.getHead();
	while (Ptr->getNext() != nullptr) {
		//printf("%d\n", );
		if (Ptr->getNext()->getNumImageInCluster() > totalFrame / (Clist.getClusterNum())) {
			int index = Ptr->getNext()->getKeyFrameInCluster();
			printf("output file\n");
			stringstream name;
			name << index << ".jpg";
			imwrite("C:\\Users\\3160105367\\Desktop\\video\\result2\\" + name.str(), frame[index]);
		}
		Ptr = Ptr->getNext();
	}

	//find key frame in key cluster, output the key frame
	system("pause");
}


