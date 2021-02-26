//
// Created by Venkatesh on 12/2/2020.
//

#include "gesturerecognition/gesture_wrapper.h"

namespace gesturerecognition {

GestureWrapper::GestureWrapper(const ProgramSettings& settings)
    : calibration_(0, settings.maximum_hsv_limit, settings.hsv_window_name,
                   settings.background_sub_window_name,
                   settings.combined_window_name,
                   settings.background_learning_rate, settings.hsv_window_size),
      OUTPUT_WINDOW_SIZE_(settings.output_window_size),
      LINE_TYPE_(settings.line_type),
      left_hand_tracker_((settings.frames_to_track)),
      right_hand_tracker_(settings.frames_to_track),
      recognition_mode_(false),
      video_capture_(settings.camera_number),
      hand_extractor_(),
      HSV_WINDOW_NAME_(settings.hsv_window_name),
      BACKGROUND_SUB_WINDOW_NAME_(settings.background_sub_window_name),
      COMBINED_WINDOW_NAME_(settings.combined_window_name),
      CONVEX_HULL_WINDOW_NAME_(settings.convex_hull_window_name),
      FINGER_TIP_CIRCLE_RADIUS_(settings.finger_tip_circle_radius),
      FINGER_TIP_CIRCLE_THICKNESS_(settings.finger_tip_circle_thickness),
      PIANO_CIRCLE_RADIUS(settings.piano_circle_radius) {
}

void GestureWrapper::ToggleGestureRecognitionMode() {
  calibration_.SetBackgroundTraining(false);
  calibration_.SetHSVCalibration(false);
  recognition_mode_ = !recognition_mode_;
  if (!recognition_mode_) {
    cv::destroyWindow(CONVEX_HULL_WINDOW_NAME_);
  }
}
void GestureWrapper::ToggleBackgroundCalibration() {
  recognition_mode_ = false;
  calibration_.SetBackgroundTraining(!calibration_.IsBackgroundTraining());
  if (!calibration_.IsBackgroundTraining()) {
    cv::destroyWindow(BACKGROUND_SUB_WINDOW_NAME_);
  }
  return;
}

void GestureWrapper::ToggleHSVCalibration() {
  recognition_mode_ = false;
  calibration_.SetHSVCalibration(!calibration_.IsHSVCalibrating());
  if (!calibration_.IsHSVCalibrating()) {
    cv::destroyWindow(HSV_WINDOW_NAME_);
    return;
  }
  calibration_.CreateTrackbars(255);
}

void GestureWrapper::Draw() {
  cv::imshow(COMBINED_WINDOW_NAME_, combined_filter_image_);

  if (recognition_mode_) {
    cv::imshow(CONVEX_HULL_WINDOW_NAME_, convex_hull_image_);

    for (auto pt : right_finger_tips) {
      ci::gl::color(ci::Color("blue"));
      ci::gl::drawSolidCircle(piano::ConvertToVec2(pt),
                              static_cast<float>(PIANO_CIRCLE_RADIUS));
    }
    for (auto pt : left_finger_tips) {
      ci::gl::color(ci::Color("pink"));
      ci::gl::drawSolidCircle(piano::ConvertToVec2(pt),
                              static_cast<float>(PIANO_CIRCLE_RADIUS));
    }
  }
  if (calibration_.IsHSVCalibrating()) {
    cv::imshow(HSV_WINDOW_NAME_, hsv_filter_image_);
  }
  if (calibration_.IsBackgroundTraining()) {
    cv::imshow(BACKGROUND_SUB_WINDOW_NAME_, background_subtracted_image_);
  }
}
std::vector<cv::Point> GestureWrapper::ConvertCoordinates(
    const std::vector<cv::Point>& points, int input_height, int input_width,
    int ext_window_height, int ext_window_width) {
  std::vector<cv::Point> converted_points;
  for (const cv::Point& point : points) {
    converted_points.push_back(cv::Point(
        static_cast<int>((ext_window_width * point.x / input_width)),
        static_cast<int>(ext_window_height / 10 +
                         (ext_window_height * point.y / input_height))));
  }
  return converted_points;
}
const std::vector<cv::Point>& GestureWrapper::Update() {
  video_capture_ >> image;

  // We laterally invert the image.
  cv::flip(image, image, 1);

  convex_hull_image_ = image.clone();
  combined_filter_image_ = calibration_.GetFinalFilterImage(image);

  if (calibration_.IsHSVCalibrating()) {
    hsv_filter_image_ = calibration_.FilterImageByHSV(image);
  }
  if (calibration_.IsBackgroundTraining()) {
    background_subtracted_image_ =
        calibration_.GetBackgroundSubtractedImage(image);
  }

  if (recognition_mode_) {
    auto hand_pair = hand_extractor_.ExtractHands(combined_filter_image_);
    gesturerecognition::Hand& hand_1 = hand_pair.first;
    gesturerecognition::Hand& hand_2 = hand_pair.second;
    left_finger_tips = ConvertCoordinates(
        hand_1.finger_tips_, image.size[0], image.size[1],
        OUTPUT_WINDOW_SIZE_.height, OUTPUT_WINDOW_SIZE_.width);
    right_finger_tips = ConvertCoordinates(
        hand_2.finger_tips_, image.size[0], image.size[1],
        OUTPUT_WINDOW_SIZE_.height, OUTPUT_WINDOW_SIZE_.width);
    ;
    // We get the left click points first and add to merge_click_points.
    merged_click_points = std::move(left_hand_tracker_.FindClickPoints(hand_1));

    std::vector<cv::Point> right_click_pts =
        right_hand_tracker_.FindClickPoints(hand_2);

    /* We could call FindClickPoints of right_hand_tracker directly into the
    merged_click_points move function. However, it would call the
    FindClickPoints(an expensive function performance-wise) function twice,*/
    merged_click_points.insert(merged_click_points.end(),
                               std::make_move_iterator(right_click_pts.begin()),
                               std::make_move_iterator(right_click_pts.end()));

    for (size_t i = 0; i < hand_1.finger_tips_.size(); ++i) {
      // We iterate through each finger tip and draw them on the convex hull
      // image
      circle(convex_hull_image_, hand_1.finger_tips_.at(i),
             FINGER_TIP_CIRCLE_RADIUS_, COLOR_1, FINGER_TIP_CIRCLE_THICKNESS_,
             LINE_TYPE_);
      putText(convex_hull_image_, std::to_string(i),
              cv::Point(hand_1.finger_tips_.at(i).x,
                        hand_1.finger_tips_.at(i).y + 20),
              cv::FONT_HERSHEY_SIMPLEX, 1, COLOR_1, cv::LINE_8, false);
      line(convex_hull_image_, hand_1.finger_tips_[i], hand_1.center_of_palm_,
           COLOR_1, FINGER_TIP_CIRCLE_THICKNESS_ / 3, LINE_TYPE_);
    }
    for (size_t i = 0; i < hand_2.finger_tips_.size(); ++i) {
      // We iterate through each finger tip and draw them on the convex hull
      // image
      circle(convex_hull_image_, hand_2.finger_tips_.at(i),
             FINGER_TIP_CIRCLE_RADIUS_, COLOR_2, FINGER_TIP_CIRCLE_THICKNESS_,
             LINE_TYPE_);
      putText(convex_hull_image_, std::to_string(i),
              cv::Point(hand_2.finger_tips_.at(i).x,
                        hand_2.finger_tips_.at(i).y + 20),
              cv::FONT_HERSHEY_SIMPLEX, 1, COLOR_2, cv::LINE_8, false);
      line(convex_hull_image_, hand_2.finger_tips_[i], hand_2.center_of_palm_,
           COLOR_2, FINGER_TIP_CIRCLE_THICKNESS_ / 3, LINE_TYPE_);
    }

    // We translate the click points to the desired coordinate system and return
    // them
    merged_click_points = ConvertCoordinates(
        merged_click_points, image.size[0], image.size[1],
        OUTPUT_WINDOW_SIZE_.height, OUTPUT_WINDOW_SIZE_.width);
    return merged_click_points;
  }
  // Merging the two arrays into one.
  merged_click_points.clear();
  return merged_click_points;
}

}  // namespace gesturerecognition