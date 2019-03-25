#include "shoes/app.h"
#include "shoes/types/native.h"
#include "shoes/types/color.h"
#include "shoes/types/pattern.h"
#include "shoes/types/shape.h"
#include "shoes/types/image.h"
#include "shoes/app.h"

// ruby
VALUE cImage;
#ifdef NEW_MACRO_APP
FUNC_T("+image", image, -1);
FUNC_T(".imagesize", imagesize, 1);
FUNC_T(".nostroke", nostroke, 0);
FUNC_T(".stroke", stroke, -1);
FUNC_T(".strokewidth", strokewidth, 1);
FUNC_T(".dash", dash, 1);
FUNC_T(".cap", cap, 1);
FUNC_T(".nofill", nofill, 0);
FUNC_T(".fill", fill, -1);
FUNC_T("+arc", arc, -1);
FUNC_T("+rect", rect, -1);
FUNC_T("+oval", oval, -1);
FUNC_T("+line", line, -1);
FUNC_T("+arrow", arrow, -1);
FUNC_T("+star", star, -1);
FUNC_T(".blur", blur, -1);
FUNC_T(".glow", glow, -1);
FUNC_T(".shadow", shadow, -1);
FUNC_T(".move_to", move_to, 2);
FUNC_T(".line_to", line_to, 2);
FUNC_T(".curve_to", curve_to, 6);
FUNC_T(".arc_to", arc_to, 6);
FUNC_T(".transform", transform, 1);
FUNC_T(".translate", translate, 2);
FUNC_T(".rotate", rotate, 1);
FUNC_T(".scale", scale, -1);
FUNC_T(".skew", skew, -1);
#else
FUNC_M("+image", image, -1);
FUNC_M(".imagesize", imagesize, 1);
FUNC_M(".nostroke", nostroke, 0);
FUNC_M(".stroke", stroke, -1);
FUNC_M(".strokewidth", strokewidth, 1);
FUNC_M(".dash", dash, 1);
FUNC_M(".cap", cap, 1);
FUNC_M(".nofill", nofill, 0);
FUNC_M(".fill", fill, -1);
FUNC_M("+arc", arc, -1);
FUNC_M("+rect", rect, -1);
FUNC_M("+oval", oval, -1);
FUNC_M("+line", line, -1);
FUNC_M("+arrow", arrow, -1);
FUNC_M("+star", star, -1);
FUNC_M(".blur", blur, -1);
FUNC_M(".glow", glow, -1);
FUNC_M(".shadow", shadow, -1);
FUNC_M(".move_to", move_to, 2);
FUNC_M(".line_to", line_to, 2);
FUNC_M(".curve_to", curve_to, 6);
FUNC_M(".arc_to", arc_to, 6);
FUNC_M(".transform", transform, 1);
FUNC_M(".translate", translate, 2);
FUNC_M(".rotate", rotate, 1);
FUNC_M(".scale", scale, -1);
FUNC_M(".skew", skew, -1);
#endif

PLACE_COMMON(image);
CLASS_COMMON2(image);
TRANS_COMMON(image, 1);

void shoes_image_init() {
#ifdef NEW_MACRO_IMAGE
    cImage    = rb_define_class_under(cTypes, "Image", rb_cData);
#else
    cImage    = rb_define_class_under(cTypes, "Image", rb_cObject);
    rb_define_alloc_func(cImage, shoes_image_alloc);
#endif
    rb_define_method(cImage, "[]", CASTHOOK(shoes_image_get_pixel), 2);
    rb_define_method(cImage, "[]=", CASTHOOK(shoes_image_set_pixel), 3);
    rb_define_method(cImage, "nostroke", CASTHOOK(shoes_canvas_nostroke), 0);
    rb_define_method(cImage, "stroke", CASTHOOK(shoes_canvas_stroke), -1);
    rb_define_method(cImage, "strokewidth", CASTHOOK(shoes_canvas_strokewidth), 1);
    rb_define_method(cImage, "dash", CASTHOOK(shoes_canvas_dash), 1);
    rb_define_method(cImage, "cap", CASTHOOK(shoes_canvas_cap), 1);
    rb_define_method(cImage, "nofill", CASTHOOK(shoes_canvas_nofill), 0);
    rb_define_method(cImage, "fill", CASTHOOK(shoes_canvas_fill), -1);
    rb_define_method(cImage, "arrow", CASTHOOK(shoes_canvas_arrow), -1);
    rb_define_method(cImage, "line", CASTHOOK(shoes_canvas_line), -1);
    rb_define_method(cImage, "arc", CASTHOOK(shoes_canvas_arc), -1);
    rb_define_method(cImage, "oval", CASTHOOK(shoes_canvas_oval), -1);
    rb_define_method(cImage, "rect", CASTHOOK(shoes_canvas_rect), -1);
    rb_define_method(cImage, "shape", CASTHOOK(shoes_canvas_shape), -1);
    rb_define_method(cImage, "star", CASTHOOK(shoes_canvas_star), -1);
    rb_define_method(cImage, "move_to", CASTHOOK(shoes_canvas_move_to), 2);
    rb_define_method(cImage, "line_to", CASTHOOK(shoes_canvas_line_to), 2);
    rb_define_method(cImage, "curve_to", CASTHOOK(shoes_canvas_curve_to), 6);
    rb_define_method(cImage, "arc_to", CASTHOOK(shoes_canvas_arc_to), 6);
    rb_define_method(cImage, "blur", CASTHOOK(shoes_canvas_blur), -1);
    rb_define_method(cImage, "glow", CASTHOOK(shoes_canvas_glow), -1);
    rb_define_method(cImage, "shadow", CASTHOOK(shoes_canvas_shadow), -1);
    rb_define_method(cImage, "image", CASTHOOK(shoes_canvas_image), -1);
    rb_define_method(cImage, "path", CASTHOOK(shoes_image_get_path), 0);
    rb_define_method(cImage, "path=", CASTHOOK(shoes_image_set_path), 1);
    rb_define_method(cImage, "app", CASTHOOK(shoes_canvas_get_app), 0);
    rb_define_method(cImage, "displace", CASTHOOK(shoes_image_displace), 2);
    rb_define_method(cImage, "draw", CASTHOOK(shoes_image_draw), 2);
    rb_define_method(cImage, "size", CASTHOOK(shoes_image_size), 0);
    rb_define_method(cImage, "move", CASTHOOK(shoes_image_move), 2);
    rb_define_method(cImage, "parent", CASTHOOK(shoes_image_get_parent), 0);
    rb_define_method(cImage, "rotate", CASTHOOK(shoes_image_rotate), 1);
    rb_define_method(cImage, "scale", CASTHOOK(shoes_image_scale), -1);
    rb_define_method(cImage, "skew", CASTHOOK(shoes_image_skew), -1);
    rb_define_method(cImage, "transform", CASTHOOK(shoes_image_transform), 1);
    rb_define_method(cImage, "translate", CASTHOOK(shoes_image_translate), 2);
    rb_define_method(cImage, "top", CASTHOOK(shoes_image_get_top), 0);
    rb_define_method(cImage, "left", CASTHOOK(shoes_image_get_left), 0);
    rb_define_method(cImage, "width", CASTHOOK(shoes_image_get_width), 0);
    rb_define_method(cImage, "height", CASTHOOK(shoes_image_get_height), 0);
    rb_define_method(cImage, "full_width", CASTHOOK(shoes_image_get_full_width), 0);
    rb_define_method(cImage, "full_height", CASTHOOK(shoes_image_get_full_height), 0);
    rb_define_method(cImage, "remove", CASTHOOK(shoes_basic_remove), 0);
    rb_define_method(cImage, "style", CASTHOOK(shoes_image_style), -1);
    rb_define_method(cImage, "hide", CASTHOOK(shoes_image_hide), 0);
    rb_define_method(cImage, "show", CASTHOOK(shoes_image_show), 0);
    rb_define_method(cImage, "toggle", CASTHOOK(shoes_image_toggle), 0);
    rb_define_method(cImage, "click", CASTHOOK(shoes_image_click), -1);
    rb_define_method(cImage, "release", CASTHOOK(shoes_image_release), -1);
    rb_define_method(cImage, "hover", CASTHOOK(shoes_image_hover), -1);
    rb_define_method(cImage, "leave", CASTHOOK(shoes_image_leave), -1);

    RUBY_M("+image", image, -1);
    RUBY_M(".imagesize", imagesize, 1);

    RUBY_M(".nostroke", nostroke, 0);
    RUBY_M(".stroke", stroke, -1);
    RUBY_M(".strokewidth", strokewidth, 1);
    RUBY_M(".dash", dash, 1);
    RUBY_M(".cap", cap, 1);
    RUBY_M(".nofill", nofill, 0);
    RUBY_M(".fill", fill, -1);
    RUBY_M("+arc", arc, -1);
    RUBY_M("+rect", rect, -1);
    RUBY_M("+oval", oval, -1);
    RUBY_M("+line", line, -1);
    RUBY_M("+arrow", arrow, -1);
    RUBY_M("+star", star, -1);
    RUBY_M(".blur", blur, -1);
    RUBY_M(".glow", glow, -1);
    RUBY_M(".shadow", shadow, -1);
    RUBY_M(".move_to", move_to, 2);
    RUBY_M(".line_to", line_to, 2);
    RUBY_M(".curve_to", curve_to, 6);
    RUBY_M(".arc_to", arc_to, 6);
    RUBY_M(".transform", transform, 1);
    RUBY_M(".translate", translate, 2);
    RUBY_M(".rotate", rotate, 1);
    RUBY_M(".scale", scale, -1);
    RUBY_M(".skew", skew, -1);
}

// ruby
void shoes_image_mark(shoes_image *image) {
    rb_gc_mark_maybe(image->path);
    rb_gc_mark_maybe(image->parent);
    rb_gc_mark_maybe(image->attr);
}

void shoes_image_free(shoes_image *image) {
    if (image->type == SHOES_CACHE_MEM) {
        if (image->cr != NULL)     cairo_destroy(image->cr);
        if (image->cached != NULL) cairo_surface_destroy(image->cached->surface);
        SHOE_FREE(image->cached);
    }
    shoes_transform_release(image->st);
    RUBY_CRITICAL(SHOE_FREE(image));
}

#ifdef NEW_MACRO_IMAGE
// creates struct shoes_app_type
TypedData_Type_New(shoes_image);
#endif

VALUE shoes_image_alloc(VALUE klass) {
    VALUE obj;
    shoes_image *image = SHOE_ALLOC(shoes_image);
    SHOE_MEMZERO(image, shoes_image, 1);
#ifdef NEW_MACRO_IMAGE
    obj = TypedData_Wrap_Struct(klass, &shoes_image_type, image);
#else
    obj = Data_Wrap_Struct(klass, shoes_image_mark, shoes_image_free, image);
#endif
    image->path = Qnil;
    image->st = NULL;
    image->attr = Qnil;
    image->parent = Qnil;
    image->type = SHOES_CACHE_MEM;
    return obj;
}


// TODO: clean up after macro transition
VALUE shoes_image_new(VALUE klass, VALUE path, VALUE attr, VALUE parent, shoes_transform *st) {
    VALUE obj = Qnil;
    //shoes_basic *basic;
    //Data_Get_Struct(parent, shoes_basic, basic);
    SETUP_BASIC_T(parent);

    obj = shoes_image_alloc(klass);
#ifdef NEW_MACRO_IMAGE
    Get_TypedStruct2(obj, shoes_image, image);
#else
    shoes_image *image;
    Data_Get_Struct(obj, shoes_image, image);
#endif
    image->path = Qnil;
    image->st = shoes_transform_touch(st);
    image->attr = attr;
    image->parent = shoes_find_canvas(parent);
    COPY_PENS(image->attr, basic->attr);
    int saved_cache_setting = shoes_cache_setting;
    VALUE cache_opt = shoes_cache_setting ? Qtrue : Qfalse;
    VALUE vcache = shoes_hash_get(attr,rb_intern("cache"));
    if (! NIL_P(vcache)) {
      cache_opt = vcache;
    }

    if (rb_obj_is_kind_of(path, cImage)) {
#ifdef NEW_MACRO_IMAGE
        Get_TypedStruct2(path, shoes_image, image2);
#else
        shoes_image *image2;
        Data_Get_Struct(path, shoes_image, image2);
#endif
        image->cached = image2->cached;
        image->type = SHOES_CACHE_ALIAS;
    } else if (!NIL_P(path)) {
        path = shoes_native_to_s(path);
        image->path = path;
        image->cached = shoes_load_image(image->parent, path, cache_opt);
        image->type = SHOES_CACHE_FILE;
    } else {
        shoes_canvas *canvas;
        Data_Get_Struct(image->parent, shoes_canvas, canvas);
        int w = ATTR2(int, attr, width, canvas->width);
        int h = ATTR2(int, attr, height, canvas->height);
        VALUE block = ATTR(attr, draw);
        image->cached = shoes_cached_image_new(w, h, NULL);
        image->cr = cairo_create(image->cached->surface);
        image->type = SHOES_CACHE_MEM;
        if (!NIL_P(block)) DRAW(obj, canvas->app, rb_funcall(block, s_call, 0));
    }
    shoes_cache_setting = saved_cache_setting;
    return obj;
}

VALUE shoes_image_get_full_width(VALUE self) {
#ifdef NEW_MACRO_IMAGE
    Get_TypedStruct2(self, shoes_image, image);
#else
    shoes_image *image;
    Data_Get_Struct(self, shoes_image, image);
#endif
    return INT2NUM(image->cached->width);
}

VALUE shoes_image_get_full_height(VALUE self) {
#ifdef NEW_MACRO_IMAGE
    Get_TypedStruct2(self, shoes_image, image);
#else
    shoes_image *image;
    Data_Get_Struct(self, shoes_image, image);
#endif
    return INT2NUM(image->cached->height);
}

void shoes_image_ensure_dup(shoes_image *image) {
    if (image->type == SHOES_CACHE_MEM)
        return;
    shoes_cached_image *cached = shoes_cached_image_new(image->cached->width, image->cached->height, NULL);
    image->cr = cairo_create(cached->surface);
    cairo_set_source_surface(image->cr, image->cached->surface, 0, 0);
    cairo_paint(image->cr);

    image->cached = cached;
    image->type = SHOES_CACHE_MEM;
}

unsigned char *shoes_image_surface_get_pixel(shoes_cached_image *cached, int x, int y) {
    if (x >= 0 && y >= 0 && x < cached->width && y < cached->height) {
        unsigned char* pixels = cairo_image_surface_get_data(cached->surface);
        if (cairo_image_surface_get_format(cached->surface) == CAIRO_FORMAT_ARGB32)
            return pixels + (y * (4 * cached->width)) + (4 * x);
    }
    return NULL;
}

VALUE shoes_image_get_pixel(VALUE self, VALUE _x, VALUE _y) {
    VALUE color = Qnil;
    int x = NUM2INT(_x), y = NUM2INT(_y);
#ifdef NEW_MACRO_IMAGE
    Get_TypedStruct2(self, shoes_image, image);
#else
    shoes_image *image;
    Data_Get_Struct(self, shoes_image, image);
#endif
    unsigned char *pixels = shoes_image_surface_get_pixel(image->cached, x, y);
    if (pixels != NULL)
        color = shoes_color_new(pixels[2], pixels[1], pixels[0], pixels[3]);
    return color;
}

VALUE shoes_image_set_pixel(VALUE self, VALUE _x, VALUE _y, VALUE col) {
    int x = NUM2INT(_x), y = NUM2INT(_y);
#ifdef NEW_MACRO_IMAGE
    Get_TypedStruct2(self, shoes_image, image);
#else
    shoes_image *image;
    Data_Get_Struct(self, shoes_image, image);
#endif
    shoes_image_ensure_dup(image);
    unsigned char *pixels = shoes_image_surface_get_pixel(image->cached, x, y);
    if (pixels != NULL) {
        if (TYPE(col) == T_STRING)
            col = shoes_color_parse(cColor, col);
        if (rb_obj_is_kind_of(col, cColor)) {
#ifdef NEW_MACRO_COLOR
            Get_TypedStruct2(col, shoes_color, color);
#else
            shoes_color *color;
            Data_Get_Struct(col, shoes_color, color);
#endif
            pixels[0] = color->b;
            pixels[1] = color->g;
            pixels[2] = color->r;
            pixels[3] = color->a;
        }
    }
    return self;
}

VALUE shoes_image_get_path(VALUE self) {
#ifdef NEW_MACRO_IMAGE
    Get_TypedStruct2(self, shoes_image, image);
#else
    shoes_image *image;
    Data_Get_Struct(self, shoes_image, image);
#endif
    return image->path;
}

VALUE shoes_image_set_path(VALUE self, VALUE path) {
#ifdef NEW_MACRO_IMAGE
    Get_TypedStruct2(self, shoes_image, image);
#else
    shoes_image *image;
    Data_Get_Struct(self, shoes_image, image);
#endif
    image->path = path;
    image->cached = shoes_load_image(image->parent, path, Qfalse);
    image->type = SHOES_CACHE_FILE;
    shoes_canvas_repaint_all(image->parent);
    return path;
}

static void shoes_image_draw_surface(cairo_t *cr, shoes_image *self_t, shoes_place *place, cairo_surface_t *surf, int imw, int imh) {
    shoes_apply_transformation(cr, self_t->st, place, 0);
    cairo_translate(cr, place->ix + place->dx, place->iy + place->dy);
    if (place->iw != imw || place->ih != imh)
        cairo_scale(cr, (place->iw * 1.) / imw, (place->ih * 1.) / imh);
    cairo_set_source_surface(cr, surf, 0., 0.);
    cairo_paint(cr);
    shoes_undo_transformation(cr, self_t->st, place, 0);
    self_t->place = *place;
}
// TODO useless macro - only used once. clean up after transition
#ifdef NEW_MACRO_IMAGE
#define SHOES_IMAGE_PLACE(type, imw, imh, surf) \
  SETUP_DRAWING_T(shoes_##type, (REL_CANVAS | REL_SCALE), imw, imh); \
  VALUE ck = rb_obj_class(c); \
  if (RTEST(actual)) \
    shoes_image_draw_surface(CCR(canvas), self_t, &place, surf, imw, imh); \
  FINISH(); \
  return self;
#else
#define SHOES_IMAGE_PLACE(type, imw, imh, surf) \
  SETUP_DRAWING(shoes_##type, (REL_CANVAS | REL_SCALE), imw, imh); \
  VALUE ck = rb_obj_class(c); \
  if (RTEST(actual)) \
    shoes_image_draw_surface(CCR(canvas), self_t, &place, surf, imw, imh); \
  FINISH(); \
  return self;
#endif

VALUE shoes_image_draw(VALUE self, VALUE c, VALUE actual) {
    SHOES_IMAGE_PLACE(image, self_t->cached->width, self_t->cached->height, self_t->cached->surface);
}

void shoes_image_image(VALUE parent, VALUE path, VALUE attr) {
    shoes_place place;
#ifdef NEW_MACRO_IMAGE
    Get_TypedStruct2(parent, shoes_image, pi);
#else
    shoes_image *pi;
    Data_Get_Struct(parent, shoes_image, pi);
#endif
    VALUE self = shoes_image_new(cImage, path, attr, parent, pi->st);
#ifdef NEW_MACRO_IMAGE
    Get_TypedStruct2(self, shoes_image, image);
#else
    shoes_image *image;
    Data_Get_Struct(self, shoes_image, image);
#endif
    shoes_image_ensure_dup(pi);
    shoes_place_exact(&place, image->attr, 0, 0);
    if (place.iw < 1) place.w = place.iw = image->cached->width;
    if (place.ih < 1) place.h = place.ih = image->cached->height;
    shoes_image_draw_surface(pi->cr, image, &place, image->cached->surface, image->cached->width, image->cached->height);
}

VALUE shoes_image_size(VALUE self) {
#ifdef NEW_MACRO_IMAGE
    Get_TypedStruct2(self, shoes_image, self_t);
#else
    shoes_image *self_t;
    Data_Get_Struct(self, shoes_image, self_t);
#endif
    return rb_ary_new3(2, INT2NUM(self_t->cached->width), INT2NUM(self_t->cached->height));
}

VALUE shoes_image_motion(VALUE self, int x, int y, char *touch) {
    char h = 0;
    VALUE click;
#ifdef NEW_MACRO_IMAGE
    Get_TypedStruct2(self, shoes_image, self_t);
#else
    shoes_image *self_t;
    Data_Get_Struct(self, shoes_image, self_t);
#endif

    click = ATTR(self_t->attr, click);
    if (self_t->cached == NULL) return Qnil;

    if (IS_INSIDE(self_t, x, y)) {
        if (!NIL_P(click)) {
            shoes_canvas *canvas;
            Data_Get_Struct(self_t->parent, shoes_canvas, canvas);
            shoes_app_cursor(canvas->app, s_link);
        }
        h = 1;
    }

    CHECK_HOVER(self_t, h, touch);

    return h ? click : Qnil;
}

VALUE shoes_image_send_click(VALUE self, int button, int x, int y) {
    VALUE v = Qnil;

    if (button > 0) {
#ifdef NEW_MACRO_IMAGE
        Get_TypedStruct2(self, shoes_image, self_t);
#else
        shoes_image *self_t;
        Data_Get_Struct(self, shoes_image, self_t);
#endif
        v = shoes_image_motion(self, x, y, NULL);
        if (self_t->hover & HOVER_MOTION)
            self_t->hover = HOVER_MOTION | HOVER_CLICK;
    }

    return v;
}

// Return t/f if x,y points to the image
VALUE shoes_image_event_is_here(VALUE self, int x, int y) {
#ifdef NEW_MACRO_IMAGE
    Get_TypedStruct2(self, shoes_image, img);
#else
  shoes_image *img;
  Data_Get_Struct(self, shoes_image, img);
#endif
  if (IS_INSIDE(img, x, y)) 
    return Qtrue;
  else 
    return Qnil;
}

void shoes_image_send_release(VALUE self, int button, int x, int y) {
#ifdef NEW_MACRO_IMAGE
    Get_TypedStruct2(self, shoes_image, self_t);
#else
    shoes_image *self_t;
    Data_Get_Struct(self, shoes_image, self_t);
#endif
    if (button > 0 && (self_t->hover & HOVER_CLICK)) {
        VALUE proc = ATTR(self_t->attr, release);
        self_t->hover ^= HOVER_CLICK;
        if (!NIL_P(proc))
            shoes_safe_block(self, proc, rb_ary_new3(3, INT2NUM(button), INT2NUM(x), INT2NUM(y)));
    }
}

// canvas
VALUE shoes_canvas_image(int argc, VALUE *argv, VALUE self) {
    rb_arg_list args;
    VALUE path = Qnil, attr = Qnil, _w, _h, image;

    switch (rb_parse_args(argc, argv, "ii|h,s|h,|h", &args)) {
        case 1:
            _w = args.a[0];
            _h = args.a[1];
            attr = args.a[2];
            ATTRSET(attr, width, _w);
            ATTRSET(attr, height, _h);
            if (rb_block_given_p()) ATTRSET(attr, draw, rb_block_proc());
            break;

        case 2:
            path = args.a[0];
            attr = args.a[1];
            if (rb_block_given_p()) ATTRSET(attr, click, rb_block_proc());
            break;

        case 3:
            attr = args.a[0];
            if (rb_block_given_p()) ATTRSET(attr, draw, rb_block_proc());
            break;
    }

    if (rb_obj_is_kind_of(self, cImage)) {
        shoes_image_image(self, path, attr);
        return self;
    }

    SETUP_CANVAS();
    image = shoes_image_new(cImage, path, attr, self, canvas->st);
    shoes_add_ele(canvas, image);

    return image;
}

// TODO Below functions belong in canvas.c

VALUE shoes_canvas_nostroke(VALUE self) {
    //SETUP_BASIC();
    SETUP_BASIC_T(self);
    ATTRSET(basic->attr, stroke, Qnil);
    return self;
}

VALUE shoes_canvas_stroke(int argc, VALUE *argv, VALUE self) {
    VALUE pat;
    //SETUP_BASIC();
    SETUP_BASIC_T(self);

    if (argc == 1 && rb_obj_is_kind_of(argv[0], cPattern))
        pat = argv[0];
    else
        pat = shoes_pattern_args(argc, argv, self);

    ATTRSET(basic->attr, stroke, pat);

    return pat;
}

VALUE shoes_canvas_strokewidth(VALUE self, VALUE w) {
    //SETUP_BASIC();
    SETUP_BASIC_T(self);
    ATTRSET(basic->attr, strokewidth, w);
    return self;
}

VALUE shoes_canvas_dash(VALUE self, VALUE dash) {
    //SETUP_BASIC();
    SETUP_BASIC_T(self);
    ATTRSET(basic->attr, dash, dash);
    return self;
}

VALUE shoes_canvas_cap(VALUE self, VALUE cap) {
    //SETUP_BASIC();
    SETUP_BASIC_T(self);
    ATTRSET(basic->attr, cap, cap);
    return self;
}

VALUE shoes_canvas_nofill(VALUE self) {
    //SETUP_BASIC();
    SETUP_BASIC_T(self);
    ATTRSET(basic->attr, fill, Qnil);
    return self;
}

VALUE shoes_canvas_fill(int argc, VALUE *argv, VALUE self) {
    VALUE pat;
    //SETUP_BASIC();
    SETUP_BASIC_T(self);

    if (argc == 1 && rb_obj_is_kind_of(argv[0], cPattern))
        pat = argv[0];
    else
        pat = shoes_pattern_args(argc, argv, self);

    ATTRSET(basic->attr, fill, pat);

    return pat;
}

VALUE shoes_canvas_imagesize(VALUE self, VALUE _path) {
    int w, h;
    if (shoes_load_imagesize(_path, &w, &h) == SHOES_OK)
        return rb_ary_new3(2, INT2NUM(w), INT2NUM(h));
    return Qnil;
}
