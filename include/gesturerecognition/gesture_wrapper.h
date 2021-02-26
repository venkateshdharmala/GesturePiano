//
// Created by Venkatesh on 12/2/2020.
//

#ifndef FINAL_PROJECT_GESTURE_WRAPPER_H
#define FINAL_PROJECT_GESTURE_WRAPPER_H

#include <cinder/Color.h>
#include <cinder/gl/gl.h>
#include <pianoapp/piano_engine.h>

#include <iostream>
#include <fstream>

#include "gesturerecognition/calibration.h"
#include "gesturerecognition/hand_extractor.h"
#include "gesturerecognition/hand_tracker.h"
#include "nlohmann/json.hpp"

/**
 * This class ties everything in the gesturerecognition namespace together
 */
namespace gesturerecognition {

struct ProgramSettings {
  ProgramSettings(const std::string& json_file_name) {
    std::ifstream istream;
    //Replace *ENTER_PROJECT_PATH_HERE* with the location of this project directory
    istream.open("*ENTER_PROJECT_PATH_HERE*\\config.json");
    if (istream.is_open()) {
      nlohmann::json j = nlohmann::json::parse(istream);
      //We feed in all the necessary data from the config file.
      camera_number = j["camera_number"];
      maximum_hsv_limit = j["maximum_hsv_limit"];
      output_window_size =
          cv::Size(j["output_window_length"], j["output_window_height"]);
      hsv_window_size =
          cv::Size(j["hsv_window_length"], j["hsv_window_height"]);
      background_learning_rate = j["background_learning_rate"];
      frames_to_track = j["frames_to_track"];
      combined_window_name = j["combined_window_name"];
      background_sub_window_name = j["background_sub_window_name"];
      hsv_window_name = j["hsv_window_name"];
      convex_hull_window_name = j["convex_hull_window_name"];
      number_of_white_keys = j["number_of_white_keys"];
      row_margin = j["row_margin"];
      number_of_rows = j["number_of_rows"];
      piano_notes_file_name = j["piano_notes_file_name"];
      finger_tip_circle_radius = j["finger_tip_circle_radius"];
      piano_circle_radius = j["piano_circle_radius"];
      finger_tip_circle_thickness = j["finger_tip_circle_thickness"];
      line_type = j["line_type"];
    }
  }
  int camera_number;
  int number_of_rows;
  int row_margin;
  cv::Size output_window_size;
  size_t maximum_hsv_limit;
  int number_of_white_keys;
  std::string hsv_window_name;
  std::string background_sub_window_name;
  std::string combined_window_name;
  double background_learning_rate;
  cv::Size hsv_window_size;
  size_t frames_to_track;
  std::string convex_hull_window_name;
  std::string piano_notes_file_name;
  int finger_tip_circle_radius;
  int piano_circle_radius;
  int finger_tip_circle_thickness;
  int line_type;
};

class GestureWrapper {
 public:
  /**
   * Default constructor.
   * @param settings: the configuration settings in the form of a
   * ProgramSettings object.
   */
  GestureWrapper(const ProgramSettings& settings);

  /**
   * Toggles the background subtraction mode.
   */
  void ToggleBackgroundCalibration();

  /**
   * Toggles the HSV filtering mode.
   */
  void ToggleHSVCalibration();

  /**
   * Toggles the Gesture recognition mode. When it is turned on, the program
   * starts extracting finger tips from the video feed and performs the
   * necessary actions
   */
  void ToggleGestureRecognitionMode();

  /**
   * Draws all the UI elements on screen: the combined filter, the convex hulls
   * image,etc
   */
  void Draw();

  /**
   * Extracts finger tips from the input video frame, tracks both hands, and
   * @return
   */
  const std::vector<cv::Point>& Update();

  /**
   * Translates the coordinates of the fingertips to that of the cinder program.
   * @param points              a vector of points to be translated
   * @param input_height        the height of the input image
   * @param input_width         the width of the input image
   * @param ext_window_height   the height of the cinder program window.
   * @param ext_window_width    the width of the cinder program window
   * @return                    the translated points.
   */
  static std::vector<cv::Point> ConvertCoordinates(
      const std::vector<cv::Point>& points, int input_height, int input_width,
      int ext_window_height, int ext_window_width);

 private:
  const std::string CONVEX_HULL_WINDOW_NAME_;
  const int PIANO_CIRCLE_RADIUS;
  const int FINGER_TIP_CIRCLE_RADIUS_;
  const int FINGER_TIP_CIRCLE_THICKNESS_;
  const std::string HSV_WINDOW_NAME_;
  const std::string BACKGROUND_SUB_WINDOW_NAME_;
  const std::string COMBINED_WINDOW_NAME_;
  const cv::Size OUTPUT_WINDOW_SIZE_;
  const int LINE_TYPE_;
  const cv::Scalar COLOR_1 = cv::Scalar(0, 255, 200);
  const cv::Scalar COLOR_2 = cv::Scalar(255, 0, 200);

  // A videoCapture object to capture the webcam stream.
  cv::VideoCapture video_capture_;
  HandExtractor hand_extractor_;
  Calibration calibration_;
  bool recognition_mode_;  // Whether or not the program should detect gestures.
  // I added a tracker for two hands.
  HandTracker left_hand_tracker_;
  HandTracker right_hand_tracker_;

  cv::Mat
      image;  // Original image from the webcam is copied here for each frame.
  cv::Mat hsv_filter_image_;  // The image after passing it through the HSV
                              // filter is copied here for each frame
  cv::Mat background_subtracted_image_;  // The image after passing it through
                                         // the background subtraction filter.
  cv::Mat convex_hull_image_;  // The image which contains all the features(i.e
                               // finger_tips)
  cv::Mat
      combined_filter_image_;  // The image after passing it through the
                               // background subtraction filter and HSV filter.

  std::vector<cv::Point>
      left_finger_tips;  // Stores the finger tips of the left hand
  std::vector<cv::Point>
      right_finger_tips;  // Stores the finger tips of the right hand
  std::vector<cv::Point>
      merged_click_points;  // Stores the points clicked by both hands.
};

}  // namespace gesturerecognition
#endif  // FINAL_PROJECT_GESTURE_WRAPPER_H
