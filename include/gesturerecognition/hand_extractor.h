//
// Created by Venkatesh on 11/18/2020.
//

#ifndef FINAL_PROJECT_HAND_EXTRACTOR_H
#define FINAL_PROJECT_HAND_EXTRACTOR_H

#include <iostream>
#include <opencv2/objdetect.hpp>
#include <opencv2/opencv.hpp>

#include "cinder/Cinder.h"
#include "stdio.h"

namespace gesturerecognition {

/**
 * A struct representing the Hand. Stores the finger tips of the hand, as well
 * as its center
 */

struct Hand {
  std::vector<cv::Point> finger_tips_;
  cv::Point center_of_palm_;
  Hand() : finger_tips_(), center_of_palm_(cv::Point(-1, -1)) {
  }
  Hand(const std::vector<cv::Point>& finger_tips,
       const cv::Point& center_of_palm)
      : finger_tips_(std::move(finger_tips)), center_of_palm_(center_of_palm) {
  }
  Hand(const std::pair<const std::vector<cv::Point>&, const cv::Point&>& pair)
      : finger_tips_(std::move(pair.first)), center_of_palm_(pair.second) {
  }
  int Hand::getNumberOfFingers() const {
    return finger_tips_.size();
  }
};

/**
 * Class which processes hands in images and extracts necessary information from
 * them.
 */
class HandExtractor {
 public:
  HandExtractor();
  /**
   * Finds the 2 largest contours in the contours list
   * @param contours : a vector of contours
   * @return the indices of the two largest contours(contours with largest
   * areas)
   */
  static std::pair<int, int> Find2LargestContours(
      const std::vector<std::vector<cv::Point>>& contours);

  /**
   * Extracts the hands in the image.
   * @param input_image : the inputted image
   * @return and image with convex hulls drawn around it.
   */

  std::pair<Hand, Hand> ExtractHands(const cv::Mat& input_image);

 private:
  /**
   * Finds the center along with the finger tips of the inputted contour
   * @param contour         contour of the hand
   * @return                a pair containing vector of points representing
   *                        finger_tips, and a point representing the center of
   *                        the palm
   */
  std::pair<std::vector<cv::Point>, cv::Point> FindHandFeatures(
      std::vector<cv::Point>& contour);

  /**
   * Finds points around the convexity defects. These are points near the
   * fingertips.
   * @param convexity_defects   a vector containing the convexity defects of the
   *                            hand
   * @param finger_length       the shortest length a finger can have
   * @param contour             the contour of the hand(found with OpenCV's
   * findContours function)
   * @return                    a vector of points around the convexity defects.
   */
  std::vector<cv::Point> FindFingerDefects(
      const std::vector<cv::Vec4i>& convexity_defects, double finger_length,
      std::vector<cv::Point> contour);

  /**
   * Finds the center of the input rectangle.
   * @param bounding_rectangle  a cv::Rect object representing the rectangle
   *                            whose center is to be found
   * @return                    the center of the input rectangle
   */
  cv::Point FindCenterOfRectangle(cv::Rect bounding_rectangle);

  /**
   * Finds the fingertips of the hand, by filtering the points returned by
   * FindFingerDefects
   * @param finger_tips q       the approximate finger_tips returned by
   * FindFingerDefects
   * @param bounding_rectangle  the rectangle bounding the hand.
   * @return                    the finger tips of the hand
   */
  std::vector<cv::Point> FindFingerTips(
      const std::vector<cv::Point>& finger_tips, cv::Rect bounding_rectangle);
  const int MAX_ANGLE_BETWEEN_FINGERS_ = 95;
  const int LOWEST_FINGER_RATIO = 10;
  const int MIN_HAND_SIZE = 1000;
};

/**
 * Finds the euclidean distance between two points
 * @param a first point
 * @param b second point
 * @return the euclidean distance of a,b
 */
double FindEuclideanDistance(cv::Point a, cv::Point b);

/**
 * Finds the angle of the middle point using the cosine law
 * @param a first end point
 * @param b second end point( the angle at this point is returned)
 * @param c third point
 * @return  angle ABC.
 */
double CalculateAngle(cv::Point a, cv::Point b, cv::Point c);
const int ERROR_NUMBER = -1;

}  // namespace gesturerecognition
#endif  // FINAL_PROJECT_HAND_EXTRACTOR_H