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

	//// ����һ��ͼƬ����Ϸԭ���� 
	//Mat img = imread("D:/projC/QtGuiApp3/HomeWork32/TestImage/warp_test.bmp");
	//if (!img.data)
	//{
	//	printf("the img's data is null!\n");
	//	return 0;
	//}

	//// ����һ����Ϊ "��Ϸԭ��"���� 
	//namedWindow("��Ϸԭ��");
	//// �ڴ�������ʾ��Ϸԭ�� 
	//imshow("��Ϸԭ��", img);
	//// �ȴ�6000 ms�󴰿��Զ��ر�
	//waitKey(6000);
}
