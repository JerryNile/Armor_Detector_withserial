#include"Armor_detection.h"

using namespace Robomaster;

int main(int argc, char** argv) 
{


	double t, tc;

	VideoCapture cap(argv[1]);

	//cap.set(CAP_PROP_FRAME_WIDTH, 1000);
	//cap.set(CAP_PROP_FRAME_HEIGHT, 1000);
	//cap.set(CAP_PROP_BRIGHTNESS, 500);
	//cap.set(CV_CAP_PROP_FPS, 3);

	//cout << "Width" << cap.get(CAP_PROP_FRAME_WIDTH) << endl;
	//cout << "height" << cap.get(CAP_PROP_FRAME_HEIGHT) << endl;
	//cout << "CV_CAP_PROP_FPS" << cap.get(CV_CAP_PROP_FPS) << endl;

	do
	{

		t = getTickCount();


		Mat src;
		cap.read(src);
		ArmorDetector detector;
		int flag;

		detector.setEnemyColor(BLUE);
		detector.loadImg(src);


		flag = detector.detect();

		//detector.drawArmor_Points(src);

		detector.draw_All_Armor(src);
		//ArmorDescriptor target;
		//cout<<target.center.x<<endl;
		//detector.drawArmor_Points(src);
		//Point2f Target(target.center.x, target.center.y);
		//cout << Target << endl;
		cout << detector._targetArmor.center.x << endl;
		tc = (getTickCount() - t) / getTickFrequency();
		int fps = 1 / tc;
		//printf("FPS: %.5f\n", fps);    //��ʾ���ķ�ʱ��Ķ���
		string s_fps = "FPS: " + to_string(fps);
		putText(src, s_fps, Point(10, 30),FONT_HERSHEY_COMPLEX,1,Scalar(0,0,255),1);

		imshow("src", src);
		waitKey(1);

	} while (true);


}