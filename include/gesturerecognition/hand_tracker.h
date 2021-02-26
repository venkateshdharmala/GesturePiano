//
// Created by Venkatesh on 11/28/2020.
//

#ifndef FINAL_PROJECT_HAND_TRACKER_H
#define FINAL_PROJECT_HAND_TRACKER_H
#include "hand_extractor.h"

namespace gesturerecognition {

class HandTracker {
 public:
  /**
   * Default constructor
   *@param number_of_frames : the number of frame that each batch holds.
   */
  HandTracker(size_t number_of_frames);

  /**
   * Finds points clicked by the fingers. Returns an empty vector if there are
   * no points clicked
   * @param hand     the Hand object that we want to track.
   * @return        the points clicked.
   */
  std::vector<cv::Point> FindClickPoints(const Hand& hand);

 private:
  /**
   * Finds the most common number of fingers open in a batch, by counting the
   * number of fingers in each frame of the batch.
   * @param hands   the Hand object that we want to track.
   * @return        the most common number of fingers open in a batch
   */
  int FindMostFrequentFingerNumber(const std::vector<Hand>& hands);

  /**
   * Gets the latest frame in the batch which has the most common number of
   * fingers open.
   * @param hands               the Hand object that we want to track.
   * @param frequent_number     the most frequent number of fingers in the batch
   * @return
   */
  int GetLatestReferenceFrame(const std::vector<Hand>& hands,
                              int frequent_number);

  /**
   * Compares the current batch hand and previous batch hand and performs
   * clicks/unclicks when required.
   * @param tolerance  the maximum distance that a finger can move without being
   *                    detected as clicked.
   */
  void AnalyseHand(double tolerance);

  /**
   * Updates clicked and unclicked finger tips.
   * @param previous_hand_fingers   vector of fingertips of the previous batches
   *                                hand
   * @param current_hand_fingers    vector of fingertips of the current batches
   *                                hand
   * @param tolerance               the maximum distance that a finger can move
   *                                without being detected as clicked.
   */
  void UpdatePoints(const std::vector<cv::Point>& previous_hand_fingers,
                    const std::vector<cv::Point>& current_hand_fingers,
                    double tolerance);

  /**
   * Finds the finger tip(s) clicked and adds them to click_pts
   * @param previous_hand_fingers   vector of fingertips of the previous batches
   *                                hand
   * @param current_hand_fingers    vector of fingertips of the current batches
   *                                hand
   * @param tolerance               the maximum distance that a finger can move
   *                                without being detected as clicked.
   * @param size_difference
   */
  void ClickFingers(const std::vector<cv::Point>& previous_hand_fingers,
                    const std::vector<cv::Point>& current_hand_fingers,
                    double tolerance, size_t size_difference);

  /**
   * Finds finger tip(s) unclicked and removes them from click_pts
   * @param previous_hand_fingers   vector of fingertips of the previous batches
   *                                reference hand
   * @param current_hand_fingers    vector of fingertips of the current batches
   *                                reference hand
   * @param tolerance               the maximum distance that a finger can move
   *                                without being detected as clicked.
   * @param size_difference         the absolute difference in size of
   *                                previous_hand_fingers and
   * current_hand_fingers
   */
  void UnclickFingers(const std::vector<cv::Point>& previous_hand_fingers,
                      const std::vector<cv::Point>& current_hand_fingers,
                      double tolerance, size_t size_difference);

  const size_t number_of_frames_;
  Hand previous_batch_hand;
  Hand current_batch_hand;
  std::vector<cv::Point> click_points;
  std::vector<Hand> hands;
  const int MAX_CHANGE_IN_FINGER_POSITION_ = 20;
};
/**
 * A comparator function to sort a points vector by ascending order of the
 * points'      dinates
 * @param point_1: the first point to be compareed.
 * @param point_2: the second point to be compared
 * @return true if point_1.x > point_2.x, false otherwise.
 */
bool ComparePoints(cv::Point point_1, cv::Point point_2);

/**
 * Finds the index of the point(in points_vector) that is nearest to the input
 * point
 * @param points_vector: a vector of points out of which we need the closest
 * point
 * @param point:  input point
 * @return index of closest point with respect to points_vector.
 */
int FindIndexOfClosestPoint(const std::vector<cv::Point>& points_vector,
                            const cv::Point& point);
}  // namespace gesturerecognition
#endif  // FINAL_PROJECT_HAND_TRACKER_H
