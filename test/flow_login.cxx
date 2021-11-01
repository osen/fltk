#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Flow.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Multiline_Input.H>

int main() {
  Fl_Double_Window win(640, 480);

  Fl_Flow flow(0, 0, win.w(), win.h());
  flow.color(FL_WHITE);
  flow.box(FL_FLAT_BOX);

  Fl_Flow center(0, 0, 620, 300);
  win.size_range(center.w(), center.h());
  center.box(FL_FLAT_BOX);

  Fl_Box logo(0, 0, 1, 1);
  logo.color(FL_WHITE);
  logo.box(FL_FLAT_BOX);

  Fl_Box logoPad(0, 0, 45, 1);
  Fl_Button login(0, 0, 100, 30, "Login");
  Fl_Input text(0, 0, 175, 30);
  text.label("Username:");
  Fl_Box sep(0, 0, 10, 1);
  sep.color(FL_BLACK);
  sep.box(FL_FLAT_BOX);

  flow.rule(center, "/</^");
  center.rule(logo, "/<=<=^");
  center.rule(logoPad, "<^");

  center.rule(text, "^/<");
  center.rule(sep, "=<^");
  center.rule(logoPad, "=v");
  center.rule(login, "v");
  center.rule(sep, "v");
  center.rule(text, "/v");

  center.end();
  flow.end();
  win.end();
  win.resizable(flow);
  win.show();

  return Fl::run();
}

