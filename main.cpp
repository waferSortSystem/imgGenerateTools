#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <iostream>
#include <fstream>  
#include <random>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <direct.h>
#include <errno.h>

using namespace std;
using namespace cv;

struct imgInfo
{
	int row; //行
	int col; //列
	string name; //图像名称
	string standardCoordinates; //标准坐标
	int offset_x; //x偏移
	int offset_y; //y偏移
	string actualCoordinates; //实际坐标
};

 
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

bool boundaryJudgment(int x, int y, int imgLength, int imgWidth, int center_x, int center_y, int radius)
{
	int x1 = x + imgLength;
	int y1 = y + imgWidth;
	// cout<<"x="<<x<<endl;
	// cout<<"y="<<y<<endl;
	// cout<<"x1="<<x1<<endl;
	// cout<<"y1="<<y1<<endl;
	if((x-center_x) * (x-center_x) + (y-center_y) * (y-center_y) < radius * radius)return true;
	if((x1-center_x) * (x1-center_x)  + (y-center_y) * (y-center_y) < radius * radius)return true;
	if((x-center_x) * (x-center_x) + (y1-center_y) * (y1-center_y) < radius * radius)return true;
	if((x1-center_x) * (x1-center_x) + (y1-center_y) * (y1-center_y) < radius * radius)return true;
	return false;
}

void generateImgFunction(int imgLength, int imgWidth, string sourceFilePath, string savePath, float coverage, float maxError, int maxLength, int maxWidth, int center_x, int center_y, int radius){

	deleteAllFiles(savePath);

	cout<<"start"<<endl;
	cv::Mat originImg;//原始图片
	cv::Rect selectArea;
	string path; 
	originImg = imread(sourceFilePath);
	int x = originImg.cols;//图片的长度
	int y = originImg.rows;//图片的宽度

	Mat originCompositeImg;//原始图片的合成图
	Mat tmp1, tmp2;
	tmp1 = landscapeStitching(originImg, originImg);
	tmp2 = landscapeStitching(tmp1, originImg);//横向拼接
	tmp1 = longitudinalStitching(tmp2, tmp2);
	originCompositeImg = longitudinalStitching(tmp1, tmp2);//纵向拼接
	imwrite("D://MyFiles//Code//C++//imgGenerateTools//origin.jpg", originCompositeImg);
	
	int cur_x = 0;
	int cur_y = 0;
	int step_x = imgLength * coverage;
	int step_y = imgWidth * coverage;

	default_random_engine e;
	uniform_real_distribution<double> u(-maxError, maxError);

	std::random_device rd;  //Will be used to obtain a seed for the random number engine
	std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
	std::uniform_int_distribution<> distrib(-maxError, maxError); // 指定范围

	int cur_row = 1;
	int cur_col = 1;
	int maxColNumber = maxLength / step_x + 1;
	int maxRowNumber = maxWidth / step_y + 1;

	if(maxLength % step_x == 0)--maxColNumber;
	if(maxWidth % step_y == 0)--maxRowNumber;

	ofstream ofs;						//定义流对象
    ofs.open("output.txt",ios::out);		//以写的方式打开文件
	ofs<<"总计*"<<maxRowNumber<<"*行*"<<maxColNumber<<"*列"<<endl;
    ofs<<"行"<<"*"<<"列"<<"*"<<"图像名称"<<"*"<<"标准坐标"<<"*"<<"x偏移"<<"*"<<"y偏移"<<"*"<<"偏移后坐标"<<endl;//写入
    
	for(cur_row; cur_row <= maxRowNumber; cur_row++){

		string command;
		command = "mkdir " + std::string("D:\\MyFiles\\Code\\C++\\imgGenerateTools\\split\\") + to_string(cur_row);
		system(command.c_str());
		
		for(cur_col; cur_col <= maxColNumber; cur_col++){
			Mat generateImg;
			path = savePath + "//" + to_string(cur_row) + "//" + to_string(cur_row) + "_" + to_string(cur_col) + ".jpg";
			int offset_x = distrib(gen);
			int offset_y = distrib(gen);
			if(cur_col == maxColNumber && cur_row == maxRowNumber){

				// selectArea = Rect(cur_x * (1 + u(e)), cur_y * (1 + u(e)), maxLength % step_x, maxWidth % step_y);
				selectArea = Rect(x + cur_x + offset_x, y + cur_y + offset_y, maxLength % step_x, maxWidth % step_y);
				originCompositeImg(selectArea).copyTo(generateImg);
				cv::copyMakeBorder(generateImg, generateImg, 0, imgWidth - maxWidth % step_y, 0, imgLength - maxLength % step_x, cv::BORDER_CONSTANT);
				if(boundaryJudgment((cur_col - 1) * step_x, (cur_row - 1) * step_y, imgLength, imgWidth, center_x, center_y, radius)){
					ofs<<cur_row<<"*"<<cur_col<<"*"<<to_string(cur_row) + "_" + to_string(cur_col) + ".jpg"<<"*"<<"(" + to_string((cur_col - 1) * step_x) + "," + to_string((cur_row - 1) * step_y) + ")"<<"*"<<offset_x<<"*"<<offset_y<<"*"<<"(" + to_string((cur_col - 1) * step_x + offset_x) + "," + to_string((cur_row - 1) * step_y + offset_y) + ")"<<endl;
					imwrite(path, generateImg);
				}

			}else if (cur_col == maxColNumber){

				// selectArea = Rect(cur_x * (1 + u(e)), cur_y * (1 + u(e)), maxLength % step_x, imgWidth);
				selectArea = Rect(x + cur_x + offset_x, y + cur_y + offset_y, maxLength % step_x, imgWidth);
				originCompositeImg(selectArea).copyTo(generateImg);
				cv::copyMakeBorder(generateImg, generateImg, 0, 0, 0, imgLength - maxLength % step_x, cv::BORDER_CONSTANT);
				if(boundaryJudgment((cur_col - 1) * step_x, (cur_row - 1) * step_y, imgLength, imgWidth, center_x, center_y, radius)){
					ofs<<cur_row<<"*"<<cur_col<<"*"<<to_string(cur_row) + "_" + to_string(cur_col) + ".jpg"<<"*"<<"(" + to_string((cur_col - 1) * step_x) + "," + to_string((cur_row - 1) * step_y) + ")"<<"*"<<offset_x<<"*"<<offset_y<<"*"<<"(" + to_string((cur_col - 1) * step_x + offset_x) + "," + to_string((cur_row - 1) * step_y + offset_y) + ")"<<endl;
					imwrite(path, generateImg);
				}

			}else if(cur_row == maxRowNumber){

				// selectArea = Rect(cur_x * (1 + u(e)), cur_y * (1 + u(e)), imgLength, maxWidth % step_y);
				selectArea = Rect(x + cur_x + offset_x, y + cur_y + offset_y, imgLength, maxWidth % step_y);
				originCompositeImg(selectArea).copyTo(generateImg);
				cv::copyMakeBorder(generateImg, generateImg, 0, imgWidth - maxWidth % step_y, 0, 0, cv::BORDER_CONSTANT);
				if(boundaryJudgment((cur_col - 1) * step_x, (cur_row - 1) * step_y, imgLength, imgWidth, center_x, center_y, radius)){
					ofs<<cur_row<<"*"<<cur_col<<"*"<<to_string(cur_row) + "_" + to_string(cur_col) + ".jpg"<<"*"<<"(" + to_string((cur_col - 1) * step_x) + "," + to_string((cur_row - 1) * step_y) + ")"<<"*"<<offset_x<<"*"<<offset_y<<"*"<<"(" + to_string((cur_col - 1) * step_x + offset_x) + "," + to_string((cur_row - 1) * step_y + offset_y) + ")"<<endl;
					imwrite(path, generateImg);
				}

			}else{

				// selectArea = Rect(cur_x * (1 + u(e)), cur_y * (1 + u(e)), imgLength, imgWidth);
				selectArea = Rect(x + cur_x + offset_x, y + cur_y +offset_y, imgLength, imgWidth);
				generateImg = originCompositeImg(selectArea);
				if(boundaryJudgment((cur_col - 1) * step_x, (cur_row - 1) * step_y, imgLength, imgWidth, center_x, center_y, radius)){
					ofs<<cur_row<<"*"<<cur_col<<"*"<<to_string(cur_row) + "_" + to_string(cur_col) + ".jpg"<<"*"<<"(" + to_string((cur_col - 1) * step_x) + "," + to_string((cur_row - 1) * step_y) + ")"<<"*"<<offset_x<<"*"<<offset_y<<"*"<<"(" + to_string((cur_col - 1) * step_x + offset_x) + "," + to_string((cur_row - 1) * step_y + offset_y) + ")"<<endl;
					imwrite(path, generateImg);
				}

			}

			// cur_x += step_x * (1 + u(e));
			cur_x += step_x;
			if(cur_x >= x)cur_x -= x;
		}
		// cur_y += step_y * (1 + u(e));
		cur_y += step_y;
		if(cur_y >= y)cur_y -= y;
		cur_col = 1;
		cur_x = 0;
	}
	ofs.close();
	return;
}

vector<imgInfo> readFile(string filename){
// void readFile(string filename){
	ifstream in(filename);
	string line;
	vector<vector<string>> vv;
	int count = 1;
	getline(in, line);
	stringstream _ss(line);
	string _tmp;
	vector<string> _v;
	while (getline(_ss, _tmp, '*')){//按“，”隔开字符串
		_v.push_back(_tmp);
	}
	int col = std::stoi(_v[3]);
	vector<imgInfo> imgInfoList(std::stoi(_v[1]) * std::stoi(_v[3]) + 1);
	getline(in, line);
	while (getline(in, line)){
		stringstream ss(line);
		string tmp;
		vector<string> v;
		while (getline(ss, tmp, '*')){//按“*”隔开字符串
			v.push_back(tmp);
		}
		imgInfo info;
		info.row = std::stoi(v[0]);
		info.col = std::stoi(v[1]);
		info.name = v[2];
		info.standardCoordinates = v[3];
		info.offset_x = std::stoi(v[4]);
		info.offset_y = std::stoi(v[5]);
		info.actualCoordinates = v[6];
		imgInfoList[info.row * col + info.col] = info;
		// vv.push_back(v);
	}
	// for (auto row : vv){
	// 	for (auto col : row){
	// 		cout << col << "\t";
	// 	}
	// 	cout << endl;
	// }
	// cout << endl;
	in.close();
	return imgInfoList;
}

int main()
{	
	int imgLength = 1000;
	int imgWidth = 1000;
	string sourceFilePath = "D://MyFiles//Code//C++//imgGenerateTools//1.bmp";
	string savePath = "D://MyFiles//Code//C++//imgGenerateTools//split";
	float coverage = 0.5;
	float maxError = 5;
	int maxLength = 6000;
	int maxWidth = 6000;
	int center_x = 2500; //圆心x
	int center_y = 2500; //圆心y
	int radius = 2500; //半径

	// generateImgFunction(imgLength, imgWidth, sourceFilePath, savePath, coverage, maxError, maxLength, maxWidth, center_x, center_y, radius);

	string filename = "D://MyFiles//Code//C++//imgGenerateTools//test.txt";
	vector<imgInfo> imgInfoList = readFile(filename);
	// for(int i = 0; i < imgInfoList.size(); ++i){
	// 	if(imgInfoList[i].name != ""){
	// 		cout<<i<<"--"<<imgInfoList[i].name<<endl;
	// 	}
	// }

	return 0;
}
