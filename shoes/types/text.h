#include "shoes/ruby.h"
#include "shoes/canvas.h"
#include "shoes/app.h"
#include "shoes/internal.h"
#include "shoes/world.h"
#include "shoes/native/native.h"

#ifndef SHOES_TEXT_TYPE_H
#define SHOES_TEXT_TYPE_H

//#define NEW_MACRO_TEXT

/* extern variables necessary to communicate with other parts of Shoes */
extern VALUE cShoes, cApp, cTypes, cCanvas, cWidget;
extern shoes_app _shoes_app;

SYMBOL_ID(text);

VALUE cTextClass, cSpan, cDel, cStrong, cSub, cSup, cCode, cEm, cIns;

typedef struct {
    VALUE parent;
    VALUE attr;
    VALUE texts;
    char hover;
} shoes_text;

#ifdef NEW_MACRO_TEXT
extern const rb_data_type_t shoes_text_type;
#endif

/* each widget should have its own init function */
void shoes_text_init();

// ruby
void shoes_text_mark(shoes_text *text);
void shoes_text_free(shoes_text *text);
VALUE shoes_text_check(VALUE texts, VALUE parent);
VALUE shoes_text_to_s(VALUE self);
VALUE shoes_text_new(VALUE klass, VALUE texts, VALUE attr);
VALUE shoes_text_alloc(VALUE klass);
VALUE shoes_text_parent(VALUE self);
VALUE shoes_text_children(VALUE self);

// canvas
VALUE shoes_canvas_code(int argc, VALUE *argv, VALUE self);
VALUE shoes_canvas_del(int argc, VALUE *argv, VALUE self);
VALUE shoes_canvas_em(int argc, VALUE *argv, VALUE self);
VALUE shoes_canvas_ins(int argc, VALUE *argv, VALUE self);
VALUE shoes_canvas_span(int argc, VALUE *argv, VALUE self);
VALUE shoes_canvas_strong(int argc, VALUE *argv, VALUE self);
VALUE shoes_canvas_sub(int argc, VALUE *argv, VALUE self);
VALUE shoes_canvas_sup(int argc, VALUE *argv, VALUE self);

#endif
