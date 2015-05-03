

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
	
	Mat frame, last_frame, test_frame, out_frame, m_frame, m_last_frame, d_last_frame;
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
		Mat dx, dy;
		Sobel(frame, dx, CV_8U, 1, 0);
		Sobel(frame, dy, CV_8U, 0, 1);
		convertScaleAbs(dx, dx);
		convertScaleAbs(dy, dy);
		//addWeighted(dx, 0.5, dy, 0.5, 0, frame);
		Mat g_hist;
		int hist_size[] = { 256 };
		int hist_chaneles[] = { 0 };
		float hist_range[] = { 0, 256 };
		const float * hist_range_t[]= {hist_range};
		calcHist(&frame, 1, hist_chaneles, Mat(), g_hist, 1, hist_size, hist_range_t);
		
		test_frame = Mat(g_hist.size().height, 100, g_hist.type());
		double max_histogram_value, min_histogram_value;
		minMaxIdx(g_hist, &min_histogram_value, &max_histogram_value);
		

		float previousHistogramValue = g_hist.at<float>(0);;
		bool histogram_was_growing = true;
		vector<int> breaking_points_vector;
		breaking_points_vector.push_back(0);

		/*for (size_t i = 1; i < 255; i++)
		{
			auto current_histogram_value = g_hist.at<float>(i);
			if (abs(current_histogram_value - previousHistogramValue) > max_histogram_value / 10)
			{
				bool isGrowing = current_histogram_value - previousHistogramValue > 0;
				if (!histogram_was_growing && isGrowing)
				{
					breaking_points_vector.push_back(i);
				}
				previousHistogramValue = current_histogram_value;
				histogram_was_growing = isGrowing;
			}
		}*/
		breaking_points_vector.push_back(50);
		breaking_points_vector.push_back(100);

		breaking_points_vector.push_back(150);
		breaking_points_vector.push_back(200);
		
		breaking_points_vector.push_back(255);

		double maximalPixelValue;
		double minimalPixelValue;
		minMaxIdx(frame, &minimalPixelValue, &maximalPixelValue);
		frame.copyTo(m_frame);
		for (size_t k = 0; k < m_frame.total(); k++)
		{
			for (auto i = breaking_points_vector.begin(); i < breaking_points_vector.end(); ++i)
			{
				if (m_frame.at<unsigned char>(k) > maximalPixelValue - 18)
				{
					m_frame.at<unsigned char>(k) = 255;
				}
					else 
				if (*i >= m_frame.at<unsigned char>(k))
				{
					if (i <= breaking_points_vector.begin() + 1)
					{
						m_frame.at<unsigned char>(k) = 0;
					}
					else if (i == breaking_points_vector.end() - 1)
					{
						m_frame.at<unsigned char>(k) = 255;
					}
					else
					{
						m_frame.at<unsigned char>(k) = (*i + (*i -1 ))/2;
					}
					break;
				}
			}
		}
		/*
		m_frame.forEach<unsigned char>([=](unsigned char &vale, const int * pos)
		{
			for (auto i = breaking_points_vector.begin(); i < breaking_points_vector.end(); ++i)
			{
				if (*i >= vale)
				{
					if (i <= breaking_points_vector.begin() + 1)
					{
						vale = 0;
					}
					else if (i == breaking_points_vector.end() - 1)
					{
						vale = 255;
					}
					else
					{
						vale = (*i + *(i - 1)) / 2;
					}
					break;
				}
			}
		});
		*/
		

		if (last_frame.empty())
		{
			//fisrt frame
			d_last_frame = frame;
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
				Mat out_frame_k, out_m_frame_k;
				warpAffine(m_frame, out_m_frame_k, stabiliaztion_mat, frame.size(), INTER_LINEAR | WARP_INVERSE_MAP);
				out_frame = m_last_frame - out_m_frame_k;
				warpAffine(frame, out_frame_k, stabiliaztion_mat, frame.size(), INTER_LINEAR | WARP_INVERSE_MAP);
				test_frame = d_last_frame - out_frame_k;
				d_last_frame = test_frame;
			}
			
		}
		auto toc = getTickCount();
		auto freq = getTickFrequency();
		auto exec_time = (toc - tic) / freq * 1000;
		last_frame = frame;
		m_last_frame = m_frame;

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
		waitKey(0/*MAX(10,1000/fps - exec_time)*/);
	}
	

	waitKey(0);

	return 0;
}
