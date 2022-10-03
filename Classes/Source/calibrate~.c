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
    bool hover;
    t_vec3 pos;
} t_control_point;

static t_control_point control_point_new(int type, t_vec3 position) {
    return (t_control_point){4, type, false, vec3(position.x, position.y, 0)};
}

static void control_point_update_pos(t_control_point *this, int mousex, int mousey) {
    post("this->hover %d", this->hover);
    if(this->hover) {
        this->pos.x += mousex-this->pos.x;
        this->pos.y += mousex-this->pos.y;
    }
}

static void control_point_update_hover(t_control_point *this, int mousex, int mousey) {
    t_float xdist = mousex - this->pos.x;
    t_float ydist = mousey - this->pos.y;

    t_float dist_sqrd = xdist*xdist + ydist * ydist;
    this->hover = dist_sqrd < this->radius*this->radius;
    if(this->hover)
        post("hovering");
}

// returns where cp is on the grid generically
int control_point_type(int idx, int grid_size)
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

typedef struct _calibrate
{
    t_object obj;
    t_sample f; // dummy variable for 1st inlet
    t_control_point *c_pnts;
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

    // draw the connecting lines
    for(int x = 0; x < this->grid_size; ++x) {
        for(int y = 0; y < this->grid_size; ++y) {
            t_vec3 current = this->c_pnts[ CLAMP(xy_to_idx(x, y, this->grid_size), 0, this->n_cpnts)].pos;
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
                "-tag %lxPOINTS\n",
                canvas, x1, y1, x2, y2, this);
    }
}

static void calibrate_tilde_erase(t_calibrate_tilde *this, t_glist *glist)
{
    t_canvas *canvas = glist_getcanvas(glist);
    sys_vgui(".x%lx.c delete %lxBASE\n", canvas, this);
    sys_vgui(".x%lx.c delete %lxPOINTS\n", canvas, this);
    sys_vgui(".x%lx.c delete %lxLINES\n", canvas, this);
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
    post("mouse released?");
}

static void calibrate_tilde_motion(t_calibrate_tilde *this, t_floatarg dx, t_floatarg dy) {
    this->mousex += (int)(dx/this->zoom);
    this->mousey -= (int)(dy/this->zoom);
    post("mousex %d, mousey %d", this->mousex, this->mousey);
}
static int calibrate_tilde_click(t_gobj *z, struct _glist *glist, int xpix, int ypix, int shift, int alt, int dbl, int doit) {
    dbl = shift = alt = 0;
    t_calibrate_tilde *this = (t_calibrate_tilde *)z;

    int xpos = text_xpix(&this->obj, glist);
    int ypos = text_ypix(&this->obj, glist);

    this->mousex = (xpix - xpos) / this->zoom;
    this->mousey = this->size - (ypix - ypos) / this->zoom;

    if(doit) { // same as if(mousePressed) in processing
        for(int i = 0; i < this->n_cpnts; ++i) {
            control_point_update_hover(&this->c_pnts[i], this->mousex, this->mousey);
        }
        for(int i = 0; i < this->n_cpnts; ++i) {
            control_point_update_pos(&this->c_pnts[i], this->mousex, this->mousey);
            //control_point_update_scale();
            //control_point_distort();
        }
    } else { // any reason to use this else?
        // for(int i = 0; i < this->n_cpnts; ++i) {
        //     control_point_update_hover(&this->c_pnts[i], this->mousex, this->mousey);
        // }
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
    this->size = 150;
    this->grid_size = 5;
    this->edge_offset = 10;
    this->n_cpnts = this->grid_size*this->grid_size;
    this->mousex = this->mousey = 0;
    this->zoom = this->glist->gl_zoom;

    // allocate memory for the control points and fill the array
    this->c_pnts = (t_control_point*)getbytes(this->n_cpnts*sizeof(t_control_point));
    for(int x = 0; x < this->grid_size; ++x) {
        for(int y = 0; y < this->grid_size; ++y) {
            int i = xy_to_idx(x, y, this->grid_size);
            t_vec3 pos = vec3(map_lin(x, 0, this->grid_size-1, this->edge_offset, this->size-this->edge_offset, false),
                              map_lin(y, 0, this->grid_size-1, this->edge_offset, this->size-this->edge_offset, false), 0);

            int type = control_point_type(i, this->grid_size);
            this->c_pnts[i] = control_point_new(type, pos);
        }
    }

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
