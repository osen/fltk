#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Flow.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Multiline_Input.H>

int main(int argc, char **argv) {
  Fl_Double_Window win(640, 480, "Fl_Flow login demo");

  Fl_Flow flow(0, 0, win.w(), win.h());
  flow.color(FL_WHITE);
  flow.box(FL_FLAT_BOX);
  flow.padding(0);

  Fl_Flow center(0, 0, 600, 300);
  center.box(FL_FLAT_BOX);

  Fl_Box logo(0, 0, 1, 1);
  logo.color(FL_WHITE);
  logo.box(FL_FLAT_BOX);

  Fl_Box    labelPad(0, 0,  85,   1);              // invisible space for labels
  Fl_Box    topPad  (0, 0,   1, 100);              // invisible space above inputs
  Fl_Input  username(0, 0, 175,  30, "Username:"); // input
  Fl_Input  password(0, 0, 175,  30, "Password:"); // input
  Fl_Button login   (0, 0, 100,  30, "Login");     // login button
  center.end();
  flow.end();
  win.end();

  flow.rule(center, "/</^");    // center the main Fl_Flow inside the window
  center.rule(logo, "/<=<=^");  // center hor., expand left, expand up
  center.rule(labelPad, "<=^"); // reserve space for input box labels
  center.rule(topPad, "^=<");   // reserve space above input boxes
  center.rule(username, "^<");  // move username input up and left
  center.rule(password, "^<");  // move password input up and left
  center.rule(login, "v");      // position login button

  win.resizable(flow);
  win.size_range(center.w(), center.h());
  win.show(argc, argv);
  return Fl::run();
}

