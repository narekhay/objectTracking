

#include "Utils.h"

#include <stdio.h>

using namespace cv;
using namespace std;



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
	auto testWinName = "Test";
	auto outWinName = "Out";
	// Create a window
	namedWindow(sourceWinName, 1);
	//namedWindow(outWinName, 1);
	namedWindow(testWinName, 1);
	
	Mat frame, last_frame, test_frame, out_frame;
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

				Mat ker1, ker2;
				int k_size = 3;
				auto sigma = 0.3*((k_size - 1)*0.5 - 1) + 0.8;
				int kernel_size = 31;
				double sig = 1, th = 0, lm = 1.0, gm = 0.02, ps = 0;
				ker1 = getGaussianKernel(k_size, sigma, CV_32F);
				auto ker5 = getGaborKernel(Size(kernel_size, kernel_size), gm, th, lm, ps);
				imshow("Gabor", ker5);

				ker2 = ker1;

				filter2D(out_frame_k, frame_avg, -1, ker1);
				filter2D(last_frame, last_frame_avg, -1, ker1);
				out_frame = frame_avg;
				auto s1 = frame_avg.data;
				auto s2 = last_frame_avg.data;
				test_frame = last_frame.clone();
				auto oData = test_frame.data;
				
				auto w = out_frame.size().width;
				auto h = out_frame.size().height;
				for (size_t i = 0; i < h; i++)
				{
					for (size_t j = 0; j < w; j++)
					{
						auto p1 = *((s1 + j) + w*i);
						auto p2 = *((s2 + j) + w*i);
						*((oData + j) + w*i) = abs(p1 - p2);
					}
				}


				//test_frame = out_frame_k - last_frame;
				//out_frame = last_frame - out_frame_k;


			}
			
		}
		auto toc = getTickCount();
		auto freq = getTickFrequency();
		auto exec_time = (toc - tic) / freq * 1000;
		last_frame = frame;

		printf("Processing: %d time: %f ms +time:%f \n", frame_index, exec_time, 1000 / fps - exec_time);

		if (!test_frame.empty())
		{
			imshow(testWinName, test_frame);
		}
		imshow(sourceWinName, frame);
		if (!out_frame.empty())
		{
			imshow(outWinName, out_frame);
		}
		waitKey(1000/*MAX(10,1000/fps - exec_time)*/);
	}
	

	waitKey(0);

	return 0;
}
