#include <FL/Fl_Flow.H>

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
  Fl_Widget *m_widget;
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
    , m_next(0)
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

  Fl_Instruction *m_next;
  Fl_Widget *m_widget;
  int m_instruction;
};

struct Fl_State {
  Fl_State()
    : m_widget(0)
    , m_next(0)
    , m_w()
    , m_h()
    , m_placed() {}

  Fl_Widget *m_widget;
  int m_w;
  int m_h;
  bool m_placed;
  Fl_State *m_next;
};

Fl_Flow::Fl_Flow(int _x, int _y, int _w, int _h, const char *_label)
  : Fl_Group(_x, _y, _w, _h, _label)
  , m_instructions(0)
  , m_padding(5)
  , m_drawn(0)
  , m_resize_cb(0)
  , m_resize_ctx(0) {
  resizable(NULL);
}

/*
 * TODO: Allow vector<T> replacement to work with forward declares
 */
Fl_Flow::~Fl_Flow()
{
  Fl_Instruction *curr = m_instructions;

  while (curr)
  {
    Fl_Instruction *next = curr->m_next;
    delete curr;
    curr = next;
  }
}

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

    if (instruction.m_instruction)
    {
      Fl_Instruction *curr = m_instructions;

      while (curr)
      {
        Fl_Instruction *next = curr->m_next;

        if (!next)
          break;

        curr = next;
      }

      if (curr)
        curr->m_next = new Fl_Instruction(instruction);
      else
        m_instructions = new Fl_Instruction(instruction);
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

  Fl_State *curr = m_states;

  while (curr) {
    if (curr->m_widget == widget) {
      curr->m_w = w;
      curr->m_h = h;
      ret++;
    }
    curr = curr->m_next;
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
  Fl_Instruction *ci = m_instructions;

  while (ci)
  {
    if (ci->m_instruction == Fl_Instruction::MOVE_LEFT || ci->m_instruction == Fl_Instruction::MOVE_RIGHT ||
        ci->m_instruction == Fl_Instruction::MOVE_UP || ci->m_instruction == Fl_Instruction::MOVE_DOWN ||
        ci->m_instruction == Fl_Instruction::EXPAND_LEFT || ci->m_instruction == Fl_Instruction::EXPAND_RIGHT ||
        ci->m_instruction == Fl_Instruction::EXPAND_UP || ci->m_instruction == Fl_Instruction::EXPAND_DOWN ||
        ci->m_instruction == Fl_Instruction::CENTER_LEFT || ci->m_instruction == Fl_Instruction::CENTER_RIGHT ||
        ci->m_instruction == Fl_Instruction::CENTER_UP || ci->m_instruction == Fl_Instruction::CENTER_DOWN) {
      int xDir = ci->x_direction();
      int yDir = ci->y_direction();

      Fl_Transform wt(ci->m_widget, m_padding);

      int origWidth = wt.m_w;
      int origHeight = wt.m_h;

      while (true) {
        if (ci->m_instruction == Fl_Instruction::MOVE_LEFT || ci->m_instruction == Fl_Instruction::MOVE_RIGHT ||
            ci->m_instruction == Fl_Instruction::MOVE_UP || ci->m_instruction == Fl_Instruction::MOVE_DOWN) {
          wt.translate(xDir, yDir);
        } else if (ci->m_instruction == Fl_Instruction::EXPAND_LEFT ||
                   ci->m_instruction == Fl_Instruction::EXPAND_RIGHT ||
                   ci->m_instruction == Fl_Instruction::EXPAND_UP ||
                   ci->m_instruction == Fl_Instruction::EXPAND_DOWN ||
                   ci->m_instruction == Fl_Instruction::CENTER_LEFT ||
                   ci->m_instruction == Fl_Instruction::CENTER_RIGHT ||
                   ci->m_instruction == Fl_Instruction::CENTER_UP ||
                   ci->m_instruction == Fl_Instruction::CENTER_DOWN) {
          wt.scale(xDir, yDir);
        } else {
          fprintf(stderr, "Invalid instruction: '%d'\n", ci->m_instruction);
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
        Fl_State *cs = m_states;

        while (cs) {
          Fl_State *s = cs;
          cs = cs->m_next;

          if (!s->m_placed)
            continue;
          if (s->m_widget == ci->m_widget)
            continue;

          Fl_Transform st(s->m_widget, 0);

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

      if (ci->m_instruction == Fl_Instruction::CENTER_LEFT ||
          ci->m_instruction == Fl_Instruction::CENTER_RIGHT ||
          ci->m_instruction == Fl_Instruction::CENTER_UP ||
          ci->m_instruction == Fl_Instruction::CENTER_DOWN) {
        wt.contract(origWidth, origHeight);
        wt.commit();
      }

      wt.apply();
    }

    /*
     * Flag widget as placed.
     */
    Fl_State *cs = m_states;

    while (cs) {
      if (cs->m_widget == ci->m_widget) {
        cs->m_placed = true;
        break;
      }

      cs = cs->m_next;
    }

    ci = ci->m_next;
  }
}

void Fl_Flow::prepare() {
  /*
   * Remove any states with invalid children
   */
  Fl_State *cs = m_states;
  Fl_State *ps = 0;
  while (cs) {
    Fl_State *next = cs->m_next;

    if (find(cs->m_widget) == children()) {
      if (ps)
        ps->m_next = next;
      else
        m_states = next;

      delete cs;
    } else {
      ps = cs;
    }

    cs = next;
  }

  /*
   * Remove any instructions with invalid children
   */
  Fl_Instruction *ci = m_instructions;
  Fl_Instruction *pi = 0;
  while (ci)
  {
    Fl_Instruction *next = ci->m_next;

    if (find(ci->m_widget) == children()) { // not found
      if (pi)
        pi->m_next = next;
      else
        m_instructions = next;

      delete ci;
    } else {
      pi = ci;
    }

    ci = next;
  }

  /*
   * Add any missing children into the states
   */
  for (int ci = 0; ci < children(); ++ci) {
    bool found = false;
    Fl_State *cs = m_states;
    while (cs) {
      if (child(ci) == cs->m_widget) {
        found = true;
        break;
      }
      cs = cs->m_next;
    }

    if (found == false) {
      Fl_State s;
      s.m_widget = child(ci);
      s.m_w = child(ci)->w();
      s.m_h = child(ci)->h();
      s.m_next = m_states;
      m_states = new Fl_State(s);
    }
  }

  /*
   * Reset state for the children
   */
  cs = m_states;
  while (cs) {
    cs->m_placed = false;
    Fl_Widget *wid = cs->m_widget;
    wid->size(cs->m_w, cs->m_h);
    set_position(wid, w() - wid->w() - m_padding, h() - wid->h() - m_padding);
    cs = cs->m_next;
  }
}

