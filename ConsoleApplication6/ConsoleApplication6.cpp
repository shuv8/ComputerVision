#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include <iostream>
#include <fstream>
#include <chrono> 

using namespace std;
using namespace cv;

int g_slider_position = 0;
int g_run = 1, g_dontset = 0; //start out in single step mode
//! [declare]
/// Global Variables
bool use_mask;
Mat img; Mat templ; Mat mask; Mat result;
const char* image_window = "Source Image";
const char* result_window = "Result window";
double seconds,cntr(0);
double total(0);
int match_method;
int max_Trackbar = 5;
//! [declare]

/// Function Headers
void MatchingMethod(int, void*);
cv::VideoCapture g_cap;
void onTrackbarSlide(int pos, void *) {
	g_cap.set(cv::CAP_PROP_POS_FRAMES, pos);
		if (!g_dontset)
			g_run = 1;
	g_dontset = 0;
}
int main(int argc, char** argv) {
	
	namedWindow("Example", cv::WINDOW_AUTOSIZE);
	g_cap.open(string(argv[1]));
	int frames = (int)g_cap.get(cv::CAP_PROP_FRAME_COUNT);
	int tmpw = (int)g_cap.get(cv::CAP_PROP_FRAME_WIDTH);
	int tmph = (int)g_cap.get(cv::CAP_PROP_FRAME_HEIGHT);
	cout << "Video has " << frames << " frames of dimensions("
		<< tmpw << ", " << tmph << ")." << endl;
	createTrackbar("Position", "Example", &g_slider_position, frames,
		onTrackbarSlide);
	Mat frame;
	for (;;) {
		if (g_run != 0) {
			g_cap >> frame; if (frame.empty()) break;
			int current_pos = (int)g_cap.get(cv::CAP_PROP_POS_FRAMES);
			g_dontset = 1;
			setTrackbarPos("Position", "Example", current_pos);

			///////TEMPLATE MACHING TRACKING////////////////////

			if (argc < 3){
				cout << "Not enough parameters" << endl;
				cout << "Usage:\n./MatchTemplate_Demo <image_name> <template_name> [<mask_name>]" << endl;
				return -1;
			}

			img = frame;
			templ = imread(argv[2], IMREAD_COLOR);

			if (argc > 3) {
				use_mask = true;
				mask = imread(argv[3], IMREAD_COLOR);
			}

			if (img.empty() || templ.empty() || (use_mask && mask.empty()))
			{
				cout << "Can't read one of the images" << endl;
				return -1;
			}
			const char* trackbar_label = "Method: \n 0: SQDIFF \n 1: SQDIFF NORMED \n 2: TM CCORR \n 3: TM CCORR NORMED \n 4: TM COEFF \n 5: TM COEFF NORMED";
			createTrackbar(trackbar_label, "Example", &match_method, max_Trackbar, MatchingMethod);
			MatchingMethod(0, 0);

			////////////////////////////////////////////////////////////////////////

			cv::imshow("Example", frame);

			g_run -= 1;
			//g_run = -1;
		}

		char c = (char)cv::waitKey(10);
		if (c == 's') // single step
		{
			g_run = 1; cout << "Single step, run = " << g_run << endl;
			std::cout << "TOTAL TIME: " << total/cntr << " ms\n in " << cntr << " frames." << endl;
		}
		if (c == 'r') // run mode
		{
			g_run = -1; cout << "Run mode, run = " << g_run << endl; 
			cntr= 0;
			total = 0;
		}
		if (c == 27)
			break;
	}
	return(0);
}

void MatchingMethod(int, void*)
{
	auto begin = std::chrono::steady_clock::now();
	//! [copy_source]
	/// Source image to display
	Mat img_display;
	img.copyTo(img_display);
	//! [copy_source]

	//! [create_result_matrix]
	/// Create the result matrix
	int result_cols = img.cols - templ.cols + 1;
	int result_rows = img.rows - templ.rows + 1;

	result.create(result_rows, result_cols, CV_32FC1);
	//! [create_result_matrix]

	//! [match_template]
	/// Do the Matching and Normalize
	bool method_accepts_mask = (TM_SQDIFF == match_method || match_method == TM_CCORR_NORMED);
	if (use_mask && method_accepts_mask)
	{
		matchTemplate(img, templ, result, match_method, mask);
	}
	else
	{
		matchTemplate(img, templ, result, match_method);
	}
	//! [match_template]

	//! [normalize]
	normalize(result, result, 0, 1, NORM_MINMAX, -1, Mat());
	//! [normalize]

	//! [best_match]
	/// Localizing the best match with minMaxLoc
	double minVal; double maxVal; Point minLoc; Point maxLoc;
	Point matchLoc;

	minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc, Mat());
	//! [best_match]

	//! [match_loc]
	/// For SQDIFF and SQDIFF_NORMED, the best matches are lower values. For all the other methods, the higher the better
	if (match_method == TM_SQDIFF || match_method == TM_SQDIFF_NORMED)
	{
		matchLoc = minLoc;
	}
	else
	{
		matchLoc = maxLoc;
	}
	//! [match_loc]

	//! [imshow]
	/// Show me what you got
	rectangle(img_display, matchLoc, Point(matchLoc.x + templ.cols, matchLoc.y + templ.rows), Scalar::all(0), 2, 8, 0);
	if (matchLoc.x < 5)
	{
		cout << "0";
	}
	else
		cout << "1";
	rectangle(result, matchLoc, Point(matchLoc.x + templ.cols, matchLoc.y + templ.rows), Scalar::all(0), 2, 8, 0);

	imshow(image_window, img_display);
	imshow(result_window, result);


	//TIME
	auto end = std::chrono::steady_clock::now();

	auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin);
	std::cout << "Time: " << elapsed_ms.count() << " ms\n" << endl;
	cntr+= 1;
	total += elapsed_ms.count();
	//! [imshow]

	return;
}