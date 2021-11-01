#ifndef FL_FLOW_H
#define FL_FLOW_H

#include <FL/Fl.H>
#include <FL/Fl_Group.H>

#include <vector>
#include <stdio.h>

struct Fl_Instruction;
struct Fl_State;

struct Fl_Flow : Fl_Group {
  Fl_Flow(int _x = 0, int _y = 0, int _w = 128, int _h = 128, const char *_label = 0);
  ~Fl_Flow();
  void padding(int padding);
  int padding();
  void rule(Fl_Widget &widget, const char *instructions);
  void rule(Fl_Widget *widget, const char *instructions);
  int min_size(Fl_Widget *widget, int w, int h);
  void layout();

  virtual void draw();
  virtual void resize(int x, int y, int w, int h);
  virtual void resize_callback(Fl_Callback *cb, void *ctx);

private:
  std::vector<Fl_Instruction> m_instructions;
  std::vector<Fl_State> m_states;
  int m_padding;
  bool m_drawn;
  Fl_Callback *m_resize_cb;  // resize callback
  void *m_resize_ctx; // resize calback context (user_data)

  void process();
  void prepare();
};

#endif
