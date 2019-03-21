// line chart
#include "shoes/plot/plot.h"

// forward declares in this file:
void shoes_plot_line_nub(cairo_t *, int, int);


void shoes_plot_draw_datapts(cairo_t *cr, shoes_plot *plot) {
    int i;
    int top,left,bottom,right;
    left = plot->graph_x;
    top = plot->graph_y;
    right = plot->graph_w;
    bottom = plot->graph_h;
    for (i = 0; i < plot->seriescnt; i++) {
        VALUE rbser = rb_ary_entry(plot->series, i);
#ifdef NEW_MACRO_CHARTSERIES
        Get_TypedStruct2(rbser, shoes_chart_series, cs);
#else
        shoes_chart_series *cs;
        Data_Get_Struct(rbser, shoes_chart_series, cs);
#endif
        VALUE rbvalues = cs->values;
        VALUE rbmaxv = cs->maxv;
        double maximum = NUM2DBL(rbmaxv);
        VALUE rbminv = cs->minv;
        double minimum = NUM2DBL(rbminv);
        int strokew = NUM2INT(cs->strokes);
#ifdef NEW_MACRO_COLOR
        Get_TypedStruct2(cs->color, shoes_color, color);
#else
        shoes_color *color;
        Data_Get_Struct(cs->color, shoes_color, color);
#endif
        /*
        VALUE rbvalues = rb_ary_entry(plot->values, i);
        VALUE rbmaxv = rb_ary_entry(plot->maxvs, i);
        VALUE rbminv = rb_ary_entry(plot->minvs, i);
        VALUE rbstroke = rb_ary_entry(plot->strokes, i);
        VALUE rbnubs = rb_ary_entry(plot->nubs, i);
        VALUE shcolor = rb_ary_entry(plot->color, i);
        shoes_color *color;
        Data_Get_Struct(shcolor, shoes_color, color);
        double maximum = NUM2DBL(rbmaxv);
        double minimum = NUM2DBL(rbminv);
        int strokew = NUM2INT(rbstroke);
        */
        if (strokew < 1) strokew = 1;
        cairo_set_line_width(cr, strokew);
        // Shoes: Remember - we use ints for x, y, w, h and for drawing lines and points
        int height = bottom - top;
        int width = right - left;
        int range = plot->end_idx - plot->beg_idx; // zooming adj
        float vScale = height / (maximum - minimum);
        float hScale = width / (double) (range - 1);
        //int nubs = (width / range > 10) ? NUM2INT(rbnubs) : 0;
        int nubs = (width / range > 10) ? NUM2INT(cs->point_type) : 0;

        cairo_set_source_rgba(cr, color->r / 255.0, color->g / 255.0,
                              color->b / 255.0, color->a / 255.0);

        int j;
        int brk = 0; // for missing value control
        for (j = 0; j < range; j++) {
            VALUE rbdp = rb_ary_entry(rbvalues, j + plot->beg_idx);
            if (NIL_P(rbdp)) {
                if (plot->missing == MISSING_MIN) {
                    rbdp = rbminv;
                } else if (plot->missing == MISSING_MAX) {
                    rbdp = rbmaxv;
                } else {
                    brk = 1;
                    continue;
                }
            }
            double v = NUM2DBL(rbdp);
            long x = roundl(j * hScale);
            long y = height - roundl((v - minimum) *vScale);
            x += left;
            y += top;
            //printf("draw i: %i, x: %i, y: %i %f \n", j, (int) x, (int) y, hScale);
            if (j == 0 || brk == 1) {
                cairo_move_to(cr, x, y);
                brk = 0;
            } else {
                cairo_line_to(cr, x, y);
            }
            if (nubs) {
                //shoes_plot_line_nub(cr, x, y);
                // TODO: shoes_plot_draw_nub(cr, plot, x, y, nubs, strokew + 2);
                shoes_plot_draw_nub(cr, plot, x, y, 7, strokew + 2); // 7 will default to old code
            }
        }
        cairo_stroke(cr);
        cairo_set_line_width(cr, 1.0); // reset between series
    } // end of drawing one series
    // tell cairo to draw all lines (and points)
    cairo_stroke(cr);
    // set color back to dark gray and stroke to 1
    cairo_set_source_rgba(cr, 0.9, 0.9, 0.9, 1.0);
    cairo_set_line_width(cr, 1.0);
}

void shoes_plot_line_nub(cairo_t *cr, int x, int y) {
    int sz = 2;

    cairo_move_to(cr, x - sz, y - sz);
    cairo_line_to(cr, x + sz, y - sz);

    cairo_move_to(cr, x - sz, y - sz);
    cairo_line_to(cr, x - sz, y + sz);

    cairo_move_to(cr, x + sz, y + sz);
    cairo_line_to(cr, x + sz, y - sz);

    cairo_move_to(cr, x + sz, y + sz);
    cairo_line_to(cr, x - sz, y + sz);

    cairo_move_to(cr, x, y); // back to center point.
}



// called at draw time.
void shoes_plot_line_draw(cairo_t *cr, shoes_place *place, shoes_plot *self_t) {
    shoes_plot_util_adornments(cr, place, self_t, 50);
    if (self_t->seriescnt) {
        // draw  ticks and x,y labels.
        shoes_plot_draw_ticks_and_labels(cr, self_t);
        shoes_plot_draw_legend(cr, self_t);
        shoes_plot_draw_datapts(cr, self_t); // draw data
    }
}
