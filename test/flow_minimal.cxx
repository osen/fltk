#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Flow.H>

static int maxb = 0;  // max button number
static char buf[20];  // label edit buffer

static const int BW = 100;  // default button width
static const int BH =  30;  // default button height
static const int LS =  14;  // default button labelsize

void resize_cb(Fl_Widget *, void *);

void delete_button(Fl_Widget *w, void *) {
  Fl_Flow *flow = (Fl_Flow *)w->parent();
  delete w;
  resize_cb(flow, 0);
  flow->layout();
  flow->redraw();
}

void add_button(Fl_Flow *flow) {
  Fl_Button *but = new Fl_Button(0, 0, BW, BH);
  sprintf(buf, "Button %d", ++maxb);
  but->copy_label(buf);
  but->callback(delete_button);
  but->tooltip("click to delete this button");
  but->labelsize(LS);
  flow->add(but);
  flow->rule(but, "^<");
}

void add_button_cb(Fl_Widget *w, void *v) {
  Fl_Flow *flow = (Fl_Flow *)v;
  add_button(flow);
  resize_cb(flow, 0);
  flow->layout();
  flow->redraw();
}

// Fl_Flow resize callback demo:
//
// (1) change button width so they (almost) fill the width
// (2) scale button hight proportionally to width
// (3) scale labelsize proportionally

void resize_cb(Fl_Widget *w, void *v) {
  Fl_Flow *flow = (Fl_Flow *)w;
  int ctx = fl_int(v);
  // printf("resize_cb() flow = %p, ctx = %d\n", flow, ctx);
  int nc = flow->children();
  if (!nc) return;
  int pad = 5;
  int wid = flow->w() - Fl::box_dw(flow->box());
  int bpr = (wid - pad - 2) / BW;
  if (bpr > nc)
    bpr = nc;
  int bw = (wid - pad - bpr * pad) / bpr - 2;
  flow->padding(pad);
  float scale = float(bw) / BW;
  int bh = int(scale * BH); // scale buttons proportional
  // printf("resize_cb() wid = %d, bpr = %d, bw => %d, bh => %d\n", wid, bpr, bw, bh);
  for (int i = 0; i < nc; i++) {
    Fl_Widget *c = flow->child(i);
    flow->min_size(c, bw, bh);
    flow->child(i)->labelsize(int(scale * LS));
  }
  flow->redraw();
}

int main() {
  Fl_Double_Window win(640, 480);
  Fl_Flow flow(0, 0, win.w(), win.h() - 60);
  flow.box(FL_FLAT_BOX);
  for (int i = 0; i < 10; i++) {
    add_button(&flow);
  }
  flow.end();
  flow.resize_callback(resize_cb, (void *)4711);
  Fl_Button cr(10, win.h() - 50, win.w() - 20, 40, "Add a button");
  cr.callback(add_button_cb, (void*)(&flow));
  win.end();
  win.resizable(flow);
  win.show();
  return Fl::run();
}
