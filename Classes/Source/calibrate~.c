#include "m_pd.h"
#include "g_canvas.h"
#include "Audio_Math.h"

#define SQRT_8 2.8284271247
#define SQRT_2 1.4142135624

// this is some stuff from my own version of pd next
// it'll look weird in vanilla, but idk how to do this where It changes based on the compilation
// and besides most people are getting this from deken, not compiling it.
#define CORNER_RADIUS 14 /* amount to inset the corners for rounding them */
#define CORNER_INSET ((CORNER_RADIUS / 2) - 2) /* corner locations after rounding */

static t_class *calibrate_tilde_class;
static t_widgetbehavior calibrate_tilde_widgetbehavior;

// start control point

enum type {TLCORNER, TRCORNER, BLCORNER, BRCORNER, LEDGE, REDGE, TEDGE, BEDGE, MIDDLE};

typedef enum {D_BG = 0x3D4451, D_BORDER = 0x050505, D_PNTS = 0x98C379, D_LINES = 0x7F848E, D_IOLET = 0xE4C386, D_SEL = 0xdddddd,
              LBG, LBD, LPNTS, LLINES, LIOLET, LSEL} colors;

typedef struct _control_point
{
    int radius;
    int type;
    int idx;
    t_float max_dist;
    bool has_been_scaled;
    t_vec3 pos, scale, saved_scale, start, origin;
} t_control_point;

typedef struct _calibrate
{
    t_object obj;
    t_sample f; // dummy variable for 1st inlet
    t_control_point *c_pnts;
    t_control_point corners[4];
    t_glist *glist;
    int size;
    int n_cpnts;
    t_float mousex, mousey;
    int zoom;
    int grid_size;
    int edge_offset;
    bool selected, bypass, shift_pressed;
    t_symbol *bindname;
    t_inlet *y_in; // x_in and x_out default provided
    t_outlet *y_out;
} t_calibrate_tilde;

static t_control_point control_point_new(int type, t_vec3 position, int idx) {
    return (t_control_point){5, type, idx, 0.f, false, position, vec3(1,1,1), vec3(1,1,1), position, position};
}

static bool control_point_check_hover(t_control_point *this, t_float mousex, t_float mousey) {
    t_float xdist = mousex - this->pos.x;
    t_float ydist = mousey - this->pos.y;

    t_float dist_sqrd = xdist*xdist + ydist * ydist;
    return dist_sqrd < this->radius*this->radius;
}

static void control_point_update_scale(t_control_point *this, int size, int edge_offset)
{
    t_float dx = 1, dy = 1;
    t_float xstart = this->start.x;
    t_float ystart = this->start.y;
    t_float xpos = this->pos.x;
    t_float ypos = this->pos.y;
    t_float type = this->type;
    // scale pos and start so that the top left of the grid returns a scale of 0.
    if(type > 2 && type != TEDGE && type != LEDGE)
    {
        dx = map_lin(xpos, edge_offset, size-edge_offset, 0, 1, false) / map_lin(xstart, edge_offset, size-edge_offset, 0, 1, false);
        dy = map_lin(ypos, edge_offset, size-edge_offset, 0, 1, false) / map_lin(ystart, edge_offset, size-edge_offset, 0, 1, false);
    } else if(type != BLCORNER && type != TRCORNER)
    {
        // calculate percentage change from the bottom right instead to avoid div by zero
        dx = map_lin(xpos, edge_offset, size-edge_offset, 1, 0, false) / map_lin(xstart, edge_offset, size-edge_offset, 1, 0, false);
        dy = map_lin(ypos, edge_offset, size-edge_offset, 1, 0, false) / map_lin(ystart, edge_offset, size-edge_offset, 1, 0, false);
        dx = 1+(1-dx);
        dy = 1+(1-dy);
    } else if(type == BLCORNER)
    { // calc from top right instead
        dx = map_lin(xpos, edge_offset, size-edge_offset, 1, 0, false) / map_lin(xstart, edge_offset, size-edge_offset, 1, 0, false);
        dy = map_lin(ypos, edge_offset, size-edge_offset, 0, 1, false) / map_lin(ystart, edge_offset, size-edge_offset, 0, 1, false);
        dx = 1+(1-dx);
    } else if(type == TRCORNER)
    { // calc from bottom left instead
        dx = map_lin(xpos, edge_offset, size-edge_offset, 0, 1, false) / map_lin(xstart, edge_offset, size-edge_offset, 0, 1, false);
        dy = map_lin(ypos, edge_offset, size-edge_offset, 1, 0, false) / map_lin(ystart, edge_offset, size-edge_offset, 1, 0, false);
        dy = 1+(1-dy);
    }

    this->scale.x = CLAMP(dx, 0, 2);
    this->scale.y = CLAMP(dy, 0, 2);
    this->saved_scale = this->scale; // for calibrate_tilde_distort, other scale gets set to 1 for gui purposes

    this->has_been_scaled = true;
}

static void control_point_set_max_dist(t_control_point *this, t_control_point *corners) {
    for(int i = 0; i < 4; ++i) // there's only 4 corners, :)
    {
        t_float xdist = this->start.x - corners[i].start.x;
        t_float ydist = this->start.y - corners[i].start.y;

        t_float dist = sqrtf(xdist*xdist + ydist*ydist);
        if(dist > this->max_dist)
            this->max_dist = dist;
    }
}

// what is a better name for this func?
static void control_point_reset_start_and_scale(t_control_point *this) {
    this->has_been_scaled = false;
    this->scale = vec3(1,1,1);
    this->start = this->pos;
}

static t_vec3 control_point_calc_offset(t_vec3 start, t_vec3 scale, t_float weight) {
    t_vec3 off = vec3(0,0,0);

    if(scale.x < 1) {
      off.x = -start.x*(1-scale.x)*weight;
    } else if (scale.x > 1) {
      off.x = start.x*(scale.x-1)*weight;
    }
    if(scale.y < 1) {
      off.y = -start.y*(1-scale.y)*weight;
    } else if (scale.y > 1) {
      off.y = start.y*(scale.y-1)*weight;
    }

    return off;
}

static void control_point_distort(t_control_point *this, t_control_point *others, t_calibrate_tilde *owner) {
    if(control_point_check_hover(this, owner->mousex, owner->mousey)) return;

    t_vec3 offsets = vec3(0,0,0);
    for(int i = 0; i < owner->n_cpnts; ++i)
    {
        if(this->idx != others[i].idx) // make sure we don't try to distort ourselves
        {
            t_float xdist = this->start.x - others[i].start.x;
            t_float ydist = this->start.y - others[i].start.y;

            t_float dist = sqrtf(xdist*xdist + ydist*ydist);
            t_float weight = powf(map_lin(dist, 0, others[i].max_dist, 1, 0, false) / SQRT_2, 2);

            // if I'm a middle i just apply all scales
            if(this->type == MIDDLE) {
                offsets = v3_add(offsets, control_point_calc_offset(this->start, others[i].scale, weight));
            } else if(this->type != MIDDLE && others[i].type != MIDDLE) {
                // only edges and corners effect edges and corners
                offsets = v3_add(offsets, control_point_calc_offset(this->start, others[i].scale, weight));
            }

            this->pos.x = this->start.x + offsets.x;
            this->pos.y = this->start.y + offsets.y;
        }
    }
}

// returns where cp is on the grid generically
static int control_point_type(int idx, int grid_size)
{
  if (idx == 0) return TLCORNER;
  if (idx == grid_size-1) return TRCORNER;
  if (idx == grid_size*(grid_size-1)) return BLCORNER;
  if (idx == grid_size*grid_size-1) return BRCORNER;
  if ((idx+1)%grid_size == 0) return REDGE;
  if (idx%grid_size == 0) return LEDGE;
  if (0 < idx && idx < grid_size-1) return TEDGE;
  if (grid_size*(grid_size-1) < idx && idx < grid_size*grid_size-1) return BEDGE;
  return MIDDLE;
}

// end control point

int xy_to_idx(int x, int y, int grid_size) { return x + grid_size * y; }

// start dsp functions

/* issues with distort
    - top and left edges move in the right direction, but don't distort with the proper weights
*/

static t_vec3 calibrate_tilde_distort(t_calibrate_tilde *this, t_float x, t_float y)
{
    t_float offx = 0, offy = 0;
    t_vec3 offset = vec3(0, 0, 0);
    for(int i = 0; i < this->n_cpnts; ++i) {

        t_control_point cp = this->c_pnts[i];
        // x and y range from -1 to 1, but my distortion math assumes all positive coordinates
        // also the control points are in a vastly different range (from 0 to size)
        t_float origin_x = map_lin(cp.origin.x, this->edge_offset, this->size-this->edge_offset, -1, 1, false);
        t_float origin_y = map_lin(cp.origin.y, this->edge_offset, this->size-this->edge_offset, 1, -1, false);
        t_float xdist =  x - origin_x; // no abs, we're squaring it next, so we don't need to worry about a neg dist
        t_float ydist =  y - origin_y;

        t_float dist = sqrtf(xdist*xdist + ydist*ydist); // what is actual max_dist?
        t_float weight = pow(map_lin(dist, 0, SQRT_8, 1, 0, false) / 6, 2);

        // we don't want to scale the point, we want to scale the point's distance from (-1, 1)
        // so first we calculate that distance and then calculate the offsets based on that
        // reusing xdist and ydist here since we're done using them to calculate the weights

        if(x <= -0.9 || y >= 0.9) { // dist is from bot right for top and left edges
            xdist = -1*(x - 1);
            ydist = -1*(y + 1);
            //weight *= 0.07; // this is fiddly and doesn't actually solve the problem
        } else {
            xdist = x + 1;
            ydist = y - 1;
        }

        if (cp.saved_scale.x < 1) {
            offx = -xdist*(1-cp.saved_scale.x)*weight;
        } else if (cp.saved_scale.x > 1) {
            offx = xdist*(cp.saved_scale.x-1)*weight;
        }
        if (cp.saved_scale.y < 1) {
            offy = -ydist*(1-cp.saved_scale.y)*weight;
        } else if (cp.saved_scale.y > 1) {
            offy = ydist*(cp.saved_scale.y-1)*weight;
        }

        offset.x += offx;
        offset.y += offy;
    }

    return offset;
}
static t_int *calibrate_tilde_perform(t_int *w)
{
    t_calibrate_tilde *this = (t_calibrate_tilde *)(w[1]);
    t_sample *x_in          = (t_sample *)(w[2]);
    t_sample *y_in          = (t_sample *)(w[3]);
    t_sample *x_out         = (t_sample *)(w[4]);
    t_sample *y_out         = (t_sample *)(w[5]);
    int nblock              = (int)(w[6]); // get block size

    while (nblock--)
    {
        t_sample x = CLAMP(x_in[nblock], -1, 1);
        t_sample y = CLAMP(y_in[nblock], -1, 1);
        t_vec3 out = vec3(x, y, 0);
        if(!this->bypass) {
            t_vec3 off = calibrate_tilde_distort(this, x, y);
            out = v3_add(out, off);
            x_out[nblock] = out.x; // no clamp so that if we need to push edges out we can
            y_out[nblock] = out.y;
        } else {
            x_out[nblock] = out.x;
            y_out[nblock] = out.y;
        }
    }

    return (w + 7);
}

static void calibrate_tilde_dsp(t_calibrate_tilde *this, t_signal **sp)
{
    dsp_add(calibrate_tilde_perform, 6, this,
            sp[0]->s_vec, // x_in
            sp[1]->s_vec, // y_in
            sp[2]->s_vec, // x_out
            sp[3]->s_vec, // y_out
            sp[0]->s_n); // block size
}

static void calibrate_tilde_bypass(t_calibrate_tilde *this, t_floatarg bypass) {
    this->bypass = (int)bypass;
}
// end dsp functions


// start drawing functions
static void calibrate_tilde_draw_lines(t_calibrate_tilde *this, t_glist *glist)
{
    int xpos = text_xpix(&this->obj, glist);
    int ypos = text_ypix(&this->obj, glist);
    t_canvas *canvas = glist_getcanvas(glist);

    for(int x = 0; x < this->grid_size; ++x) {
            for(int y = 0; y < this->grid_size; ++y) {
                t_vec3 current = this->c_pnts[CLAMP(xy_to_idx(x, y, this->grid_size), 0, this->n_cpnts)].pos;
                t_vec3 right = this->c_pnts[CLAMP(xy_to_idx(x+1, y,this->grid_size), 0, this->n_cpnts)].pos;
                t_vec3 left = this->c_pnts[CLAMP(xy_to_idx(x-1, y,this->grid_size), 0, this->n_cpnts)].pos;
                t_vec3 below = this->c_pnts[CLAMP(xy_to_idx(x, y+1,this->grid_size), 0, this->n_cpnts)].pos;
                t_vec3 above = this->c_pnts[CLAMP(xy_to_idx(x, y-1,this->grid_size), 0, this->n_cpnts)].pos;
                int type = this->c_pnts[CLAMP(xy_to_idx(x, y, this->grid_size), 0, this->n_cpnts)].type;

                switch(type) {
                    case TLCORNER:
                        sys_vgui(".x%lx.c create line %d %d %d %d "
                                    "-fill #%06x "
                                    "-width 2 "
                                    "-tag %lxLINES\n",
                                    canvas, xpos+(int)current.x, ypos+(int)current.y, xpos+(int)right.x, ypos+(int)right.y, D_LINES, this);
                        sys_vgui(".x%lx.c create line %d %d %d %d "
                                    "-fill #%06x "
                                    "-width 2 "
                                    "-tag %lxLINES\n",
                                    canvas, xpos+(int)current.x, ypos+(int)current.y, xpos+(int)below.x, ypos+(int)below.y, D_LINES, this);
                        break;
                    case TRCORNER:
                        sys_vgui(".x%lx.c create line %d %d %d %d "
                                    "-fill #%06x "
                                    "-width 2 "
                                    "-tag %lxLINES\n",
                                    canvas, xpos+(int)current.x, ypos+(int)current.y, xpos+(int)left.x, ypos+(int)left.y, D_LINES, this);
                        sys_vgui(".x%lx.c create line %d %d %d %d "
                                    "-fill #%06x "
                                    "-width 2 "
                                    "-tag %lxLINES\n",
                                    canvas, xpos+(int)current.x, ypos+(int)current.y, xpos+(int)below.x, ypos+(int)below.y, D_LINES, this);
                        break;
                    case BLCORNER:
                        sys_vgui(".x%lx.c create line %d %d %d %d "
                                    "-fill #%06x "
                                    "-width 2 "
                                    "-tag %lxLINES\n",
                                    canvas, xpos+(int)current.x, ypos+(int)current.y, xpos+(int)right.x, ypos+(int)right.y, D_LINES, this);
                        sys_vgui(".x%lx.c create line %d %d %d %d "
                                    "-fill #%06x "
                                    "-width 2 "
                                    "-tag %lxLINES\n",
                                    canvas, xpos+(int)current.x, ypos+(int)current.y, xpos+(int)above.x, ypos+(int)above.y, D_LINES, this);
                        break;
                    case BRCORNER:
                        sys_vgui(".x%lx.c create line %d %d %d %d "
                                    "-fill #%06x "
                                    "-width 2 "
                                    "-tag %lxLINES\n",
                                    canvas, xpos+(int)current.x, ypos+(int)current.y, xpos+(int)left.x, ypos+(int)left.y, D_LINES, this);
                        sys_vgui(".x%lx.c create line %d %d %d %d "
                                    "-fill #%06x "
                                    "-width 2 "
                                    "-tag %lxLINES\n",
                                    canvas, xpos+(int)current.x, ypos+(int)current.y, xpos+(int)above.x, ypos+(int)above.y, D_LINES, this);
                        break;
                    case TEDGE:
                    case BEDGE:
                        sys_vgui(".x%lx.c create line %d %d %d %d "
                                    "-fill #%06x "
                                    "-width 2 "
                                    "-tag %lxLINES\n",
                                    canvas, xpos+(int)current.x, ypos+(int)current.y, xpos+(int)left.x, ypos+(int)left.y, D_LINES, this);
                        sys_vgui(".x%lx.c create line %d %d %d %d "
                                    "-fill #%06x "
                                    "-width 2 "
                                    "-tag %lxLINES\n",
                                    canvas, xpos+(int)current.x, ypos+(int)current.y, xpos+(int)right.x, ypos+(int)right.y, D_LINES, this);
                        break;
                    case LEDGE:
                    case REDGE:
                        sys_vgui(".x%lx.c create line %d %d %d %d "
                                    "-fill #%06x "
                                    "-width 2 "
                                    "-tag %lxLINES\n",
                                    canvas, xpos+(int)current.x, ypos+(int)current.y, xpos+(int)above.x, ypos+(int)above.y, D_LINES, this);
                        sys_vgui(".x%lx.c create line %d %d %d %d "
                                    "-fill #%06x "
                                    "-width 2 "
                                    "-tag %lxLINES\n",
                                    canvas, xpos+(int)current.x, ypos+(int)current.y, xpos+(int)below.x, ypos+(int)below.y, D_LINES, this);
                        break;
                    default: // otherwise you're a middle point
                        sys_vgui(".x%lx.c create line %d %d %d %d "
                                    "-fill #%06x "
                                    "-width 2 "
                                    "-tag %lxLINES\n",
                                    canvas, xpos+(int)current.x, ypos+(int)current.y, xpos+(int)left.x, ypos+(int)left.y, D_LINES, this);
                        sys_vgui(".x%lx.c create line %d %d %d %d "
                                    "-fill #%06x "
                                    "-width 2 "
                                    "-tag %lxLINES\n",
                                    canvas, xpos+(int)current.x, ypos+(int)current.y, xpos+(int)right.x, ypos+(int)right.y, D_LINES, this);
                        sys_vgui(".x%lx.c create line %d %d %d %d "
                                    "-fill #%06x "
                                    "-width 2 "
                                    "-tag %lxLINES\n",
                                    canvas, xpos+(int)current.x, ypos+(int)current.y, xpos+(int)above.x, ypos+(int)above.y, D_LINES, this);
                        sys_vgui(".x%lx.c create line %d %d %d %d "
                                    "-fill #%06x "
                                    "-width 2 "
                                    "-tag %lxLINES\n",
                                    canvas, xpos+(int)current.x, ypos+(int)current.y, xpos+(int)below.x, ypos+(int)below.y, D_LINES, this);
                        break;
                }
            }
        }
}

static void calibrate_tilde_draw_points(t_calibrate_tilde *this, t_glist *glist)
{
    int xpos = text_xpix(&this->obj, glist);
    int ypos = text_ypix(&this->obj, glist);
    t_canvas *canvas = glist_getcanvas(glist);

    // draw the control points
    for(int i = 0; i < this->n_cpnts; ++i)
    {
        t_control_point cp = this->c_pnts[i];
        int x1 = xpos+cp.pos.x - cp.radius;
        int y1 = ypos+cp.pos.y - cp.radius;
        int x2 = xpos+cp.pos.x + cp.radius;
        int y2 = ypos+cp.pos.y + cp.radius;

        // tcl/tk doesn't draw circles from the center, so the formula is this
        // x1,y1 = center-radius x2,y2 = center+radius
        sys_vgui(".x%lx.c create oval %d %d %d %d "
                "-fill #%06x "
                "-width 0 "
                "-tag %lxPOINTS\n",
                canvas, x1, y1, x2, y2, D_PNTS, this);
    }
}

static void calibrate_tilde_draw_iolets(t_calibrate_tilde *this, t_glist *glist)
{
    int xpos = text_xpix(&this->obj, glist);
    int ypos = text_ypix(&this->obj, glist);
    t_canvas *canvas = glist_getcanvas(glist);

    sys_vgui(".x%lx.c create rectangle %d %d %d %d -fill #%06x -width 0 -tags %lxIOLET\n",
        canvas, xpos+CORNER_INSET, ypos, xpos+CORNER_INSET+IOWIDTH, ypos+IHEIGHT, D_IOLET, this);
    sys_vgui(".x%lx.c create rectangle %d %d %d %d -fill #%06x -width 0 -tags %lxIOLET\n",
        canvas, xpos+this->size-CORNER_INSET, ypos, xpos+this->size-CORNER_INSET-IOWIDTH, ypos+IHEIGHT, D_IOLET, this);

    sys_vgui(".x%lx.c create rectangle %d %d %d %d -fill #%06x -width 0 -tags %lxIOLET\n",
        canvas, xpos+CORNER_INSET, ypos+this->size-IHEIGHT, xpos+CORNER_INSET+IOWIDTH, ypos+this->size, D_IOLET, this);
    sys_vgui(".x%lx.c create rectangle %d %d %d %d -fill #%06x -width 0 -tags %lxIOLET\n",
        canvas,  xpos+this->size-CORNER_INSET, ypos+this->size-IHEIGHT, xpos+this->size-CORNER_INSET-IOWIDTH, ypos+this->size, D_IOLET, this);
}

static void calibrate_tilde_draw(t_calibrate_tilde *this, t_glist *glist)
{

    int xpos = text_xpix(&this->obj, glist);
    int ypos = text_ypix(&this->obj, glist);
    t_canvas *canvas = glist_getcanvas(glist);

    // draw the base
    sys_vgui(".x%lx.c create rectangle %d %d %d %d "
             "-fill #%06x "
             "-outline #%06x "
             "-width 2 "
             "-tag %lxBASE\n",
             canvas, xpos, ypos, xpos+this->size, ypos+this->size, D_BG, D_BORDER, this);

    calibrate_tilde_draw_lines(this, this->glist);
    calibrate_tilde_draw_points(this, this->glist);
    calibrate_tilde_draw_iolets(this, this->glist);
}

static void calibrate_tilde_erase(t_calibrate_tilde *this, t_glist *glist)
{
    t_canvas *canvas = glist_getcanvas(glist);
    sys_vgui(".x%lx.c delete %lxBASE\n", canvas, this);
    sys_vgui(".x%lx.c delete %lxPOINTS\n", canvas, this);
    sys_vgui(".x%lx.c delete %lxLINES\n", canvas, this);
    sys_vgui(".x%lx.c delete %lxIOLET\n", canvas, this);
    sys_vgui(".x%lx.c delete %lxIOLET\n", canvas, this);
}

static void calibrate_tilde_update(t_calibrate_tilde *this, t_glist *glist)
{
    t_canvas *canvas = glist_getcanvas(glist);
    sys_vgui(".x%lx.c delete %lxPOINTS\n", canvas, this);
    sys_vgui(".x%lx.c delete %lxLINES\n", canvas, this);
    calibrate_tilde_draw_lines(this, this->glist);
    calibrate_tilde_draw_points(this, this->glist);
}

static void calibrate_tilde_reset(t_calibrate_tilde *this) {
    for(int x = 0; x < this->grid_size; ++x) {
        for(int y = 0; y < this->grid_size; ++y) {
            int i = xy_to_idx(x, y, this->grid_size);
            t_vec3 pos = vec3(map_lin(x, 0, this->grid_size-1, this->edge_offset, this->size-this->edge_offset, false),
                              map_lin(y, 0, this->grid_size-1, this->edge_offset, this->size-this->edge_offset, false), 0);

            this->c_pnts[i].pos = pos;
            this->c_pnts[i].scale = vec3(1,1,1);
            this->c_pnts[i].saved_scale = vec3(1,1,1);
            this->c_pnts[i].start = pos;
        }
    }
    calibrate_tilde_update(this, this->glist);
}
// end drawing functions

// start widgetbehavior Functions
static void calibrate_tilde_getrect(t_gobj *z, t_glist *glist, int *xp1, int *yp1, int *xp2, int *yp2)
{
    t_calibrate_tilde *this = (t_calibrate_tilde *)z;
    *xp1 = text_xpix(&this->obj, glist);
    *yp1 = text_ypix(&this->obj, glist);
    *xp2 = *xp1 + this->size;// * zoom_factor?
    *yp2 = *yp1 + this->size;
}
static void calibrate_tilde_displace(t_gobj *z, t_glist *glist, int dx, int dy)
{
    t_calibrate_tilde *this = (t_calibrate_tilde*)z;
    this->obj.te_xpix += dx, this->obj.te_ypix += dy;
    t_canvas *canvas = glist_getcanvas(glist);

    sys_vgui(".x%lx.c move %lxBASE %d %d\n", canvas, this, dx, dy);
    sys_vgui(".x%lx.c move %lxLINES %d %d\n", canvas, this, dx, dy);
    sys_vgui(".x%lx.c move %lxPOINTS %d %d\n", canvas, this, dx, dy);
    sys_vgui(".x%lx.c move %lxIOLET %d %d\n", canvas, this, dx, dy);

    canvas_fixlinesfor(glist, (t_text *)this);
}
static void calibrate_tilde_select(t_gobj *z, t_glist *glist, int sel)
{
    t_calibrate_tilde *this = (t_calibrate_tilde*)z;
    t_canvas *canvas = glist_getcanvas(glist);

    if((this->selected = sel)) {
        sys_vgui(".x%lx.c itemconfigure -outline blue %lxBASE\n",
            canvas, this);
    } else
        sys_vgui(".x%lx.c itemconfigure -outline black %lxBASE\n",
            canvas, this);
}
static void calibrate_tilde_delete(t_gobj *z, t_glist *glist) {
    canvas_deletelinesfor(glist, (t_text *)z);
}
static void calibrate_tilde_vis(t_gobj *z, t_glist *glist, int vis)
{
    t_calibrate_tilde *this = (t_calibrate_tilde*)z;
    t_canvas *canvas = glist_getcanvas(glist);

    if(vis) {
        calibrate_tilde_draw(this, glist);
        sys_vgui(".x%lx.c bind %lxBASE <ButtonRelease> {pdsend [concat %s _mouserelease \\;]}\n",
            canvas, this, this->bindname->s_name);
        sys_vgui(".x%lx.c bind %lxLINES <ButtonRelease> {pdsend [concat %s _mouserelease \\;]}\n",
            canvas, this, this->bindname->s_name);
        sys_vgui(".x%lx.c bind %lxPOINTS <ButtonRelease> {pdsend [concat %s _mouserelease \\;]}\n",
            canvas, this, this->bindname->s_name);
        sys_vgui(".x%lx.c bind %lxIOLET <ButtonRelease> {pdsend [concat %s _mouserelease \\;]}\n",
            canvas, this, this->bindname->s_name);
    } else {
        calibrate_tilde_erase(this, glist);
    }
}

static void calibrate_tilde_key(t_calibrate_tilde *this, t_floatarg key)
{
    key = (int)key;
    if(key == 'r' || key == 'R') {
        calibrate_tilde_reset(this);

    }
}

static void calibrate_tilde_mouserelease(t_calibrate_tilde *this) {
    for(int i = 0; i < this->n_cpnts; ++i) {
        control_point_reset_start_and_scale(&this->c_pnts[i]);
    }
}

static void calibrate_tilde_follow_mouse(t_calibrate_tilde *this, t_glist *glist)
{
    for(int i = 0; i < this->n_cpnts; ++i) {
        // this gobbledigook of glist_grab -> motion -> follow_mouse is the equivalent of
        // if(mousePressed) in the draw() method in processing... fml
        if(control_point_check_hover(&this->c_pnts[i], this->mousex, this->mousey)) {
            // update the position
            this->c_pnts[i].pos.x = this->mousex;
            this->c_pnts[i].pos.y = this->mousey;
            control_point_update_scale(&this->c_pnts[i], this->size, this->edge_offset);
        }
        control_point_distort(&this->c_pnts[i], this->c_pnts, this);
        this->c_pnts[i].pos.x = CLAMP(this->c_pnts[i].pos.x, 0, this->size);
        this->c_pnts[i].pos.y = CLAMP(this->c_pnts[i].pos.y, 0, this->size);
    }
}

static void calibrate_tilde_motion(t_calibrate_tilde *this, t_floatarg dx, t_floatarg dy)
{
    if(this->shift_pressed) {
        dx *= 0.1; dy *= 0.1;
    }

    this->mousex += dx;
    this->mousey += dy;
    calibrate_tilde_follow_mouse(this, this->glist);
    calibrate_tilde_update(this, this->glist);
}
static int calibrate_tilde_click(t_gobj *z, struct _glist *glist, int xpix, int ypix, int shift, int alt, int dbl, int doit) {
    dbl = alt = 0;
    t_calibrate_tilde *this = (t_calibrate_tilde *)z;

    // xpos and ypos is the top left corner of the widget (global to patch)
    // xpix and ypix are global coordinates of the patch we're in
    int xpos = text_xpix(&this->obj, glist);
    int ypos = text_ypix(&this->obj, glist);

    this->mousex = (xpix - xpos) / this->zoom;
    this->mousey = (ypix - ypos) / this->zoom;

    if(doit) {
        this->shift_pressed = shift;
        // this allows mouse motion to be called. motion allows for adjusting points position and scale
        glist_grab(this->glist, &this->obj.te_g, (t_glistmotionfn)calibrate_tilde_motion, (t_glistkeyfn)calibrate_tilde_key, xpix, ypix);
    }

    for(int i = 0; i < this->n_cpnts; ++i) {
        control_point_reset_start_and_scale(&this->c_pnts[i]);
    }

    return 1; //why 1? idk
}
// End widgetbehavior Functions

// start free, new, and setupfunctions
static void *calibrate_tilde_free(t_calibrate_tilde *this)
{
    inlet_free(this->y_in);
    outlet_free(this->y_out);

    return (void *)this;
}

static void *calibrate_tilde_new(t_symbol *s, int argc, t_atom *argv)
{
    t_calibrate_tilde *this = (t_calibrate_tilde *)pd_new(calibrate_tilde_class);

    t_canvas *canvas = canvas_getcurrent();
    this->glist = (t_glist*)canvas;

    char buf[MAXPDSTRING];
    snprintf(buf, MAXPDSTRING-1, ".x%lx", (unsigned long)canvas);
    buf[MAXPDSTRING-1] = 0;

    sprintf(buf, "#%lx", (long)this);
    this->bindname = gensym(buf);
    pd_bind(&this->obj.ob_pd, this->bindname); // for mouserelease messages

    this->bypass = false;
    this->shift_pressed = false;

    // later have the argrid_size control some of these
    this->selected = false;
    this->size = 200;
    this->grid_size = argc ? atom_getfloat(argv) : 5;
    this->grid_size = CLAMP(this->grid_size, 3, 11);
    this->edge_offset = 15;
    this->n_cpnts = this->grid_size*this->grid_size;
    this->mousex = this->mousey = 0;
    this->zoom = this->glist->gl_zoom;

    //t_float offset = argc ? atom_getfloat(argv) : 0.f;
    //t_float strength = argc > 1 ? atom_getfloat(argv+1) : 1.f;

    // allocate memory for the control points and fill the array
    this->c_pnts = (t_control_point*)getbytes(this->n_cpnts*sizeof(t_control_point));
    int idx = 0;
    for(int x = 0; x < this->grid_size; ++x) {
        for(int y = 0; y < this->grid_size; ++y) {
            int i = xy_to_idx(x, y, this->grid_size);
            t_vec3 pos = vec3(map_lin(x, 0, this->grid_size-1, this->edge_offset, this->size-this->edge_offset, false),
                              map_lin(y, 0, this->grid_size-1, this->edge_offset, this->size-this->edge_offset, false), 0);

            int type = control_point_type(i, this->grid_size);
            this->c_pnts[i] = control_point_new(type, pos, i);
            if(type < 4)
                this->corners[idx++] = this->c_pnts[i];
        }
    }

    for(int i = 0; i < this->n_cpnts; ++i)
        control_point_set_max_dist(&this->c_pnts[i], this->corners);

    // init inlets and outlets
    this->y_in = inlet_new(&this->obj, &this->obj.ob_pd, &s_signal, &s_signal);

    outlet_new(&this->obj, &s_signal); // default provided outlet
    this->y_out = outlet_new(&this->obj, &s_signal);

    return (void *)this;
}

void calibrate_tilde_setup(void)
{
    calibrate_tilde_class = class_new(gensym("calibrate~"),
                            (t_newmethod)calibrate_tilde_new,
                            (t_method)calibrate_tilde_free,
                            sizeof(t_calibrate_tilde), // data space
                            CLASS_DEFAULT, // gui apperance, doesn't matter here since we're a gui object
                            A_GIMME, // no args yet
                            0); // no more args

    class_addmethod(calibrate_tilde_class, (t_method)calibrate_tilde_mouserelease, gensym("_mouserelease"), 0);
    class_addmethod(calibrate_tilde_class, (t_method)calibrate_tilde_bypass, gensym("bypass"), A_DEFFLOAT, 0);

    // widget behavior
    calibrate_tilde_widgetbehavior.w_getrectfn  = calibrate_tilde_getrect; // used for drawing connections correctly, and for knowing where the mouse is in relation to the widget.
    calibrate_tilde_widgetbehavior.w_displacefn = calibrate_tilde_displace;
    calibrate_tilde_widgetbehavior.w_selectfn   = calibrate_tilde_select;
    calibrate_tilde_widgetbehavior.w_activatefn = NULL;
    calibrate_tilde_widgetbehavior.w_deletefn   = calibrate_tilde_delete;
    calibrate_tilde_widgetbehavior.w_visfn      = calibrate_tilde_vis;
    calibrate_tilde_widgetbehavior.w_clickfn    = calibrate_tilde_click;
    class_setwidget(calibrate_tilde_class, &calibrate_tilde_widgetbehavior);


    class_sethelpsymbol(calibrate_tilde_class, gensym("calibrate~")); // links to the help patch

    class_addmethod(calibrate_tilde_class, (t_method)calibrate_tilde_dsp, gensym("dsp"), A_CANT, 0);
    CLASS_MAINSIGNALIN(calibrate_tilde_class, t_calibrate_tilde, f); // signal inlet as first inlet
}
