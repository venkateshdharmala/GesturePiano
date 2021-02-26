//
// Created by Venkatesh on 11/18/2020.
//

#include "gesturerecognition/hand_extractor.h"

namespace gesturerecognition {

HandExtractor::HandExtractor() {
}

std::pair<int, int> HandExtractor::Find2LargestContours(
    const std::vector<std::vector<cv::Point>>& contours) {
  if (contours.size() < 1) {
    // We need two hands
    return std::make_pair(ERROR_NUMBER, ERROR_NUMBER);
  }
  if (contours.size() == 2) {
    return std::make_pair(0, 1);
  }

  int max_1_index = 0;
  int max_2_index = 0;
  double max_1_area = 0;
  double max_2_area = 0;

  for (size_t i = 0; i < contours.size(); ++i) {
    double contour_area = cv::contourArea(contours[i]);
    if (contour_area > max_1_area) {
      max_2_index = max_1_index;
      max_2_area = max_1_area;
      max_1_index = i;
      max_1_area = contour_area;
    } else if (contour_area > max_2_area) {
      max_2_index = i;
      max_2_area = contour_area;
    }
  }
  if (contours[max_1_index][0].x >= contours[max_2_index][0].x) {
    std::swap(max_1_index, max_2_index);
  }

  return std::make_pair(max_1_index, max_2_index);
}

std::pair<Hand, Hand> HandExtractor::ExtractHands(const cv::Mat& input_image) {
  using namespace cv;
  Mat clone = input_image.clone(); /*Cloning the image as findContours will
                                    change it*/
  std::vector<std::vector<Point>> contours;
  try {
    findContours(input_image.clone(), contours, RETR_EXTERNAL,
                 CHAIN_APPROX_SIMPLE);
    std::pair<int, int> max_contour_indices = Find2LargestContours(contours);

    if (max_contour_indices.first == ERROR_NUMBER) {
      // Returning a default hand object if there is no contour.
      Hand hand;
      return std::make_pair(hand, hand);
    }

    std::vector<cv::Point> contour_1 = contours[max_contour_indices.first];
    std::vector<cv::Point> contour_2 = contours[max_contour_indices.second];
    Hand hand_1(FindHandFeatures(contour_1));
    Hand hand_2(FindHandFeatures(contour_2));
    if (hand_1.center_of_palm_.x > hand_2.center_of_palm_.x) {
      // We make sure to return left hand as 1st element in pair, and right as
      // 2nd
      return std::make_pair(hand_2, hand_1);
    }
    return std::make_pair(hand_1, hand_2);
  } catch (cv::Exception& e) {
    Hand hand;
    return std::make_pair(hand, hand);
  }
}

std::pair<std::vector<cv::Point>, cv::Point> HandExtractor::FindHandFeatures(
    std::vector<cv::Point>& contour) {
  using namespace cv;
  std::vector<cv::Point> convex_hull_pts;
  std::vector<int> convex_hull_indices;

  /*
   * Convex Hull indices are the indices of certain points in each contour (A
   * contour is a vector of points). These certain points make up vertices of
   * the convex hull of the points. Convex Hull points are just the point form
   * of the convex hull vertices
   *
   */
  convexHull(contour, convex_hull_pts, false, true);
  convexHull(contour, convex_hull_indices, false, false);

  Rect bounding_rectangle = boundingRect(Mat(convex_hull_pts));
  Point center_of_rect = FindCenterOfRectangle(bounding_rectangle);

  // Convexity defects are somewhat similar to valleys in the convex hull. The
  // finger valley points will be present in convexity defects.
  std::vector<Vec4i> convexity_defects;

  if (contour.size() > 3 && cv::contourArea(contour) > MIN_HAND_SIZE) {
    // If there are not more than 3 convex hull points, the given shape is not
    // convex.
    convexityDefects(cv::Mat(contour), convex_hull_indices, convexity_defects);
  }
  auto unfiltered_finger_tips = FindFingerDefects(
      convexity_defects, bounding_rectangle.height / LOWEST_FINGER_RATIO,
      contour);
  auto finger_tips = FindFingerTips(unfiltered_finger_tips, bounding_rectangle);
  return std::make_pair(finger_tips, center_of_rect);
}

std::vector<cv::Point> HandExtractor::FindFingerTips(
    const std::vector<cv::Point>& finger_tips, cv::Rect bounding_rectangle) {
  std::vector<cv::Point> filtered_finger_tips = finger_tips;
  cv::Point center_of_rectangle = FindCenterOfRectangle(bounding_rectangle);
  int lowest_y_coordinate_of_finger =
      center_of_rectangle.y + bounding_rectangle.height / LOWEST_FINGER_RATIO;

  for (size_t i = 0; i < filtered_finger_tips.size(); ++i) {
    for (size_t j = i + 1; j < filtered_finger_tips.size(); ++j) {
      if (FindEuclideanDistance(filtered_finger_tips[i],
                                filtered_finger_tips[j]) <
          bounding_rectangle.width / LOWEST_FINGER_RATIO) {
        // If tips are closer than the minimum distance between two fingers, we
        // remove one of them as it is a duplicate.
        filtered_finger_tips.erase(filtered_finger_tips.begin() + j);
      }
    }
    if (filtered_finger_tips[i].y > lowest_y_coordinate_of_finger) {
      // If the tip is below the lowest possible level for a finger_tip, we
      // remove the point Here we use > instead of < as the y coordinate
      // decreases as we go lower.
      filtered_finger_tips.erase(filtered_finger_tips.begin() + i);
    }
  }
  return filtered_finger_tips;
}

cv::Point HandExtractor::FindCenterOfRectangle(cv::Rect bounding_rectangle) {
  return cv::Point(bounding_rectangle.x + bounding_rectangle.width / 2,
                   bounding_rectangle.y + bounding_rectangle.height / 2);
}

double FindEuclideanDistance(cv::Point a, cv::Point b) {
  return sqrt(pow(a.x - b.x, 2) + pow(a.y - b.y, 2));
}

double CalculateAngle(cv::Point a, cv::Point b, cv::Point c) {
  double length_ab = FindEuclideanDistance(a, b);
  double length_bc = FindEuclideanDistance(b, c);
  double length_ca = FindEuclideanDistance(c, a);
  double cosine_of_angle =
      (pow(length_ab, 2) + pow(length_bc, 2) - pow(length_ca, 2)) /
      (2 * (length_ab) * (length_bc));
  double angle = acos(cosine_of_angle);
  return angle * 180 / CV_PI;
}

std::vector<cv::Point> HandExtractor::FindFingerDefects(
    const std::vector<cv::Vec4i>& convexity_defects, double finger_length,
    std::vector<cv::Point> contour) {
  std::vector<cv::Point> finger_tips;
  int number_of_fingers = 0;
  for (int i = 0; i < convexity_defects.size(); ++i) {
    /*The start point and end points are the fingertips of the two fingers that
      make up a convexity defect. The far point is the middle point in the
      convexity defect. The end point of one convexity defect is the start
      point of the next convexity defect in the hand.*/
    cv::Point start_point = contour[convexity_defects[i][0]];
    cv::Point far_point = contour[convexity_defects[i][2]];
    cv::Point end_point = contour[convexity_defects[i][1]];

    if (FindEuclideanDistance(start_point, far_point) > finger_length &&
        FindEuclideanDistance(far_point, end_point) > finger_length &&
        CalculateAngle(start_point, far_point, end_point) <
            MAX_ANGLE_BETWEEN_FINGERS_) {
      finger_tips.push_back(start_point);
      finger_tips.push_back(end_point);
      ++number_of_fingers;
    }
  }
  return finger_tips;
}

}  // namespace gesturerecognition
