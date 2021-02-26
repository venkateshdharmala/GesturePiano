#include <final_project_app.h>

using finalproject::FinalProjectApp;

void prepareSettings(FinalProjectApp::Settings* settings) {
  settings->setResizable(false);
  settings->setFrameRate(10);
}

// This line is a macro that expands into an "int main()" function.
CINDER_APP(FinalProjectApp, ci::app::RendererGl, prepareSettings);
