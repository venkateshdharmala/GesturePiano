//
// Created by Venkatesh on 11/15/2020.
//
#include "gesturerecognition/calibration.h"

namespace gesturerecognition {
Calibration::Calibration(int min_filter_limit, int max_filter_limit,
                         const std::string &hsv_window_name,
                         const std::string &bgsub_window_name,
                         const std::string &combined_window_name,
                         double learning_rate, cv::Size hsv_window_size)
    : low_hue_(min_filter_limit),
      low_saturation_(min_filter_limit),
      low_value_(min_filter_limit),
      high_hue_(max_filter_limit),
      high_saturation_(max_filter_limit),
      high_value_(max_filter_limit),
      bg_subtraction_window_name_(bgsub_window_name),
      hsv_window_name_(hsv_window_name),
      final_filter_window_name_(combined_window_name),
      background_subtraction_learning_rate(learning_rate),
      train_background(false),
      calibrate_hsv(false),
      hsv_window_size_(std::move(hsv_window_size)) {
  background_subtractor_ = cv::createBackgroundSubtractorMOG2();
}
cv::Mat Calibration::GetBackgroundSubtractedImage(const cv::Mat &input_image) {
  cv::Mat foreground_mask;
  if (train_background) {
    background_subtractor_->apply(input_image, foreground_mask,
                                  background_subtraction_learning_rate);

  } else {
    background_subtractor_->apply(input_image, foreground_mask,
                                  background_subtraction_learning_rate / 1000);
    // This will subtract background at a much lower rate.So hands staying still
    // will not be erased from foreground, but other stationary objects will.
  }
  return foreground_mask;
}

cv::Mat Calibration::GetFinalFilterImage(const cv::Mat &input_image) {
  cv::Mat final_output;
  auto background_subtracted_image = GetBackgroundSubtractedImage(input_image);
  auto hsv_threshold = FilterImageByHSV(input_image);
  cv::bitwise_and(ProcessImage(hsv_threshold),
                  ProcessImage(background_subtracted_image), final_output);
  // Bitwise_and gets an image which contains common pixels between
  // background_subtracted and hsv threshold images.
  return final_output;
}
bool Calibration::IsHSVCalibrating() {
  return calibrate_hsv;
}

void Calibration::SetHSVCalibration(bool boolean) {
  calibrate_hsv = boolean;
}

bool Calibration::IsBackgroundTraining() {
  return train_background;
}

void Calibration::SetBackgroundTraining(bool boolean) {
  train_background = boolean;
}

cv::Mat Calibration::FilterImageByHSV(const cv::Mat &input_image) {
  cv::Mat frame_hsv, frame_threshold;
  cv::cvtColor(input_image, frame_hsv,
               cv::COLOR_BGR2HSV); /* Converting the input image to HSV
                                    colorspace and saving it in frame_HSV*/
  cv::inRange(frame_hsv, cv::Scalar(low_hue_, low_saturation_, low_value_),
              cv::Scalar(high_hue_, high_saturation_, high_value_),
              frame_threshold);
  /* Filtering frame_HSV with the low and high HSV member variables, and saving
   the filtered image in frame_threshold.*/

  return frame_threshold;
}

void Calibration::CreateTrackbars(int max_value) {
  cv::namedWindow(hsv_window_name_);
  cv::resizeWindow(hsv_window_name_, hsv_window_size_.width,
                   hsv_window_size_.height);
  cv::createTrackbar("Low H", hsv_window_name_, &low_hue_, max_value,
                     on_low_H_thresh_trackbar, this);
  cv::createTrackbar("High H", hsv_window_name_, &high_hue_, max_value,
                     on_high_H_thresh_trackbar, this);
  cv::createTrackbar("Low S", hsv_window_name_, &low_saturation_, max_value,
                     on_low_S_thresh_trackbar, this);
  cv::createTrackbar("High S", hsv_window_name_, &high_saturation_, max_value,
                     on_high_S_thresh_trackbar, this);
  cv::createTrackbar("Low V", hsv_window_name_, &low_value_, max_value,
                     on_low_V_thresh_trackbar, this);
  cv::createTrackbar("High V", hsv_window_name_, &high_value_, max_value,
                     on_high_V_thresh_trackbar, this);
}

cv::Mat Calibration::ProcessImage(const cv::Mat &input_image) {
  cv::Mat processed_image;
  cv::medianBlur(input_image, processed_image, 5);
  morphologyEx(processed_image, processed_image, cv::MORPH_OPEN, cv::Mat(),
               cv::Point(-1, -1), 3);
  morphologyEx(processed_image, processed_image, cv::MORPH_CLOSE, cv::Mat(),
               cv::Point(-1, -1), 3);
  return processed_image;
}

void Calibration::on_low_H_thresh_trackbar(int, void *ptr) {
  Calibration *calibration_object = (Calibration *)ptr;
  cv::setTrackbarPos("Low H", calibration_object->hsv_window_name_,
                     calibration_object->low_hue_);
}
void Calibration::on_high_H_thresh_trackbar(int, void *ptr) {
  Calibration *calibration_object = (Calibration *)ptr;
  cv::setTrackbarPos("High H", calibration_object->hsv_window_name_,
                     calibration_object->high_hue_);
}
void Calibration::on_low_S_thresh_trackbar(int, void *ptr) {
  Calibration *calibration_object = (Calibration *)ptr;
  cv::setTrackbarPos("Low S", calibration_object->hsv_window_name_,
                     calibration_object->low_saturation_);
}
void Calibration::on_high_S_thresh_trackbar(int, void *ptr) {
  Calibration *calibration_object = (Calibration *)ptr;
  cv::setTrackbarPos("High S", calibration_object->hsv_window_name_,
                     calibration_object->high_saturation_);
}
void Calibration::on_low_V_thresh_trackbar(int, void *ptr) {
  Calibration *calibration_object = (Calibration *)ptr;
  cv::setTrackbarPos("Low V", calibration_object->hsv_window_name_,
                     calibration_object->low_value_);
}
void Calibration::on_high_V_thresh_trackbar(int, void *ptr) {
  Calibration *calibration_object = (Calibration *)ptr;
  cv::setTrackbarPos("High V", calibration_object->hsv_window_name_,
                     calibration_object->high_value_);
}
}  // namespace gesturerecognition
