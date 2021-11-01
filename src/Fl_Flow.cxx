#include <FL/Fl_Flow.H>

#include <vector>
#include <stdio.h>

static void set_position(Fl_Widget *widget, int _x, int _y) {
  Fl_Group *parent = widget->parent();
  int x = 0;
  int y = 0;

  if (parent) {
    x = parent->x();
    y = parent->y();
  }

  widget->position(x + _x, y + _y);
}

struct Fl_Transform {
  Fl_Widget * m_widget;
  int m_padding;
  int m_x;
  int m_y;
  int m_w;
  int m_h;
  int m_cx;
  int m_cy;
  int m_cw;
  int m_ch;

  Fl_Transform(Fl_Widget *widget, int padding)
    : m_widget(widget)
    , m_padding(padding)
    , m_x()
    , m_y()
    , m_w()
    , m_h()
    , m_cx()
    , m_cy()
    , m_cw()
    , m_ch() {
    m_x = widget->x() - m_padding;
    m_y = widget->y() - m_padding;
    m_w = widget->w() + m_padding * 2;
    m_h = widget->h() + m_padding * 2;

    m_cx = m_x;
    m_cy = m_y;
    m_cw = m_w;
    m_ch = m_h;
  }

  bool contains(Fl_Transform &_other) {
    if (_other.m_x < m_x)
      return false;
    if (_other.m_y < m_y)
      return false;
    if (_other.m_x + _other.m_w > m_x + m_w)
      return false;
    if (_other.m_y + _other.m_h > m_y + m_h)
      return false;
    return true;
  }

  bool colliding(Fl_Transform &_other) {
    if (m_x < _other.m_x) {
      if (m_x + m_w < _other.m_x)
        return false;
    } else {
      if (_other.m_x + _other.m_w < m_x)
        return false;
    }

    if (m_y < _other.m_y) {
      if (m_y + m_h < _other.m_y)
        return false;
    } else {
      if (_other.m_y + _other.m_h < m_y)
        return false;
    }

    return true;
  }

  void apply() {
    m_widget->resize(m_cx + m_padding, m_cy + m_padding, m_cw - m_padding * 2, m_ch - m_padding * 2);
  }

  void debug_output() {
    printf("Committed: %i %i %i %i\n", m_cx, m_cy, m_cw, m_ch);
    printf("Staging: %i %i %i %i\n", m_x, m_y, m_w, m_h);
  }

  void commit() {
    m_cx = m_x;
    m_cy = m_y;
    m_cw = m_w;
    m_ch = m_h;
  }

  void rollback() {
    m_x = m_cx;
    m_y = m_cy;
    m_w = m_cw;
    m_h = m_ch;
  }

  void contract(int _w, int _h) {
    m_x += m_w / 2 - _w / 2;
    m_y += m_h / 2 - _h / 2;
    m_w = _w;
    m_h = _h;
  }

  void translate(int _x, int _y) {
    commit();
    m_x += _x;
    m_y += _y;
  }

  void scale(int _x, int _y) {
    commit();
    if (_x < 0) {
      m_x += _x;
      m_w -= _x;
    } else {
      m_w += _x;
    }

    if (_y < 0) {
      m_y += _y;
      m_h -= _y;
    } else {
      m_h += _y;
    }
  }
};

struct Fl_Instruction {
  static const int NONE = 0;
  static const int EXPAND = 50;
  static const int CENTER = 60;

  static const int MOVE_LEFT = 1;
  static const int MOVE_RIGHT = 2;
  static const int MOVE_UP = 3;
  static const int MOVE_DOWN = 4;

  static const int EXPAND_LEFT = 6;
  static const int EXPAND_RIGHT = 7;
  static const int EXPAND_UP = 8;
  static const int EXPAND_DOWN = 9;

  static const int CENTER_LEFT = 11;
  static const int CENTER_RIGHT = 12;
  static const int CENTER_UP = 13;
  static const int CENTER_DOWN = 14;

  static int decode(char c, int _type) {
    if (_type == EXPAND) {
      if (c == '<')
        return EXPAND_LEFT;
      else if (c == '>')
        return EXPAND_RIGHT;
      else if (c == '^')
        return EXPAND_UP;
      else if (c == 'v')
        return EXPAND_DOWN;
    } else if (_type == CENTER) {
      if (c == '<')
        return CENTER_LEFT;
      else if (c == '>')
        return CENTER_RIGHT;
      else if (c == '^')
        return CENTER_UP;
      else if (c == 'v')
        return CENTER_DOWN;
    } else if (_type == NONE) {
      if (c == '<')
        return MOVE_LEFT;
      else if (c == '>')
        return MOVE_RIGHT;
      else if (c == '^')
        return MOVE_UP;
      else if (c == 'v')
        return MOVE_DOWN;
    }

    return 0;
  }

  Fl_Instruction()
    : m_widget(0)
    , m_instruction() {}

  int x_direction() {
    if (m_instruction == MOVE_LEFT || m_instruction == EXPAND_LEFT || m_instruction == CENTER_LEFT) {
      return -1;
    } else if (m_instruction == MOVE_RIGHT || m_instruction == EXPAND_RIGHT || m_instruction == CENTER_RIGHT) {
      return 1;
    }

    return 0;
  }

  int y_direction() {
    if (m_instruction == MOVE_UP || m_instruction == EXPAND_UP || m_instruction == CENTER_UP) {
      return -1;
    } else if (m_instruction == MOVE_DOWN || m_instruction == EXPAND_DOWN || m_instruction == CENTER_DOWN) {
      return 1;
    }

    return 0;
  }

  Fl_Widget *m_widget;
  int m_instruction;
};

struct Fl_State {
  Fl_State()
    : m_widget(0)
    , m_w()
    , m_h()
    , m_placed() {}

  Fl_Widget * m_widget;
  int m_w;
  int m_h;
  bool m_placed;
};

Fl_Flow::Fl_Flow(int _x, int _y, int _w, int _h, const char *_label)
  : Fl_Group(_x, _y, _w, _h, _label)
  , m_padding(5)
  , m_drawn(0)
  , m_resize_cb(0)
  , m_resize_ctx(0) {
  resizable(NULL);
}

/*
 * TODO: Allow vector<T> replacement to work with forward declares
 */
Fl_Flow::~Fl_Flow() { }

// set padding (FLTK style: w/o prefix "set_")
void Fl_Flow::padding(int padding) {
  m_padding = padding;
  // DO NOT call resize() here to allow the user to call resize
  // in their resize callback (otherwise we'd have a recursion)
  m_drawn = false; // force re-layout before drawing
}

// get current padding
int Fl_Flow::padding() {
  return m_padding;
}

void Fl_Flow::rule(Fl_Widget &widget, const char *instructions) {
  rule(&widget, instructions);
}

void Fl_Flow::rule(Fl_Widget *widget, const char *instructions) {
  int type = Fl_Instruction::NONE;

  // we should assume that the widget has already been add()ed to the group
  // DO NOT:  add(widget);

  // ... but we should verify that the widget is one of our children:
  if (children() == 0 || find(widget) == children())
    return;

  size_t len = strlen(instructions);
  for (size_t ci = 0; ci < len; ++ci) {
    char c = instructions[ci];

    if (c == '=') {
      type = Fl_Instruction::EXPAND;
      continue;
    } else if (c == '/') {
      type = Fl_Instruction::CENTER;
      continue;
    }

    Fl_Instruction instruction;
    instruction.m_widget = widget;
    instruction.m_instruction = Fl_Instruction::decode(c, type);

    if(instruction.m_instruction)
    {
      m_instructions.push_back(instruction);
    }
    else
    {
      fprintf(stderr, "Invalid instruction: '%c' '%d'\n", c, type);
    }

    type = Fl_Instruction::NONE;
  }

  m_drawn = false; // force re-layout before drawing
}

/*
 * Change the (minimal) size of a widget.
 * Returns the number of status changes or 0 if widget not found.
 * TODO: Widget in state may be dangling at this point. Call prepare()
 */
int Fl_Flow::min_size(Fl_Widget *widget, int w, int h) {
  int ret = 0;
  for (size_t si = 0; si < m_states.size(); ++si) {
    Fl_State &s = m_states.at(si);
    if (s.m_widget != widget)
      continue;
    s.m_w = w;
    s.m_h = h;
    ret++;
  }

  m_drawn = false; // force re-layout before drawing

  return ret;
}

// Force re-layout and redraw()
void Fl_Flow::layout() {
  m_drawn = false; // force re-layout before drawing
  redraw();
}

/*
 * Ensure that widget layout has occurred at least once
 * before initial draw
 */
void Fl_Flow::draw() {
  if (!m_drawn) {
    m_drawn = true;
    prepare();
    process();
  }

  Fl_Group::draw();
}

void Fl_Flow::resize(int x, int y, int w, int h) {
  Fl_Widget::resize(x, y, w, h); // resize the Flow widget first for the callback

  if (m_resize_cb) {                 // call resize callback if defined
    m_resize_cb(this, m_resize_ctx);
  }

  prepare();
  process();
}

/*
 * TODO: Double check what the point of this is. resize() is virtual and base
 * should be called anyway.
 */
void Fl_Flow::resize_callback(Fl_Callback *cb, void *ctx) {
  if (cb) {
    m_resize_cb = cb;
    m_resize_ctx = ctx;
  } else {
    m_resize_cb = (Fl_Callback *)0;
    m_resize_ctx = (void *)0;
  }
}

void Fl_Flow::process() {
  Fl_Transform pt(this, 0);

  for (size_t ii = 0; ii < m_instructions.size(); ++ii) {
    Fl_Instruction &i = m_instructions.at(ii);

    if (i.m_instruction == Fl_Instruction::MOVE_LEFT || i.m_instruction == Fl_Instruction::MOVE_RIGHT ||
        i.m_instruction == Fl_Instruction::MOVE_UP || i.m_instruction == Fl_Instruction::MOVE_DOWN ||
        i.m_instruction == Fl_Instruction::EXPAND_LEFT || i.m_instruction == Fl_Instruction::EXPAND_RIGHT ||
        i.m_instruction == Fl_Instruction::EXPAND_UP || i.m_instruction == Fl_Instruction::EXPAND_DOWN ||
        i.m_instruction == Fl_Instruction::CENTER_LEFT || i.m_instruction == Fl_Instruction::CENTER_RIGHT ||
        i.m_instruction == Fl_Instruction::CENTER_UP || i.m_instruction == Fl_Instruction::CENTER_DOWN) {
      int xDir = i.x_direction();
      int yDir = i.y_direction();

      Fl_Transform wt(i.m_widget, m_padding);

      int origWidth = wt.m_w;
      int origHeight = wt.m_h;

      while (true) {
        if (i.m_instruction == Fl_Instruction::MOVE_LEFT || i.m_instruction == Fl_Instruction::MOVE_RIGHT ||
            i.m_instruction == Fl_Instruction::MOVE_UP || i.m_instruction == Fl_Instruction::MOVE_DOWN) {
          wt.translate(xDir, yDir);
        } else if (i.m_instruction == Fl_Instruction::EXPAND_LEFT ||
                   i.m_instruction == Fl_Instruction::EXPAND_RIGHT || i.m_instruction == Fl_Instruction::EXPAND_UP ||
                   i.m_instruction == Fl_Instruction::EXPAND_DOWN || i.m_instruction == Fl_Instruction::CENTER_LEFT ||
                   i.m_instruction == Fl_Instruction::CENTER_RIGHT || i.m_instruction == Fl_Instruction::CENTER_UP ||
                   i.m_instruction == Fl_Instruction::CENTER_DOWN) {
          wt.scale(xDir, yDir);
        } else {
          fprintf(stderr, "Invalid instruction: '%d'\n", i.m_instruction);
        }

        /*
         * Collide with parent bounds
         */
        if (!pt.contains(wt)) {
          break;
        }

        bool colliding = false;

        /*
         * Collide with *positioned* siblings
         */
        for (size_t si = 0; si < m_states.size(); ++si) {
          Fl_State &s = m_states.at(si);
          if (!s.m_placed)
            continue;
          if (s.m_widget == i.m_widget)
            continue;

          Fl_Transform st(s.m_widget, 0);

          if (wt.colliding(st)) {
            colliding = true;
            break;
          }
        }

        if (colliding)
          break;
      }

      /*
       * Transformed *just* too far, so rollback.
       * TODO: Rollback just after collision check
       */
      wt.rollback();
      // wt.debug_output();

      if (i.m_instruction == Fl_Instruction::CENTER_LEFT || i.m_instruction == Fl_Instruction::CENTER_RIGHT ||
          i.m_instruction == Fl_Instruction::CENTER_UP || i.m_instruction == Fl_Instruction::CENTER_DOWN) {
        wt.contract(origWidth, origHeight);
        wt.commit();
      }

      wt.apply();
    }

    /*
     * Flag widget as placed.
     */
    for (size_t si = 0; si < m_states.size(); ++si) {
      Fl_State &s = m_states.at(si);
      if (s.m_widget != i.m_widget)
        continue;
      s.m_placed = true;
      break;
    }
  }
}

void Fl_Flow::prepare() {
  /*
   * Remove any states with invalid children
   */
  for (size_t si = 0; si < m_states.size(); ++si) {
    if (find(m_states.at(si).m_widget) == children()) { // not found
      m_states.erase(m_states.begin() + si);
      --si;
      continue;
    }

    bool found = false;

    for (int ci = 0; ci < children(); ++ci) {
      if (child(ci) == m_states.at(si).m_widget) {
        found = true;
        break;
      }
    }

    if (!found) {
      m_states.erase(m_states.begin() + si);
      --si;
      continue;
    }
  }

  /*
   * Remove any instructions with invalid children
   */
  for (size_t ii = 0; ii < m_instructions.size(); ++ii) {
    if (find(m_instructions.at(ii).m_widget) == children()) { // not found
      m_instructions.erase(m_instructions.begin() + ii);
      --ii;
      continue;
    }

    bool found = false;
    for (int ci = 0; ci < children(); ++ci) {
      if (child(ci) == m_instructions.at(ii).m_widget) {
        found = true;
        break;
      }
    }

    if (!found) {
      m_instructions.erase(m_instructions.begin() + ii);
      --ii;
      continue;
    }
  }

  /*
   * Add any missing children into the states
   */
  for (int ci = 0; ci < children(); ++ci) {
    bool found = false;
    for (size_t si = 0; si < m_states.size(); ++si) {
      if (child(ci) == m_states.at(si).m_widget) {
        found = true;
        break;
      }
    }

    if (found == false) {
      Fl_State s;
      s.m_widget = child(ci);
      s.m_w = child(ci)->w();
      s.m_h = child(ci)->h();
      m_states.push_back(s);
    }
  }

  /*
   * Reset state for the children
   */
  for (size_t si = 0; si < m_states.size(); ++si) {
    m_states.at(si).m_placed = false;
    Fl_Widget *wid = m_states.at(si).m_widget;
    wid->size(m_states.at(si).m_w, m_states.at(si).m_h);
    set_position(wid, w() - wid->w() - m_padding, h() - wid->h() - m_padding);
  }
}

