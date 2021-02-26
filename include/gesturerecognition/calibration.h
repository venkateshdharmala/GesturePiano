//
// Created by Venkatesh on 11/15/2020.
//

#ifndef FINAL_PROJECT_CALIBRATION_H
#define FINAL_PROJECT_CALIBRATION_H

#include <iostream>
#include <opencv2/objdetect.hpp>
#include <opencv2/opencv.hpp>
#include <sstream>

namespace gesturerecognition {

/**
 * Class which handles initial setup to detect Hands of the user.
 */
class Calibration {
 public:
  Calibration(int min_filter_limit, int max_filter_limit,
              const std::string& hsv_window_name,
              const std::string& bgsub_window_name,
              const std::string& combined_window_name, double learning_rate,
              cv::Size hsv_window_size);

  /**
   * Processes the image by performing Median Blur, followed by Morphological
   * opening and closing
   * @param input_image : the image to be processed
   * @return the processed version of input_image
   */
  cv::Mat ProcessImage(const cv::Mat& input_image);

  /**
   * Filters input_image by the HSV ranges stored in the object instance.
   * @param input_image : the image to be filtered
   * @return : a binary image formed by thresholding
   */
  cv::Mat FilterImageByHSV(const cv::Mat& input_image);

  /**
   * Performs MOG2 background subtraction on image. Learning rate depends on
   * IsBackgroundTraining.
   * @param input_image: the image whose background is to be subtracted
   * @return : the foreground mask of the image
   */
  cv::Mat GetBackgroundSubtractedImage(const cv::Mat& input_image);

  /**
   * Creates trackbars in HSV window.
   * @param max_value
   */
  void CreateTrackbars(int max_value);

  /**
   * Returns whether background subtraction is learning or just applying
   * foreground mask
   * @return
   */
  bool IsBackgroundTraining();

  /**
   * Returns whether HSV calibration mode is on or not.
   * @return
   */
  bool IsHSVCalibrating();

  void SetHSVCalibration(bool boolean);
  void SetBackgroundTraining(bool boolean);

  /**
   * Gets the bitwise_and image of both the background subtracted and HSV
   * filtered image.
   * @param input_image
   * @return
   */
  cv::Mat GetFinalFilterImage(const cv::Mat& input_image);

 private:
  /**
   * Functions to deal with change in trackbar position
   */
  static void on_low_H_thresh_trackbar(int, void*);
  static void on_low_S_thresh_trackbar(int, void*);
  static void on_low_V_thresh_trackbar(int, void*);
  static void on_high_H_thresh_trackbar(int, void*);
  static void on_high_S_thresh_trackbar(int, void*);
  static void on_high_V_thresh_trackbar(int, void*);

  const std::string hsv_window_name_;  // Name of the HSV calibration window:
                                       // Used in CreateTrackbars
  bool train_background;
  bool calibrate_hsv;
  const std::string bg_subtraction_window_name_;
  const std::string final_filter_window_name_;
  double background_subtraction_learning_rate;
  cv::Ptr<cv::BackgroundSubtractor> background_subtractor_;
  /*
   * The ranges for hue,saturation and values of filter
   */
  int low_hue_;
  int low_saturation_;
  int low_value_;
  int high_hue_;
  int high_saturation_;
  int high_value_;
  cv::Size hsv_window_size_;
};
}  // namespace gesturerecognition

#endif  // FINAL_PROJECT_CALIBRATION_H
