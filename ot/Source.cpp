

#include "Utils.h"

#include <stdio.h>
#include <iostream>

using namespace cv;
using namespace std;


#define SHOW(img) cv::imshow(#img,img)



Mat findRANSACHomography(Mat last_frame, Mat frame)
{
	auto detector = GFTTDetector::create();
	vector<KeyPoint> last_frame_descriptors, current_frame_descriptor;
	Mat last_desctiptors_object, current_descriptors_object;

	detector->detect(last_frame, last_frame_descriptors);
	detector->compute(last_frame, last_frame_descriptors, last_desctiptors_object);
	
	detector = GFTTDetector::create();
	detector->detect(frame, current_frame_descriptor);
	detector->compute(frame, current_frame_descriptor, current_descriptors_object);

	FlannBasedMatcher matcher;
	std::vector< DMatch > matches;
	matcher.match(last_desctiptors_object, current_descriptors_object, matches);

	double max_dist = 0; double min_dist = 100;

	//-- Quick calculation of max and min distances between keypoints
	for (int i = 0; i < current_descriptors_object.rows; i++)
	{
		double dist = matches[i].distance;
		if (dist < min_dist) min_dist = dist;
		if (dist > max_dist) max_dist = dist;
	}

	std::vector< DMatch > good_matches;

	for (int i = 0; i < current_descriptors_object.rows; i++)
	{
		if (matches[i].distance < 3 * min_dist)
		{
			good_matches.push_back(matches[i]);
		}
	}

	//-- Localize the object
	std::vector<Point2f> obj;
	std::vector<Point2f> scene;

	for (int i = 0; i < good_matches.size(); i++)
	{
		//-- Get the keypoints from the good matches
		obj.push_back(current_frame_descriptor[good_matches[i].queryIdx].pt);
		scene.push_back(last_frame_descriptors[good_matches[i].trainIdx].pt);
	}
	return findHomography(obj, scene, CV_RANSAC);

}

int main(int /*argc*/, const char** /*argv*/)
{
	string filename = "C:\\Users\\Aghasy\\Documents\\GitHub\\objectTracking\\videos\\ta2_1.avi";
	
	VideoCapture video_capture(filename);
	if (!video_capture.isOpened())
	{
		printf("Cannot read video file: %s\n", filename.c_str());
		return -1;
	}


	Mat frame, last_frame;
	auto fps = video_capture.get(CAP_PROP_FPS);

	for (int frame_index = 0; ; frame_index++)
	{
		auto tic = getTickCount();
		video_capture >> frame;
		
		if (frame.empty())
		{
			printf("Ended\n");
			break;
		} 
		if (frame_index %3)
		{
			continue;
		}
		cvtColor(frame, frame, CV_BGR2GRAY);

		SHOW(frame);
		if (last_frame.empty())
		{
			//fisrt frame
			
		} 
		else
		{
			Mat stabiliaztion_mat = findRANSACHomography(last_frame, frame);
			if (stabiliaztion_mat.empty())
			{
				printf("Error: cannot estimateRigidTransform\n");
			} 
			else
			{
				Mat frame_w,frame_k;
				warpAffine(frame, frame_w, stabiliaztion_mat, frame.size(), INTER_LINEAR | WARP_INVERSE_MAP);

				 

				Mat frame_avg, last_frame_avg;

				auto med_l_f = mean(last_frame);
				auto med_f = mean(frame_w);

				Mat last_frame_k = last_frame - med_l_f;
				frame_k = frame_w - med_f;


				
				//SHOW(abs_diff_k);

				//SHOW(last_frame_k);
				//SHOW(frame_k);

				float kerData[] = {
					1,	0.01,	-1,
					0.5,	0.005,	0.01,
					1,	0.01,	-1
				};

				
				const int k_size =3;

				Mat ker1(k_size,k_size,CV_32F);
				 

				memcpy(ker1.data, kerData, sizeof(kerData));

				filter2D(frame_k, frame_avg, -1, ker1);
				filter2D(last_frame_k, last_frame_avg, -1, ker1);
				
				


				//SHOW(frame_avg);
				//SHOW(last_frame_avg);

				Mat abs_diff;
				absdiff(frame_avg, last_frame_avg, abs_diff);
				

				Mat frame_m(abs_diff.clone());

				
				bool any = false;

				
				rectangle(abs_diff, Rect(0, 0, 10, 10), Scalar(255));
				SHOW(abs_diff);
				

				

				
			}
			
		}
		auto toc = getTickCount();
		auto freq = getTickFrequency();
		auto exec_time = (toc - tic) / freq * 1000;
		last_frame = frame;

		printf("Processing: %d time: %f ms +time:%f \n", frame_index, exec_time, 1000 / fps - exec_time);
		waitKey(100/*MAX(10,1000/fps - exec_time)*/);
	}
	

	waitKey(0);

	return 0;
}
