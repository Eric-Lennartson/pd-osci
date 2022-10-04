#include "m_pd.h"
#include "g_canvas.h"
#include "Audio_Math.h"

static t_class *calibrate_tilde_class;
static t_widgetbehavior calibrate_tilde_widgetbehavior;

// start control point

enum type {TLCORNER, TRCORNER, BLCORNER, BRCORNER, LEDGE, REDGE, TEDGE, BEDGE, MIDDLE};

typedef struct _control_point
{
    int radius;
    int type;
    int idx;
    t_float max_dist;
    bool has_been_scaled;
    t_vec3 pos, scale, start;
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
    int mousex, mousey;
    int zoom;
    int grid_size;
    int edge_offset;
    int bypass;
    t_symbol *bindname;
    t_inlet *y_in; // x_in and x_out default provided
    t_outlet *y_out;
} t_calibrate_tilde;

static t_control_point control_point_new(int type, t_vec3 position, int idx) {
    return (t_control_point){5, type, idx, 0.f, false, vec3(position.x, position.y, 0), vec3(1,1,1), vec3(position.x, position.y, 1)};
}


static bool control_point_check_hover(t_control_point *this, int mousex, int mousey) {
    t_float xdist = mousex - this->pos.x;
    t_float ydist = mousey - this->pos.y;

    t_float dist_sqrd = xdist*xdist + ydist * ydist;
    return dist_sqrd < this->radius*this->radius;
}

static void control_point_update_scale(t_control_point *this, int size, int edge_offset)
{
            t_float dx = 1, dy = 1;
            int xstart = this->start.x;
            int ystart = this->start.y;
            int xpos = this->pos.x;
            int ypos = this->pos.y;
            int type = this->type;
            // scale pos and start so that the top left of the grid returns a scale of 0.
            if(type > 2 && type != TEDGE && type != LEDGE) {
                dx = map_lin(xpos, edge_offset, size-edge_offset, 0, 1, false) / map_lin(xstart, edge_offset, size-edge_offset, 0, 1, false);
                dy = map_lin(ypos, edge_offset, size-edge_offset, 0, 1, false) / map_lin(ystart, edge_offset, size-edge_offset, 0, 1, false);
            } else if(type != BLCORNER && type != TRCORNER) {
                // calculate percentage change from the bottom right instead to avoid div by zero
                dx = map_lin(xpos, edge_offset, size-edge_offset, 1, 0, false) / map_lin(xstart, edge_offset, size-edge_offset, 1, 0, false);
                dy = map_lin(ypos, edge_offset, size-edge_offset, 1, 0, false) / map_lin(ystart, edge_offset, size-edge_offset, 1, 0, false);
                dx = 1+(1-dx);
                dy = 1+(1-dy);
            } else if(type == BLCORNER) { // calc from top right instead
                dx = map_lin(xpos, edge_offset, size-edge_offset, 1, 0, false) / map_lin(xstart, edge_offset, size-edge_offset, 1, 0, false);
                dy = map_lin(ypos, edge_offset, size-edge_offset, 0, 1, false) / map_lin(ystart, edge_offset, size-edge_offset, 0, 1, false);
                dx = 1+(1-dx);
            } else if(type == TRCORNER) { // calc from bottom left instead
                dx = map_lin(xpos, edge_offset, size-edge_offset, 0, 1, false) / map_lin(xstart, edge_offset, size-edge_offset, 0, 1, false);
                dy = map_lin(ypos, edge_offset, size-edge_offset, 1, 0, false) / map_lin(ystart, edge_offset, size-edge_offset, 1, 0, false);
                dy = 1+(1-dy);
            }

            this->scale.x = CLAMP(dx, 0, 2);
            this->scale.y = CLAMP(dy, 0, 2);

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

static void control_point_reset_start_and_scale(t_control_point *this, int mx, int my) {
    if(this->has_been_scaled && control_point_check_hover(this, mx, my)) {
        this->has_been_scaled = false;
        this->scale = vec3(1,1,1);
        this->start = this->pos;
    } else {
        this->start = this->pos;
    }
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
            t_float weight = powf(map_lin(dist, 0, others[i].max_dist, 1, 0, false) / sqrt(2), 2);

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
        if(!this->bypass) {
            x_out[nblock] = 0;
            y_out[nblock] = 0;
        } else {
            x_out[nblock] = 0;
            y_out[nblock] = 0;
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
                                    "-fill black "
                                    "-width 2 "
                                    "-tag %lxLINES\n",
                                    canvas, xpos+(int)current.x, ypos+(int)current.y, xpos+(int)right.x, ypos+(int)right.y, this);
                        sys_vgui(".x%lx.c create line %d %d %d %d "
                                    "-fill black "
                                    "-width 2 "
                                    "-tag %lxLINES\n",
                                    canvas, xpos+(int)current.x, ypos+(int)current.y, xpos+(int)below.x, ypos+(int)below.y, this);
                        break;
                    case TRCORNER:
                        sys_vgui(".x%lx.c create line %d %d %d %d "
                                    "-fill black "
                                    "-width 2 "
                                    "-tag %lxLINES\n",
                                    canvas, xpos+(int)current.x, ypos+(int)current.y, xpos+(int)left.x, ypos+(int)left.y, this);
                        sys_vgui(".x%lx.c create line %d %d %d %d "
                                    "-fill black "
                                    "-width 2 "
                                    "-tag %lxLINES\n",
                                    canvas, xpos+(int)current.x, ypos+(int)current.y, xpos+(int)below.x, ypos+(int)below.y, this);
                        break;
                    case BLCORNER:
                        sys_vgui(".x%lx.c create line %d %d %d %d "
                                    "-fill black "
                                    "-width 2 "
                                    "-tag %lxLINES\n",
                                    canvas, xpos+(int)current.x, ypos+(int)current.y, xpos+(int)right.x, ypos+(int)right.y, this);
                        sys_vgui(".x%lx.c create line %d %d %d %d "
                                    "-fill black "
                                    "-width 2 "
                                    "-tag %lxLINES\n",
                                    canvas, xpos+(int)current.x, ypos+(int)current.y, xpos+(int)above.x, ypos+(int)above.y, this);
                        break;
                    case BRCORNER:
                        sys_vgui(".x%lx.c create line %d %d %d %d "
                                    "-fill black "
                                    "-width 2 "
                                    "-tag %lxLINES\n",
                                    canvas, xpos+(int)current.x, ypos+(int)current.y, xpos+(int)left.x, ypos+(int)left.y, this);
                        sys_vgui(".x%lx.c create line %d %d %d %d "
                                    "-fill black "
                                    "-width 2 "
                                    "-tag %lxLINES\n",
                                    canvas, xpos+(int)current.x, ypos+(int)current.y, xpos+(int)above.x, ypos+(int)above.y, this);
                        break;
                    case TEDGE:
                    case BEDGE:
                        sys_vgui(".x%lx.c create line %d %d %d %d "
                                    "-fill black "
                                    "-width 2 "
                                    "-tag %lxLINES\n",
                                    canvas, xpos+(int)current.x, ypos+(int)current.y, xpos+(int)left.x, ypos+(int)left.y, this);
                        sys_vgui(".x%lx.c create line %d %d %d %d "
                                    "-fill black "
                                    "-width 2 "
                                    "-tag %lxLINES\n",
                                    canvas, xpos+(int)current.x, ypos+(int)current.y, xpos+(int)right.x, ypos+(int)right.y, this);
                        break;
                    case LEDGE:
                    case REDGE:
                        sys_vgui(".x%lx.c create line %d %d %d %d "
                                    "-fill black "
                                    "-width 2 "
                                    "-tag %lxLINES\n",
                                    canvas, xpos+(int)current.x, ypos+(int)current.y, xpos+(int)above.x, ypos+(int)above.y, this);
                        sys_vgui(".x%lx.c create line %d %d %d %d "
                                    "-fill black "
                                    "-width 2 "
                                    "-tag %lxLINES\n",
                                    canvas, xpos+(int)current.x, ypos+(int)current.y, xpos+(int)below.x, ypos+(int)below.y, this);
                        break;
                    default: // otherwise you're a middle point
                        sys_vgui(".x%lx.c create line %d %d %d %d "
                                    "-fill black "
                                    "-width 2 "
                                    "-tag %lxLINES\n",
                                    canvas, xpos+(int)current.x, ypos+(int)current.y, xpos+(int)left.x, ypos+(int)left.y, this);
                        sys_vgui(".x%lx.c create line %d %d %d %d "
                                    "-fill black "
                                    "-width 2 "
                                    "-tag %lxLINES\n",
                                    canvas, xpos+(int)current.x, ypos+(int)current.y, xpos+(int)right.x, ypos+(int)right.y, this);
                        sys_vgui(".x%lx.c create line %d %d %d %d "
                                    "-fill black "
                                    "-width 2 "
                                    "-tag %lxLINES\n",
                                    canvas, xpos+(int)current.x, ypos+(int)current.y, xpos+(int)above.x, ypos+(int)above.y, this);
                        sys_vgui(".x%lx.c create line %d %d %d %d "
                                    "-fill black "
                                    "-width 2 "
                                    "-tag %lxLINES\n",
                                    canvas, xpos+(int)current.x, ypos+(int)current.y, xpos+(int)below.x, ypos+(int)below.y, this);
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
                "-fill grey "
                "-width 0 "
                "-tag %lxPOINTS%d\n",
                canvas, x1, y1, x2, y2, this, i);
    }
}

static void calibrate_tilde_erase_points(t_calibrate_tilde *this, t_glist *glist)
{
    t_canvas *canvas = glist_getcanvas(glist);
    for(int i = 0; i < this->n_cpnts; ++i)
       sys_vgui(".x%lx.c delete %lxPOINTS%d\n", canvas, this, i);
}

static void calibrate_tilde_draw(t_calibrate_tilde *this, t_glist *glist)
{
    int xpos = text_xpix(&this->obj, glist);
    int ypos = text_ypix(&this->obj, glist);
    t_canvas *canvas = glist_getcanvas(glist);

    // draw the base
    sys_vgui(".x%lx.c create rectangle %d %d %d %d "
             "-fill white "
             "-outline black "
             "-width 2 "
             "-tag %lxBASE\n",
             canvas, xpos, ypos, xpos+this->size, ypos+this->size, this);

    calibrate_tilde_draw_lines(this, this->glist);
    calibrate_tilde_draw_points(this, this->glist);
}

static void calibrate_tilde_erase(t_calibrate_tilde *this, t_glist *glist)
{
    t_canvas *canvas = glist_getcanvas(glist);
    sys_vgui(".x%lx.c delete %lxBASE\n", canvas, this);
    calibrate_tilde_erase_points(this, this->glist);
    sys_vgui(".x%lx.c delete %lxLINES\n", canvas, this);
}

static void calibrate_tilde_update(t_calibrate_tilde *this, t_glist *glist)
{
    t_canvas *canvas = glist_getcanvas(glist);
    calibrate_tilde_erase_points(this, this->glist);
    sys_vgui(".x%lx.c delete %lxLINES\n", canvas, this);
    calibrate_tilde_draw_lines(this, this->glist);
    calibrate_tilde_draw_points(this, this->glist);
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
static void calibrate_tilde_displace() {}
static void calibrate_tilde_select() {}
static void calibrate_tilde_delete() {}
static void calibrate_tilde_vis(t_gobj *z, t_glist *glist, int vis)
{
    t_calibrate_tilde *this = (t_calibrate_tilde*)z;
    t_canvas *canvas = glist_getcanvas(glist);

    if(vis) {
        calibrate_tilde_draw(this, glist);
        sys_vgui(".x%lx.c bind %lxBASE <ButtonRelease> {pdsend [concat %s _mouserelease \\;]}\n",
            canvas, this, this->bindname->s_name);
    } else {
        calibrate_tilde_erase(this, glist);
    }
}

static void calibrate_tilde_mouserelease(t_calibrate_tilde *this) {
    post("mouse released");
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
    }
}

static void calibrate_tilde_motion(t_calibrate_tilde *this, t_floatarg dx, t_floatarg dy)
{
    this->mousex += (int)(dx/this->zoom);
    this->mousey += (int)(dy/this->zoom);
    //post("motion mouse xy: %d, %d", this->mousex, this->mousey);
    //post("mouse down and moving cursor");
    calibrate_tilde_follow_mouse(this, this->glist);
    calibrate_tilde_update(this, this->glist);
    for(int i = 0; i < this->n_cpnts; ++i) {
        control_point_reset_start_and_scale(&this->c_pnts[i], this->mousex, this->mousey);
    }
}
static int calibrate_tilde_click(t_gobj *z, struct _glist *glist, int xpix, int ypix, int shift, int alt, int dbl, int doit) {
    dbl = shift = alt = 0;
    t_calibrate_tilde *this = (t_calibrate_tilde *)z;

    // xpos and ypos is the top left corner of the widget (global to patch)
    // xpix and ypix are global coordinates of the patch we're in
    int xpos = text_xpix(&this->obj, glist);
    int ypos = text_ypix(&this->obj, glist);

    this->mousex = (xpix - xpos) / this->zoom;
    this->mousey = (ypix - ypos) / this->zoom;

    //post("click mouse xy: %d, %d", this->mousex, this->mousey);

    if(doit) { // same as if(mousePressed) in processing
        // this allows mouse motion to be called. motion allows for adjusting points position and scale
        glist_grab(this->glist, &this->obj.te_g, (t_glistmotionfn)calibrate_tilde_motion, NULL, xpix, ypix);
    } else { // any reason to use this else?
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

    // later have the argrid_size control some of these
    this->size = 180;
    this->grid_size = 5;
    this->edge_offset = 10;
    this->n_cpnts = this->grid_size*this->grid_size;
    this->mousex = this->mousey = 0;
    this->zoom = this->glist->gl_zoom;

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

    //t_float offset = argc ? atom_getfloat(argv) : 0.f;
    //t_float strength = argc > 1 ? atom_getfloat(argv+1) : 1.f;

    this->y_in = inlet_new(&this->obj, &this->obj.ob_pd, &s_signal, &s_signal);

    // pd_float((t_pd*)this->offset_in, offset);
    // pd_float((t_pd*)this->strength_in, strength);

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
                            CLASS_DEFAULT, // gui apperance
                            A_GIMME, // no argrid_size yet
                            0); // no more argrid_size

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
