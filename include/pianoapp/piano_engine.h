//
// Created by Venkatesh on 11/30/2020.
//

#ifndef FINAL_PROJECT_PIANO_ENGINE_H
#define FINAL_PROJECT_PIANO_ENGINE_H
#include <cinder/app/AppBase.h>

#include <opencv2/opencv.hpp>

#include "cinder/Area.h"
#include "cinder/Cinder.h"
#include "cinder/CinderResources.h"
#include "cinder/Vector.h"
#include "cinder/audio/Voice.h"
#include "cinder/gl/gl.h"

namespace piano {

/**
 * A struct representing a piano key. If it is a white key(flat key in this
 * case), it has a pointer to its corresponding black key(sharp key)
 */
struct Key {
  Key() = default;
  Key(const ci::Rectf& rect, const std::string& audio_file_name,
      const std::string note_name)
      : rectangular_region(rect),
        audio_file_name(audio_file_name),
        black_key_ptr(nullptr),
        note_name(note_name) {
    ci::audio::SourceFileRef source_file =
        ci::audio::load(cinder::app::loadAsset(audio_file_name));
    key_sound = ci::audio::Voice::create(source_file);
    key_sound->setVolume(2.0f);
  }

  Key(const ci::Rectf& rect, const std::string& audio_file_name,
      Key* black_key_ptr, const std::string note_name)
      : rectangular_region(rect),
        audio_file_name(audio_file_name),
        black_key_ptr(black_key_ptr),
        note_name(note_name) {
    ci::audio::SourceFileRef source_file =
        ci::audio::load(cinder::app::loadAsset(audio_file_name));
    key_sound = ci::audio::Voice::create(source_file);
    key_sound->setVolume(2.0f);
  }
  const ci::Rectf rectangular_region;
  ci::audio::VoiceRef key_sound;
  ci::vec2 trigger_point;
  const std::string note_name;
  const Key* black_key_ptr;
  const std::string audio_file_name;
};

/**This class manages the whole piano app.
 */
class PianoEngine {
 public:
  /**
   * Constructor
   * @param top_left_corner           the top left corner of the application
   *                                  window
   * @param window_width              the width of the window
   * @param window_height             the height of the window
   * @param row_margin                the vertical space between keys in two
   *                                  separate rows
   * @param number_of_white_keys      the total number of white keys
   * @param number_of_rows            the total number of rows in the piano.
   * @param file_name                 the file name which contains the order of
   *                                  notes that the user wants the piano to
   * have
   */
  PianoEngine(const cv::Point& top_left_corner, double window_width,
              double window_height, int row_margin, int number_of_white_keys,
              int number_of_rows, const std::string& file_name);

  /**
   * Draws all the keys on to the application window. Used in the cinder draw
   * function
   */
  void DrawKeys();
  /**
   * Updates the state of the piano by responding to the inputted click points.
   * @param points the points on the piano which are to be clicked.
   */
  void Run(const std::vector<cv::Point>& points);

  const std::vector<Key>& getWhiteKeys();
  const std::vector<Key>& getBlackKeys();
  const std::unordered_map<float, Key>& getPressedKeys();

  /**
   * Checks if the inputted point lies on a black key. A helper function made
   * public for testing.
   * @param point:          the input point
   * @param previous_key:   the key before the nearest white key
   * @param key:            the key nearest to the input point
   * @return                0.5 if the point lies on the key, -0.5 if it lies on
   *                        the previous key, and 0 otherwise
   */
  static double CheckIfPointOnBlackKey(const cv::Point& point,
                                       const Key& previous_key, const Key& key);

 private:
  /**
   * Plays the inputted key if it isnt playing already.
   * @param key
   */
  void PlayKey(Key& key);

  /**
   * Unplays the inputted key if it is playing
   */
  void UnplayKey(Key& key);

  /**
   * Sets up the initial state of keys : from positioning to matching the notes.
   */
  void SetupKeys();

  /**
   * Gets the index of the nearest white key to the point. Index returned is
   * with respect to the white_keys vector.
   * @param point the inputted point
   * @return the nearest white key index.
   */
  float GetKeyIndexAtPoint(const cv::Point& point);
  const double CORNER_RADIUS_OF_KEYS = 0.5;
  const double BLACK_KEY_WIDTH_BY_WHITE_KEY_WIDTH = 0.4;
  const double BLACK_KEY_HEIGHT_BY_WHITE_KEY_HEIGHT = 0.66;
  std::ifstream stream_reader_;
  std::string audio_file_name_prefix_;
  std::string audio_file_name_suffix_;
  std::vector<Key> white_keys_;
  int row_margin_;
  std::vector<Key> black_keys_;
  int number_of_white_keys_;
  int number_of_rows_;
  int number_of_keys_in_row_;
  ci::Rectf window_region_;
  double white_key_width_;
  double white_key_height_;
  std::unordered_map<float, Key> pressed_keys_;
};
/**
 * Converts an OpenCV point to a cinder/glm point
 * @param point opencv point
 * @return the glm version of point.
 */
glm::vec2 ConvertToVec2(const cv::Point& point);
}  // namespace piano
#endif  // FINAL_PROJECT_PIANO_ENGINE_H
