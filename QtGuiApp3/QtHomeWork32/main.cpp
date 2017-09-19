#include "MainWindow.h"
//#include <QtWidgets/QApplication>
//#include <iostream> 
//#include <opencv2/core/core.hpp> 
//#include <opencv2/highgui/highgui.hpp> 

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	MainWindow w;
	w.show();
	return a.exec();

	//// 读入一张图片（游戏原画） 
	//Mat img = imread("D:/projC/QtGuiApp3/HomeWork32/TestImage/warp_test.bmp");
	//if (!img.data)
	//{
	//	printf("the img's data is null!\n");
	//	return 0;
	//}

	//// 创建一个名为 "游戏原画"窗口 
	//namedWindow("游戏原画");
	//// 在窗口中显示游戏原画 
	//imshow("游戏原画", img);
	//// 等待6000 ms后窗口自动关闭
	//waitKey(6000);
}
