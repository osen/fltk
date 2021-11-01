//
// Flow header file for the Fast Light Tool Kit (FLTK).
//
// Copyright 2021 by Karsten Pedersen and others.
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file. If this
// file is missing or damaged, see the license at:
//
//     https://www.fltk.org/COPYING.php
//
// Please see the following page on how to report bugs and issues:
//
//     https://www.fltk.org/bugs.php
//

#ifndef FL_FLOW_H
#define FL_FLOW_H

#include <FL/Fl.H>
#include <FL/Fl_Group.H>

struct Fl_Flow : Fl_Group {
  Fl_Flow(int x = 0, int y = 0, int w = 128, int h = 128, const char *label = 0);
  virtual ~Fl_Flow();
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
  struct Fl_Instruction;
  struct Fl_State;

  Fl_Instruction *m_instructions;
  Fl_State *m_states;
  int m_padding;
  bool m_drawn;
  Fl_Callback *m_resize_cb;  // resize callback
  void *m_resize_ctx; // resize calback context (user_data)

  void process();
  void prepare();
};

#endif
