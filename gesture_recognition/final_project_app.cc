#include <final_project_app.h>

namespace finalproject {

FinalProjectApp::FinalProjectApp()
    : settings("config.json"),
      gesture_wrapper(settings),
      piano_engine(cv::Point(0, 0), settings.output_window_size.width,
                   settings.output_window_size.height, settings.row_margin,
                   settings.number_of_white_keys, settings.number_of_rows,
                   settings.piano_notes_file_name)

{
  ci::app::setWindowSize(settings.output_window_size.width,
                         settings.output_window_size.height);
}

void FinalProjectApp::draw() {
  ci::gl::clear(ci::ColorA::black());
  piano_engine.DrawKeys();
  gesture_wrapper.Draw();
}

void FinalProjectApp::update() {
  piano_engine.Run(gesture_wrapper.Update());
}

void FinalProjectApp::keyDown(ci::app::KeyEvent event) {
  switch (event.getCode()) {
    case ci::app::KeyEvent::KEY_h:
      gesture_wrapper.ToggleHSVCalibration();
      break;
    case ci::app::KeyEvent::KEY_b:
      gesture_wrapper.ToggleBackgroundCalibration();
      break;
    case ci::app::KeyEvent::KEY_y:
      gesture_wrapper.ToggleGestureRecognitionMode();
      break;
    case ci::app::KeyEvent::KEY_ESCAPE:
      quit();
  }
}
}  // namespace finalproject
