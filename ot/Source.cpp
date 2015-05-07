

#include "Utils.h"

#include <stdio.h>
#include <iostream>

using namespace cv;
using namespace std;


#define SHOW(img) cv::imshow(#img,img)


int main(int /*argc*/, const char** /*argv*/)
{
	string filename = "C:\\Users\\Aghasy\\Documents\\GitHub\\objectTracking\\videos\\ta2_1.avi";
	
	VideoCapture video_capture(filename);
	if (!video_capture.isOpened())
	{
		printf("Cannot read video file: %s\n", filename.c_str());
		return -1;
	}
	auto sourceWinName = "Source";
	// Create a window
	namedWindow(sourceWinName, 1);

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

		
		if (last_frame.empty())
		{
			//fisrt frame
			
		} 
		else
		{
			Mat stabiliaztion_mat = estimateRigidTransform(last_frame, frame, 0);
			if (stabiliaztion_mat.empty())
			{
				printf("Error: cannot estimateRigidTransform\n");
			} 
			else
			{
				Mat out_frame_k;
				warpAffine(frame, out_frame_k, stabiliaztion_mat, frame.size(), INTER_LINEAR | WARP_INVERSE_MAP);

			

				Mat frame_avg, last_frame_avg;

				auto med_f = mean(frame_avg);
				auto med_l_f = mean(out_frame_k);

				frame_avg = frame_avg - med_f;
				out_frame_k = out_frame_k - med_l_f;



				float kerData[] = {
					1,	0.01,	-1,
					0.5,	0.005,	0.01,
					1,	0.01,	-1
				};

				
				const int k_size =3;

				Mat ker1(k_size,k_size,CV_32F);
				 

				memcpy(ker1.data, kerData, sizeof(kerData));

				filter2D(out_frame_k, frame_avg, -1, ker1);
				filter2D(last_frame, last_frame_avg, -1, ker1);
				



				SHOW(frame_avg);
				SHOW(last_frame_avg);

				Mat abs_diff;
				absdiff(frame_avg, last_frame_avg, abs_diff);
				SHOW(abs_diff);


				

				
			}
			
		}
		auto toc = getTickCount();
		auto freq = getTickFrequency();
		auto exec_time = (toc - tic) / freq * 1000;
		last_frame = frame;

		printf("Processing: %d time: %f ms +time:%f \n", frame_index, exec_time, 1000 / fps - exec_time);
		waitKey(1000/*MAX(10,1000/fps - exec_time)*/);
	}
	

	waitKey(0);

	return 0;
}
