#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <opencv2/opencv.hpp> 
#include <opencv2/highgui/highgui.hpp>   
#include <opencv2/imgproc/imgproc.hpp>   
#include <opencv2/core/core.hpp>
#include <filesystem>
#include <vector>

using namespace cv;
namespace fs = std::experimental::filesystem;
using marray = std::vector<std::vector<int>>;
using std::cout;
using std::endl;

int imgNum = 744;
int testNum = 100;
int trainNum = 644;
int boxNum = 817;

string rootServer = "/home/crb/datasets/wajueji/images/";

string intToStrLen5(int i)
{
	std::stringstream ss;
	string s;
	char t[256];
	sprintf_s(t, "%05d", i);
	s = t;
	return s;
}

int wmain(int argc, wchar_t*argv[])
{
	fs::path rootFolder = argc == 1 ? L"./" : argv[1];
	if (!fs::exists(rootFolder) || !fs::is_directory(rootFolder))
	{
		throw std::runtime_error("Not a folder");
	}
	cout << "working directory: " << endl << rootFolder.string() << endl << endl;

	fs::path imgOldFolder = rootFolder / L"image";
	fs::path txtOldFolder = rootFolder / L"label";

	fs::path imgNewFolder = rootFolder / L"images";
	fs::path txtNewFolder = rootFolder / L"labels";

	fs::create_directories(imgNewFolder);
	fs::create_directories(txtNewFolder);

	std::ofstream ftrain(rootFolder.string() + "/train.txt", std::ios::app | std::ios::out);
	std::ofstream ftest(rootFolder.string() + "/test.txt", std::ios::app | std::ios::out);

	int count = 0;

	for (auto&fe : fs::directory_iterator(txtOldFolder))
	{
		count++;
		if (count % 100 == 0) {
			cout << count << endl;
		}

		fs::path fp = fe.path();
		if (fs::is_regular_file(fp))
		{
			std::wstring fn = fp.stem();
			string txtFile = fp.string();
			//cout << txtFile << std::endl;

			std::ifstream inFile(txtFile);
			if (!inFile.is_open()) {
				cout << "can not open txt:" << endl << txtFile << endl;
				//throw std::runtime_error("No txt file");
				continue;
			}

			fs::path imgPath = imgOldFolder / (fn + L".jpg");
			fs::path imgSave = imgNewFolder / (fn + L".jpg");
			if (!fs::exists(imgPath)) {
				cout << "image do not exist:" << endl << imgPath.string() << endl;
				//throw std::runtime_error("No img file");
				continue;
			}
			string imgName = imgPath.string();
			//cout << imgName << endl;

			Mat img = imread(imgName);
			if (!img.data) {
				cout << "can not open image:" << endl << imgName << endl;
				//throw std::runtime_error("No img");
				continue;
			}

			int col = img.cols;
			int row = img.rows;

			//cout << col << ' ' << row << endl;

			vector<int> labelIn;
			int n;
			while (inFile >> n) {
				labelIn.push_back(n);//x1 y1 x2 y2
			}

			if (labelIn.size() % 5 != 0) {
				cout << "error label file:" << endl << txtFile << endl;
				//throw std::runtime_error("label file error");
				continue;
			}


			imgNum++;
			string name = intToStrLen5(imgNum);

			std::ofstream outFile(txtNewFolder.string() + "/" + name + ".txt");
			vector<double> lableOut;

			for (size_t i = 0; i < labelIn.size() / 5; ++i) {
				int index = i * 5;
				// x y w h
				int x1 = labelIn[index + 1];
				int y1 = labelIn[index + 2];
				int x2 = labelIn[index + 3];
				int y2 = labelIn[index + 4];

				if (x1 == 0 && x2 == 0 && y1 == 0 && y2 == 0) {
					cout << "error label: " << endl << txtFile << endl;
					//throw std::runtime_error("label file error");
					continue;
				}

				if (x2 - x1<10 || y2 - y1<10) {
					cout << "checked label: " << endl << txtFile << endl;
					//throw std::runtime_error("label error");
					continue;
				}

				if (x1 < 0) x1 = 1;
				if (y1 < 0) y1 = 1;
				if (x2 > col) x2 = col - 2;
				if (y2 > row) y2 = row - 2;
				double x = (x1 + x2) / 2.0 / col;
				double y = (y1 + y2) / 2.0 / row;
				double w = (x2 - x1 - 4) / (double)col;
				double h = (y2 - y1 - 4) / (double)row;
				if (x<0 || x>1 || y<0 || y>1 || w<0 || w>1 || h<0 || h>1) {
					cout << "checked label: " << endl << txtFile << endl;
					//throw std::runtime_error("label error");
					continue;
				}
				outFile << 0 << ' ';
				outFile << x << ' ' << y << ' ' << w << ' ' << h << '\n';
				boxNum++;
			}

			fs::copy_file(imgPath, imgNewFolder / (name + ".jpg"));

			if (imgNum % 5 == 0) {
				testNum++;
				ftest << (rootServer + name + ".jpg\n");
			}
			else {
				trainNum++;
				ftrain << (rootServer + name + ".jpg\n");
			}

			inFile.close();
			outFile.close();

		}

	}

	cout << "total img num: " << imgNum << endl;
	cout << "test num: " << testNum << "    " << "train num: " << trainNum << endl;
	cout << "target num: " << boxNum << endl;

	ftrain.close();
	ftest.close();

	getchar();

	return 0;
}