#include "text~.h" // contains vec3, and m_pd
#include <string.h> // for strcmp and strncpy
// for now all that will happen is that it takes a symbol message and sends it out as audio.


/*  Goals

- 1. Figure out what's going on with alignments
- 1b. Potentially change it be called justify?
- 2. Fonts will be a fair amount of work, I'll work with them one day over the rainbow...
- space => _ newline => .
 
*/
static t_class *text_tilde_class;

typedef struct _text_tilde
{
    t_object x_obj;
    t_sample f; // dummy variable for 1st inlet
    OsciText text;
    t_vec3 v_out;
    
    t_inlet *line_height_in, *letter_spacing_in, *ratio_in; // *driver_in default provided
    t_outlet *yChan_out; // *xChan_out default provided
} t_text_tilde;

static t_int *text_tilde_perform(t_int *w)
{
    t_text_tilde *x             = (t_text_tilde *)(w[1]);
    t_sample *driver_in         =     (t_sample *)(w[2]);
    t_sample *line_height_in    =     (t_sample *)(w[3]);
    t_sample *letter_spacing_in =     (t_sample *)(w[4]);
    t_sample *ratio_in          =     (t_sample *)(w[5]);
    t_sample *xChan_out         =     (t_sample *)(w[6]);
    t_sample *yChan_out         =     (t_sample *)(w[7]);
    int     nblock              =            (int)(w[8]); // get blocksize
    
    
    while (nblock--) // dsp here
    {
        t_float t = driver_in[nblock];
        t_float line_height = line_height_in[nblock];
        t_float letter_spacing = letter_spacing_in[nblock];
        t_float ratio = ratio_in[nblock];
        
        if( letter_spacing < 0.f) { letter_spacing = 0.f; }
        if( ratio < 0.3f) { ratio = 0.3f; }
        
        x->v_out = gen(&x->text, t, line_height, letter_spacing, ratio);
        
        xChan_out[nblock] = x->v_out.x;
        yChan_out[nblock] = x->v_out.y;
    }
    
    return (w + 9); // point to the end of the signal vector, #ptrs + 1
}

static void text_tilde_dsp(t_text_tilde *x, t_signal **sp)
{
    dsp_add(text_tilde_perform, 8, x,
            sp[0]->s_vec, // driver
            sp[1]->s_vec, // line_height
            sp[2]->s_vec, // letter_spacing
            sp[3]->s_vec, // ratio
            sp[4]->s_vec, // xOut
            sp[5]->s_vec, // yOut
            sp[0]->s_n // block size
            );
}

static void onSetMsg(t_text_tilde *x,t_symbol *s)
{    
    char temp[100];
    if( strlen(s->s_name) > 100 ) {
        strncpy(temp, s->s_name, 100);
        setText(&x->text, temp);
        pd_error(x, "Warning: [text~] can only take up to 100 characters at a time. Characters over 100 have been removed.");
    }
    else
        setText(&x->text, s->s_name);
}

static void onhAlignMsg(t_text_tilde *x, t_symbol *s)
{
    if( !strcmp(s->s_name, "default") ) // strcmp returns 0 if true...
        set_horz_align(&x->text, Default);
    else if( !strcmp(s->s_name, "left") )
        set_horz_align(&x->text, Left);
    else if( !strcmp(s->s_name, "right") )
        set_horz_align(&x->text, Right);
    else if( !strcmp(s->s_name, "center") )
        set_horz_align(&x->text, Center);
    else
        pd_error(x, "[text~] has 4 horizontal alignment options: Default, Left, Right, Center");
}

static void onvAlignMsg(t_text_tilde *x,t_symbol *s)
{
    if( !strcmp(s->s_name, "none") )  // strcmp returns 0 if true...
        set_vert_align(&x->text, None);
    else if( !strcmp(s->s_name, "top") )
        set_vert_align(&x->text, Top);
    else if( !strcmp(s->s_name, "bottom") )
        set_vert_align(&x->text, Bottom);
    else if( !strcmp(s->s_name, "middle") )
        set_vert_align(&x->text, Middle);
    else
        pd_error(x, "[text~] has 4 vertical alignment options: None, Top, Bottom, Middle");
}

static void onFontMsg(t_text_tilde *x,t_symbol *s)
{
    // takes a font and adjusts the OsciText to use that font instead.
}

// ctor
static void *text_tilde_new(t_symbol *s, t_floatarg line_height, t_floatarg letter_spacing, t_floatarg ratio)
{
    t_text_tilde *x = (t_text_tilde *)pd_new(text_tilde_class);
    
    // bounds checking
    line_height = (line_height == 0) ? 1.5 : line_height;
    letter_spacing = (letter_spacing == 0) ? 0.25 : letter_spacing;
    ratio = (ratio == 0) ? 0.75 : ratio;
    
    x->text = NEW_OSCITEXT;
    x->v_out = NEW_VEC3;
    setText(&x->text, s->s_name);
    
    // allocate memory
    x->line_height_in = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    x->letter_spacing_in = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    x->ratio_in = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    
    // assign creation arguments
    pd_float((t_pd*)x->line_height_in, line_height);
    pd_float((t_pd*)x->letter_spacing_in, letter_spacing);
    pd_float((t_pd*)x->ratio_in, ratio);
    
    
    outlet_new(&x->x_obj, &s_signal); // default provided outlet
    x->yChan_out = outlet_new(&x->x_obj, &s_signal); // outlet we made
    
    return (x);
}

// dtor
static void *text_tilde_free(t_text_tilde *x)
{
    outlet_free(x->yChan_out);
    
    return (void *)x;
}

// NEVER MAKE THIS STATIC!!!
void text_tilde_setup(void)
{
    text_tilde_class = class_new(gensym("text~"),
                            (t_newmethod)text_tilde_new, //ctor
                            (t_method)text_tilde_free, //dtor
                            sizeof(t_text_tilde), // data space
                            CLASS_DEFAULT, // gui apperance
                            A_DEFSYM, // the text
                            A_DEFFLOAT, // line_height
                            A_DEFFLOAT, // letter_spacing
                            A_DEFFLOAT, // ratio
                            0); // no more args
    
    class_sethelpsymbol(text_tilde_class, gensym("text~")); // links to the help patch
    
    class_addmethod(text_tilde_class, (t_method)onSetMsg, gensym("set"), A_DEFSYM, 0);
    class_addmethod(text_tilde_class, (t_method)onhAlignMsg, gensym("halign"), A_DEFSYM, 0);
    class_addmethod(text_tilde_class, (t_method)onvAlignMsg, gensym("valign"), A_DEFSYM, 0);
    class_addmethod(text_tilde_class, (t_method)onFontMsg, gensym("font"), A_DEFSYM, 0);
    
    class_addmethod(text_tilde_class, (t_method)text_tilde_dsp, gensym("dsp"), A_CANT, 0); // add a dsp method to data space
    CLASS_MAINSIGNALIN(text_tilde_class, t_text_tilde, f); // signal inlet as first inlet
}
