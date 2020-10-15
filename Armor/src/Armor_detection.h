#pragma once
#include"opencv_extended.h"

using namespace cv;
using namespace std;

Mat kernel = getStructuringElement(MORPH_ELLIPSE, Size(3, 3));

namespace Robomaster
{


	/*
	* 颜色 B G R
	*/
	enum Colors
	{
		BLUE = 0,
		GREEN = 1,
		RED = 2
	};


	/*
	*	该结构体储存所有装甲板识别所需的参数
	*/
	struct ArmorParam
	{

		//Pre-treatment 预处理
		int color_Threshold = 30;  //recommend 40~80

		//Filter lights 灯条过滤
		float light_min_area = 10;
		float light_max_angle = 45;

		//Filter pairs 灯条匹配过滤
		float light_max_angle_diff = 3;
		float light_max_height_diff_ratio = 0.4;
		float light_max_y_diff_ratio = 0.3;
		float light_max_x_diff_ratio = 3;

		//other params 其他参数
		float sight_offset_normalized_base = 200;
		float area_normalized_base = 500;
		int enemy_color = RED;
		Size srcImageSize = Size(2048, 2048);
	} _param ;





	/*
	*   灯条类，灯条描述子，包含了描述灯条的所有对象及其属性 长、宽、角度、面积、中心点
	*/
	class LightDescriptor
	{
	public: 

		//给定义的Light Descriptor设置默认值

		LightDescriptor(){}

		LightDescriptor(const RotatedRect& light)
		{
			width = light.size.width;
			length = light.size.height;
			center = light.center;

			if (light.angle > 135.0)
				angle = light.angle - 180.0;
			else 
				angle = light.angle;
		}

		/*
		*	@Brief: return the light as a cv::RotatedRect object
		*/
		RotatedRect rec() const
		{
			return RotatedRect(center, Size2f(width, length), angle);
		}


	public:
		float width;
		float length;
		Point2f center;
		float angle;
	};



	/*
	* 	    装甲板类,装甲板描述子，包含了描述装甲板的所有对象及其属性 左右灯条、装甲板四顶点、装甲板中心点   This class describes the armor information, including maximum bounding box, vertex and so on
	*/
	class ArmorDescriptor
	{
	public:

		/*
		*	@Brief: Initialize with all 0
		*/
		ArmorDescriptor(){}

		
		/*
		*	@Brief: calculate the rest of information(except for match&final score)of ArmroDescriptor based on:
				l&r light, part of members in ArmorDetector, and the armortype(for the sake of saving time)
		*	@Calls: ArmorDescriptor::getFrontImg()
		*/
		ArmorDescriptor(const LightDescriptor& lLight,
						const LightDescriptor& rLight)
		{
			//handle two lights
			lightPairs[0] = lLight.rec();
			lightPairs[1] = rLight.rec();

			cv::Size exLSize(int(lightPairs[0].size.width), int(lightPairs[0].size.height * 2));
			cv::Size exRSize(int(lightPairs[1].size.width), int(lightPairs[1].size.height * 2));
			cv::RotatedRect exLLight(lightPairs[0].center, exLSize, lightPairs[0].angle);
			cv::RotatedRect exRLight(lightPairs[1].center, exRSize, lightPairs[1].angle);

			cv::Point2f pts_l[4];
			exLLight.points(pts_l);
			cv::Point2f upper_l = pts_l[2];
			cv::Point2f lower_l = pts_l[3];

			cv::Point2f pts_r[4];
			exRLight.points(pts_r);
			cv::Point2f upper_r = pts_r[1];
			cv::Point2f lower_r = pts_r[0];

			vertex.resize(4);
			vertex[0] = upper_l;
			vertex[1] = upper_r;
			vertex[2] = lower_r;
			vertex[3] = lower_l;


			//set armor center
			center.x = (upper_l.x + lower_r.x) / 2;
			center.y = (upper_l.y + lower_r.y) / 2;


			// calculate the size score
			//float normalized_area = contourArea(vertex) / _param.area_normalized_base;
			sizeScore = exp(contourArea(vertex));


			// calculate the distance score

			Point2f srcImgCenter(1024, 1024);

			float sightOffset = cvex::distance(srcImgCenter,center);

			distScore = exp(sightOffset / _param.sight_offset_normalized_base);

			finalScore = sizeScore + distScore;

		}


	public:
		std::array<cv::RotatedRect, 2> lightPairs;	   //0 left, 1 right
		std::array<int, 2> lightsFlags;				  //0 left's flag, 1 right's flag
		std::vector<cv::Point2f> vertex;			 //four vertex of armor area, light bar area exclued!!	
		cv::Point2f center;						    //center point of armor

		float sizeScore;		//S1 = e^(size)
		float distScore;		//S2 = e^(-offset)
		float finalScore;		//sum of all the scores


	};





	/*
	*	    装甲板识别所用的各类函数与对象  This class implement all the functions of detecting the armor
	*/
	class ArmorDetector
	{
	public:
		/*
		*	flag for the detection result
		*/
		enum DetectorFlag
		{
			ARMOR_NO = 0, 		// not found
			ARMOR_FOUND = 1         // found target_Armor
		};



		ArmorDetector() {};




		/*
		*	@Brief: set the enemy's color
		*	@Others: API for client
		*/
		void setEnemyColor(int enemy_color)
		{
			_enemy_color = enemy_color;
		}



		/*
		*	@Brief: load image
		*	@Input: frame
		*/
		void loadImg(const cv::Mat&  srcImg)
		{
			_srcImg = srcImg;
		}

		


		/*
		*	@Brief: core of detection algrithm, include all the main detection process
		*	@Outputs: ALL the info of detection result
		*	@Return: See enum DetectorFlag
		*	@Others: API for client
		*/
		int detect()
		{

			/*
			*	Detect lights and build light bars' desciptors
			*/
			_armors.clear();
			std::vector<LightDescriptor> lightInfos;


			/*
			*	pre-treatment
			*/

			// 通道相减分割颜色
	

			std::vector<cv::Mat> channels;
			cv::Mat image;
			split(_srcImg, channels);
			
			int c1, c2;

			if (_enemy_color == BLUE)
			{
				c1 = 0;
				c2 = 2;
			}
			if (_enemy_color == RED)
			{
				c1 = 2;
				c2 = 0;
			}


			subtract(channels[c1], channels[c2], image);

			cv::threshold(image, image, _param.color_Threshold, 255, cv::THRESH_BINARY);
			/*
			*	find and filter light bars
			*/

			dilate(image, image, kernel);
			
			vector<vector<Point>> lightContours;

			cv::findContours(image, lightContours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

			
			for (const auto& contour : lightContours)
			{
				float lightContourArea = contourArea(contour);

				if (contour.size() <= 5 ||
					lightContourArea < _param.light_min_area)
					continue;

				RotatedRect lightRec = fitEllipse(contour);


				//Adjust the RotatedRect lightRec angle
				if (lightRec.angle > 135)
					lightRec.angle -= 180.0;

				//filter light according to the angle
				if (abs(lightRec.angle) > 45)
					continue;

				lightInfos.emplace_back(lightRec);
			}



			if (lightInfos.empty())
				return _flag = ARMOR_NO;

			/*
			*	find and filter light bar pairs
			*/

			//升序排列灯条 将灯条从左到右排列
			sort(lightInfos.begin(), lightInfos.end(), [](const LightDescriptor& ld1, const LightDescriptor& ld2)
			{
				return ld1.center.x < ld2.center.x;
			});



			// 将检测到的装甲板从左到右一个一个进行匹配并筛选出初装甲板s

			for (size_t i = 0; i < lightInfos.size(); i++)
			{
				for (size_t j = i + 1; j < lightInfos.size(); j++)
				{
					const LightDescriptor& leftLight = lightInfos[i];
					const LightDescriptor& rightLight = lightInfos[j];



					/*
					*	Works for 3-7 meters situation
					*	morphologically similar: // parallel
									 // similar height
					*/


					float angleDiff_ = abs(leftLight.angle - rightLight.angle);
					float LenDiff_ratio = abs(leftLight.length - rightLight.length) / max(leftLight.length, rightLight.length);

					if (angleDiff_ > _param.light_max_angle_diff ||
						LenDiff_ratio > _param.light_max_height_diff_ratio)
					{
						continue;
					}


					/*
					*	proper location:  // y value of light bar close enough
					*					 // ratio of length and width is proper
					*/
					float meanLen = (leftLight.length + rightLight.length) / 2;

					float yDiff = abs(leftLight.center.y - rightLight.center.y);
					float yDiff_ratio = yDiff / meanLen;

					float xDiff = abs(leftLight.center.x - rightLight.center.x);
					float xDiff_ratio = xDiff / meanLen;

					if (yDiff_ratio > _param.light_max_y_diff_ratio ||
						xDiff_ratio > _param.light_max_x_diff_ratio)
						continue;

	

					ArmorDescriptor armor(leftLight, rightLight);
					armor.lightsFlags[0] = i;
					armor.lightsFlags[1] = j;
					_armors.emplace_back(armor);
					break;
				}
			}

			if (_armors.empty())
			{
				return _flag = ARMOR_NO;
			}


			// 过滤出假装甲板（两个中x投影更小）



			for (int i = 0; i < _armors.size(); i++)
			{
				bool true_or_false = 1;
				if (_armors.size() > 2)
				{
					for (int j = i + 1; j < _armors.size(); j++)
					{
						if (_armors[i].lightsFlags[0] == _armors[j].lightsFlags[0] ||
							_armors[i].lightsFlags[0] == _armors[j].lightsFlags[1] ||
							_armors[i].lightsFlags[1] == _armors[j].lightsFlags[0] ||
							_armors[i].lightsFlags[1] == _armors[j].lightsFlags[1])
						{
							true_or_false = 0;
							abs(_armors[i].vertex[0].x - _armors[i].vertex[1].x) > abs(_armors[j].vertex[0].x - _armors[j].vertex[1].x) ? _True_armors.emplace_back(_armors[i]) : _True_armors.emplace_back(_armors[j]);
							i++;
							break;
						}
					}
				}
				if (true_or_false)
					_True_armors.emplace_back(_armors[i]);

			}





			sort(_True_armors.begin(), _True_armors.end(), [](const ArmorDescriptor& ld1, const ArmorDescriptor& ld2)
			{
				return ld1.finalScore > ld2.finalScore;
			});

			_targetArmor = _True_armors[0];
			
			return _flag = ARMOR_FOUND;
		};


		


		void drawArmor_Points(Mat src)
		{
			if(_flag)
			{
				for (int i = 0; i < 4; i++) {
					line(src, _targetArmor.vertex[i], _targetArmor.vertex[(i + 1) % 4], Scalar(0, 255, 0), 1, LINE_AA);
				}
			}

		}


		void draw_All_Armor(Mat src)
		{
			if (_flag)
			{
				for (size_t i = 0; i < _True_armors.size(); i++)
				{
					for (int j = 0; j < 4; j++) {
						line(src, _True_armors[i].vertex[j], _True_armors[i].vertex[(j + 1) % 4], Scalar(0, 255, 0), 3, LINE_AA);
					}
				}

			}

		}
		

	public:

		cv::Mat _srcImg; //source img

		int _enemy_color;

		std::vector<ArmorDescriptor> _armors;
		std::vector<ArmorDescriptor> _True_armors;

		
		ArmorDescriptor _targetArmor; //relative coordinates
		int _flag;

	};


}