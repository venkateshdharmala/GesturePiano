#pragma once

#include "cinder/Cinder.h"
#include "cinder/CinderResources.h"
#include "cinder/ImageIo.h"
#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "pianoapp/piano_engine.h"
#include "gesturerecognition/calibration.h"
#include "gesturerecognition/hand_extractor.h"
#include "gesturerecognition/hand_tracker.h"
#include "gesturerecognition/gesture_wrapper.h"


namespace finalproject {
/**
 * This class integrates the gesture recognition program with the piano app in cinder. This class is run with the CINDER_APP function.
 */

class FinalProjectApp : public ci::app::App {
 public:
  FinalProjectApp();
  /**
   * Overridden cinder update method : used to update state of elements. Cinder runs this method in an endless loop.
   */
  void update() override;

  /**
   * Overridden cinder draw method: used to draw elements on screen.Cinder runs this method in an endless loop
   */
  void draw() override;

  /**
   * Overridden cinder key_down method. Used to handle key presses when the program is running
   * @param event : the key pressed
   */
  void keyDown(ci::app::KeyEvent event) override;

 private:
  gesturerecognition::ProgramSettings settings;//Loads all settings from config_file.
  gesturerecognition::GestureWrapper gesture_wrapper;
  piano::PianoEngine piano_engine;
};
}  // namespace finalproject
