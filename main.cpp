#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <iostream>

#include <random>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <direct.h>
#include <errno.h>

using namespace std;
using namespace cv;
 
void deleteAllFiles(string sourcePath){//删除文件夹下的所有子文件
	//在目录后面加上"\\*.*"进行第一次搜索
	string path = sourcePath + "\\*.*";
	//用于查找的句柄
	intptr_t handle;
	struct _finddata_t fileInfo;
	//第一次查找
	handle = _findfirst(path.c_str(), &fileInfo);
 
	if (handle == -1) {
		cout << "no files" << endl;
		// system("pause");
		return;
	}
 
	do
	{
		if (fileInfo.attrib & _A_SUBDIR) {//如果为文件夹，加上文件夹path，再次遍历
			if (strcmp(fileInfo.name, ".") == 0 || strcmp(fileInfo.name, "..") == 0)
				continue;
 
			// 在目录后面加上"\\"和搜索到的目录名进行下一次搜索
			path = sourcePath + "\\" + fileInfo.name;
			deleteAllFiles(path.c_str());//先遍历删除文件夹下的文件，再删除空的文件夹
			// cout << path.c_str() << endl;
			if (_rmdir(path.c_str()) == 0) {//删除空文件夹
				cout << "delete empty dir success" << endl;
			}
			else {
				cout << "delete empty dir error" << endl;
			}
		}
		else{
			string filePath = sourcePath + "\\" + fileInfo.name;
			cout << filePath.c_str() << endl;
			if (remove(filePath.c_str()) == 0) {//删除文件
				// cout << "delete file success" << endl;
			}else{
				cout << "delete file error" << endl;
			}
		}
	} while (!_findnext(handle, &fileInfo));
 
	_findclose(handle);
	return;
}

Mat landscapeStitching(Mat sourceImg1, Mat sourceImg2)//横向拼接
{
	int allCols = sourceImg1.cols + sourceImg2.cols;
	Mat resultImg(sourceImg1.rows, allCols, sourceImg1.type());
	Mat middleImg = resultImg.colRange(0, sourceImg1.cols);
	sourceImg1.copyTo(middleImg);
	middleImg = resultImg.colRange(sourceImg1.cols, allCols);
	sourceImg2.copyTo(middleImg);
	return resultImg;
}

Mat longitudinalStitching(Mat sourceImg1, Mat sourceImg2)//纵向拼接
{
	int allRows = sourceImg1.rows + sourceImg2.rows;
	Mat resultImg(allRows, sourceImg1.cols, sourceImg1.type());
	Mat middleImg = resultImg.rowRange(0, sourceImg1.rows);
	sourceImg1.copyTo(middleImg);
	middleImg = resultImg.rowRange(sourceImg1.rows, allRows);
	sourceImg2.copyTo(middleImg);
	return resultImg;
}

void generateImgFunction(int imgLength, int imgWidth, string sourceFilePath, string savePath, float coverage, float maxError, int maxLength, int maxWidth){

	deleteAllFiles(savePath);

	cout<<"start"<<endl;
	cv::Mat originImg;//原始图片
	cv::Rect selectArea;
	string path; 
	originImg = imread(sourceFilePath);
	int x = originImg.cols;//图片的长度
	int y = originImg.rows;//图片的宽度

	Mat originCompositeImg;//原始图片的合成图
	originCompositeImg = landscapeStitching(originImg, originImg);
	originCompositeImg = longitudinalStitching(originCompositeImg, originCompositeImg);
	
	int cur_x = 0;
	int cur_y = 0;
	int step_x = imgLength * coverage;
	int step_y = imgWidth * coverage;

	default_random_engine e;
	uniform_real_distribution<double> u(-maxError, maxError);

	int cur_row, cur_col;
	cur_row = 1;
	cur_col = 1;
	int maxColNumber = maxLength / step_x + 1;
	int maxRowNumber = maxWidth / step_y + 1;

	for(cur_row; cur_row <= maxRowNumber; cur_row++){

		string command;
		command = "mkdir " + std::string("D:\\MyFiles\\Code\\C++\\imgGenerateTools\\split\\") + to_string(cur_row);
		system(command.c_str());
		
		for(cur_col; cur_col <= maxColNumber; cur_col++){
			Mat generateImg;
			path = savePath + "//" + to_string(cur_row) + "//" + to_string(cur_row) + "_" + to_string(cur_col) + ".jpg";
			if(cur_col == maxColNumber && cur_row == maxRowNumber){

				selectArea = Rect(cur_x * (1 + u(e)), cur_y * (1 + u(e)), maxLength % step_x, maxWidth % step_y);
				originCompositeImg(selectArea).copyTo(generateImg);
				cv::copyMakeBorder(generateImg, generateImg, 0, imgWidth - maxWidth % step_y, 0, imgLength - maxLength % step_x, cv::BORDER_CONSTANT);
				imwrite(path, generateImg);

			}else if (cur_col == maxColNumber){

				selectArea = Rect(cur_x * (1 + u(e)), cur_y * (1 + u(e)), maxLength % step_x, imgWidth);
				originCompositeImg(selectArea).copyTo(generateImg);
				cv::copyMakeBorder(generateImg, generateImg, 0, 0, 0, imgLength - maxLength % step_x, cv::BORDER_CONSTANT);
				imwrite(path, generateImg);

			}else if(cur_row == maxRowNumber){

				selectArea = Rect(cur_x * (1 + u(e)), cur_y * (1 + u(e)), imgLength, maxWidth % step_y);
				originCompositeImg(selectArea).copyTo(generateImg);
				cv::copyMakeBorder(generateImg, generateImg, 0, imgWidth - maxWidth % step_y, 0, 0, cv::BORDER_CONSTANT);
				imwrite(path, generateImg);

			}else{

				selectArea = Rect(cur_x * (1 + u(e)), cur_y * (1 + u(e)), imgLength, imgWidth);
				generateImg = originCompositeImg(selectArea);
				imwrite(path, generateImg);

			}

			cur_x += step_x * (1 + u(e));
			if(cur_x >= x)cur_x -= x;
		}
		cur_y += step_y * (1 + u(e));
		if(cur_y >= y)cur_y -= y;
		cur_col = 1;
		cur_x = 0;
	}
	return;
}

int main()
{	
	int imgLength = 1024;
	int imgWidth = 1024;
	string sourceFilePath = "D://MyFiles//Code//C++//imgGenerateTools//1.jpg";
	string savePath = "D://MyFiles//Code//C++//imgGenerateTools//split";
	float coverage = 0.5;
	float maxError = 0.1;
	int maxLength = 6000;
	int maxWidth = 5000;

	generateImgFunction(imgLength, imgWidth, sourceFilePath, savePath, coverage, maxError, maxLength, maxWidth);

	return 0;
}
