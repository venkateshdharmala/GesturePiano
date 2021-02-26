//
// Created by Venkatesh on 11/30/2020.
//

#include "pianoapp/piano_engine.h"

namespace piano {
PianoEngine::PianoEngine(const cv::Point& top_left_corner, double window_width,
                         double window_height, int row_margin,
                         int number_of_white_keys, int number_of_rows,
                         const std::string& file_name)
    : window_region_(ConvertToVec2(top_left_corner),
                     glm::vec2((top_left_corner.x + window_width,
                                top_left_corner.y + window_height))),
      number_of_rows_(number_of_rows),
      row_margin_(row_margin),
      number_of_white_keys_(number_of_white_keys),
      number_of_keys_in_row_(number_of_white_keys_ / number_of_rows),
      white_key_height_((window_region_.getHeight() / number_of_rows) -
                        row_margin),
      white_key_width_(window_region_.getWidth() / number_of_keys_in_row_) {
  stream_reader_.open(file_name);
  stream_reader_ >> audio_file_name_prefix_;
  stream_reader_ >> audio_file_name_suffix_;
  SetupKeys();
}

void PianoEngine::SetupKeys() {
  if (number_of_white_keys_ % number_of_rows_ != 0 ||
      number_of_white_keys_ < 0) {
    throw std::invalid_argument(
        "Number of Rows must be a factor of number of white keys!");
  }
  /*We reserve number_of_white_keys_ space to prevent the vector from being
    reallocated when elements are pushed back. When it is reallocated, the
    pointers to the black keys in white keys are invalidated*/
  black_keys_.reserve(number_of_white_keys_);
  std::string white_key_audio_file_name;
  std::string black_key_audio_file_name;
  stream_reader_ >> white_key_audio_file_name;

  glm::vec2 start_point = window_region_.getLowerLeft();
  for (int row_number = number_of_rows_; row_number > 0; --row_number) {
    for (size_t j = 0; j < number_of_keys_in_row_; ++j) {
      glm::vec2 white_key_start_point = glm::vec2(
          (start_point.x + j * white_key_width_),
          (start_point.y - row_number * (white_key_height_ + row_margin_)));
      glm::vec2 white_key_end_point(
          (white_key_start_point.x + white_key_width_),
          (white_key_start_point.y + white_key_height_));
      if (j != 0) {
        // We want the first key of a row to be the same as the last key of the
        // previous row.
        stream_reader_ >> white_key_audio_file_name;
      }

      char note = white_key_audio_file_name.at(0);
      if (note == 'B' || note == 'E' || j == number_of_keys_in_row_ - 1) {
        // There is no B# or E# in a piano, so we just add a white key
        Key key(
            ci::Rectf(white_key_start_point, glm::vec2((white_key_end_point))),
            audio_file_name_prefix_ + white_key_audio_file_name +
                audio_file_name_suffix_,
            white_key_audio_file_name);
        white_keys_.push_back(key);
      } else {
        glm::vec2 black_key_start_point =
            glm::vec2((white_key_start_point.x + 0.75 * white_key_width_),
                      white_key_start_point.y);
        glm::vec2 black_key_end_point = glm::vec2(
            (black_key_start_point.x +
             BLACK_KEY_WIDTH_BY_WHITE_KEY_WIDTH * white_key_width_),
            (white_key_start_point.y +
             (white_key_height_ * BLACK_KEY_HEIGHT_BY_WHITE_KEY_HEIGHT)));
        stream_reader_ >> black_key_audio_file_name;
        Key black_key(ci::Rectf(black_key_start_point, black_key_end_point),
                      audio_file_name_prefix_ + black_key_audio_file_name +
                          audio_file_name_suffix_,
                      black_key_audio_file_name);
        black_keys_.push_back(black_key);
        Key white_key(
            ci::Rectf(white_key_start_point, glm::vec2((white_key_end_point))),
            audio_file_name_prefix_ + white_key_audio_file_name +
                audio_file_name_suffix_,
            &(*black_keys_.rbegin()), white_key_audio_file_name);
        white_keys_.push_back(white_key);
      }
    }
  }
  return;
}

void PianoEngine::DrawKeys() {
  for (const Key& white_key : white_keys_) {
    ci::gl::color(ci::Color("white"));
    ci::gl::drawSolidRoundedRect(white_key.rectangular_region,
                                 CORNER_RADIUS_OF_KEYS);
    ci::gl::color(ci::Color("black"));
    ci::gl::drawStrokedRect(white_key.rectangular_region);
  }
  for (const Key& black_key : black_keys_) {
    ci::gl::color(ci::Color("black"));
    ci::gl::drawSolidRoundedRect(black_key.rectangular_region,
                                 CORNER_RADIUS_OF_KEYS);
  }
  for (auto pair : pressed_keys_) {
    ci::gl::color(ci::Color("red"));
    ci::gl::drawSolidRoundedRect(pair.second.rectangular_region,
                                 CORNER_RADIUS_OF_KEYS);
  }
}

void PianoEngine::Run(const std::vector<cv::Point>& points) {
  std::vector<double> indexes_of_keys;
  std::vector<double> keys_to_remove;

  for (cv::Point point : points) {
    // We convert the point to the coordinate system of the piano.
    // We check for any additional piano keys to be pressed.
    float key_index = GetKeyIndexAtPoint(point);
    if (key_index < white_keys_.size()) {
      indexes_of_keys.push_back(key_index);
      if (pressed_keys_.find(key_index) == pressed_keys_.end()) {
        // If pressed_keys does not have the key, then we add that key.
        if (std::floor(key_index) != key_index) {
          // If the key_value ends with 0.5, we add the black key
          pressed_keys_.insert(
              {(key_index),
               *white_keys_.at(static_cast<int>(key_index)).black_key_ptr});
        } else {
          pressed_keys_.insert(
              {(key_index), white_keys_.at(static_cast<int>(key_index))});
        }
        PlayKey(pressed_keys_.at(key_index));
      }
    }
  }
  if (pressed_keys_.size() != points.size() && pressed_keys_.size() != 0) {
    // This means there are points which are unclicked, and the piano  keys
    // associated with them must be unplayed.
    for (auto iterator = pressed_keys_.begin(); iterator != pressed_keys_.end();
         ++iterator) {
      if (std::find(indexes_of_keys.begin(), indexes_of_keys.end(),
                    iterator->first) == indexes_of_keys.end()) {
        UnplayKey(pressed_keys_.at(iterator->first));
        keys_to_remove.push_back(iterator->first);
      }
    }
    for (auto keytr : keys_to_remove) {
      pressed_keys_.erase(keytr);
    }
  }
}

glm::vec2 ConvertToVec2(const cv::Point& point) {
  return glm::vec2(point.x, point.y);
}

float PianoEngine::GetKeyIndexAtPoint(const cv::Point& point) {
  if (!window_region_.contains(ConvertToVec2(point))) {
    return static_cast<float>(white_keys_.size());
  }
  double marginal_value;
  int column_number = static_cast<int>(number_of_keys_in_row_ * point.x /
                                       window_region_.getWidth());
  int row_number =
      static_cast<int>(number_of_rows_ * point.y / window_region_.getHeight());
  int index_to_return = row_number * number_of_keys_in_row_ + column_number;
  if (index_to_return == 0) {
    // Since 0 -1 = 1 is not a valid index
    marginal_value =
        CheckIfPointOnBlackKey(point, white_keys_[white_keys_.size() - 1],
                               white_keys_[index_to_return]);
  } else {
    marginal_value = CheckIfPointOnBlackKey(
        point, white_keys_[index_to_return - 1], white_keys_[index_to_return]);
  }
  return index_to_return + marginal_value;
}
void PianoEngine::PlayKey(Key& key) {
  if (!key.key_sound->isPlaying()) {
    key.key_sound->start();
  }
}

double PianoEngine::CheckIfPointOnBlackKey(const cv::Point& point,
                                           const Key& previous_key,
                                           const Key& key) {
  if (previous_key.black_key_ptr != nullptr &&
      previous_key.black_key_ptr->rectangular_region.contains(
          ConvertToVec2(point))) {
    return -0.5;
  }
  if (key.black_key_ptr != nullptr &&
      key.black_key_ptr->rectangular_region.contains(ConvertToVec2(point))) {
    return 0.5;
  }
  return 0;
}
void PianoEngine::UnplayKey(Key& key) {
  if (key.key_sound->isPlaying()) {
    key.key_sound->stop();
  }
}

const std::vector<Key>& PianoEngine::getBlackKeys() {
  return black_keys_;
}
const std::vector<Key>& PianoEngine::getWhiteKeys() {
  return white_keys_;
}
const std::unordered_map<float, Key>& PianoEngine::getPressedKeys() {
  return pressed_keys_;
}
}  // namespace piano
