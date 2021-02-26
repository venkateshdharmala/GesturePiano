//
// Created by Venkatesh on 11/28/2020.
//
#include "gesturerecognition/hand_tracker.h"

namespace gesturerecognition {
HandTracker::HandTracker(size_t number_of_frames)
    : number_of_frames_(number_of_frames), previous_batch_hand() {
}

bool ComparePoints(cv::Point point_1, cv::Point point_2) {
  return point_1.x < point_2.x;
}

std::vector<cv::Point> HandTracker::FindClickPoints(const Hand& hand) {
  hands.push_back(hand);
  if (hands.size() == number_of_frames_) {
    size_t frequent_number_of_fingers = FindMostFrequentFingerNumber(hands);
    current_batch_hand =
        hands[GetLatestReferenceFrame(hands, frequent_number_of_fingers)];
    AnalyseHand(MAX_CHANGE_IN_FINGER_POSITION_);
    previous_batch_hand = current_batch_hand;
    hands.clear();
  }
  return click_points;
}

void HandTracker::UpdatePoints(
    const std::vector<cv::Point>& previous_finger_tips,
    const std::vector<cv::Point>& current_finger_tips, double tolerance) {
  int size_difference = current_batch_hand.getNumberOfFingers() -
                        previous_batch_hand.getNumberOfFingers();
  // This gives the number of fingers that were bent/unbent
  if (size_difference == 0) {
    return;
  }
  if (size_difference > 0) {
    // This means a finger has been unbent, so we must unclick the button
    UnclickFingers(previous_finger_tips, current_finger_tips, tolerance,
                   size_difference);
  } else {
    // This means a finger has been unbent, so we must click the button

    ClickFingers(previous_finger_tips, current_finger_tips, tolerance,
                 abs(size_difference));
  }
}

void HandTracker::UnclickFingers(
    const std::vector<cv::Point>& previous_finger_tips,
    const std::vector<cv::Point>& current_finger_tips, double tolerance,
    size_t size_difference) {
  int previous_hand_finger_index = 0;
  int points_unclicked = 0;

  for (int current_hand_finger_index = 0;
       current_hand_finger_index < current_finger_tips.size();
       ++current_hand_finger_index) {
    if (previous_hand_finger_index >= previous_finger_tips.size()) {
      // This means the right-most finger in the previous hand was unclicked.
      auto point_to_remove = current_finger_tips.at(current_hand_finger_index);
      int index_to_remove =
          FindIndexOfClosestPoint(click_points, point_to_remove);
      if (!click_points.empty()) {
        click_points.erase(click_points.begin() + index_to_remove);
      }
    } else if (FindEuclideanDistance(
                   current_finger_tips[current_hand_finger_index],
                   previous_finger_tips[previous_hand_finger_index]) >
               tolerance) {
      // If there is a noticeable difference between ith element in current and
      // previous hand fingers, then the ith finger (in current
      // batch hand) was unpressed.
      auto point_to_remove = current_finger_tips.at(current_hand_finger_index);
      int index_to_remove =
          FindIndexOfClosestPoint(click_points, point_to_remove);
      if (!click_points.empty()) {
        // We remove the point closest to the unbent finger in click points.
        click_points.erase(click_points.begin() + index_to_remove);
        ++points_unclicked;

        // When there's an extra finger, we need to decrement the index of
        // previous fingertips, so that we compare the same type of fingers in
        // previous and current batch hands
        --previous_hand_finger_index;
      }
      if (points_unclicked == size_difference) {
        // Once we've unclicked the necessary amount of points, we can
        // break the loop
        break;
      }
    }
    ++previous_hand_finger_index;
  }
}

void HandTracker::ClickFingers(
    const std::vector<cv::Point>& previous_finger_tips,
    const std::vector<cv::Point>& current_finger_tips, double tolerance,
    size_t size_difference) {
  int points_clicked = 0;
  int current_hand_finger_index = 0;
  for (int previous_hand_finger_index = 0;
       previous_hand_finger_index < previous_finger_tips.size();
       ++previous_hand_finger_index) {
    if (current_hand_finger_index >= current_finger_tips.size()) {
      // This means the right-most finger in the previous hand was pressed.
      auto point_to_click = previous_finger_tips.at(previous_hand_finger_index);
      click_points.push_back(point_to_click);

    } else if (FindEuclideanDistance(
                   current_finger_tips[current_hand_finger_index],
                   previous_finger_tips[previous_hand_finger_index]) >
               tolerance) {
      // If there is a noticeable difference between ith element in current and
      // previous hand fingers, then the ith finger (in previous
      // batch hand) was pressed
      click_points.push_back(previous_finger_tips[previous_hand_finger_index]);
      ++points_clicked;

      // When there's a missing finger, we need to decrement the index of
      // current fingertips, so that we compare the same type of fingers in
      // previous and current batch hands
      --current_hand_finger_index;
      if (points_clicked == (size_difference)) {
        // Once we've pressed the necessary amount of points, we can
        // break the loop
        break;
      }
    }
    ++current_hand_finger_index;
  }
}

double FindSlopeOfLine(const cv::Point finger_tip,
                       const cv::Point palm_center) {
  if (finger_tip.x == palm_center.x) {
    return 90;
  }
  auto tan_value =
      (finger_tip.y - palm_center.y) / (finger_tip.x - palm_center.x);
  return atan(tan_value) * 180 / CV_PI;
}

void HandTracker::AnalyseHand(double tolerance) {
  if (current_batch_hand.finger_tips_.size() >= 5) {
    // If the current hand is an open palm, we just need unclick all previously
    // clicked points
    click_points.clear();
    return;
  }
  if (current_batch_hand.finger_tips_.size() == 0) {
    // If the current hand is a closed palm. we just need to click all
    // previously unclicked points.
    click_points.insert(click_points.end(),
                        previous_batch_hand.finger_tips_.begin(),
                        previous_batch_hand.finger_tips_.end());
  }
  if (previous_batch_hand.finger_tips_.size() !=
          current_batch_hand.finger_tips_.size() &&
      previous_batch_hand.center_of_palm_.x != ERROR_NUMBER) {
    std::sort(current_batch_hand.finger_tips_.begin(),
              current_batch_hand.finger_tips_.end(), ComparePoints);
    std::sort(previous_batch_hand.finger_tips_.begin(),
              previous_batch_hand.finger_tips_.end(), ComparePoints);
    UpdatePoints(previous_batch_hand.finger_tips_,
                 current_batch_hand.finger_tips_, tolerance);
  }
}

int FindIndexOfClosestPoint(const std::vector<cv::Point>& points_vector,
                            const cv::Point& point) {
  double maximum_distance = INT32_MAX;
  int index_of_closest_point = ERROR_NUMBER;
  for (size_t i = 0; i < points_vector.size(); ++i) {
    double distance = FindEuclideanDistance(points_vector[i], point);
    if (distance < maximum_distance) {
      index_of_closest_point = i;
      maximum_distance = distance;
    }
  }
  return index_of_closest_point;
}

int HandTracker::FindMostFrequentFingerNumber(const std::vector<Hand>& hands) {
  std::vector<int> counts_vector{0, 0, 0, 0, 0, 0};
  for (size_t i = 0; i < hands.size(); ++i) {
    if (hands[i].getNumberOfFingers() <= 5) {
      ++counts_vector[hands[i].getNumberOfFingers()];
    }
  }
  return std::max_element(counts_vector.begin(), counts_vector.end()) -
         counts_vector.begin();
}

int HandTracker::GetLatestReferenceFrame(const std::vector<Hand>& hands,
                                         int frequent_number) {
  for (int i = hands.size() - 1; i > 0; --i) {
    if (hands.at(i).getNumberOfFingers() == frequent_number) {
      return i;
    }
  }
  return 0;
}

}  // namespace gesturerecognition
