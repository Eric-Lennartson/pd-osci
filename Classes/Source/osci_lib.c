/*
This will necessary in order to load up non alphanumeric objects
- Namely the binary operators I would like to add to remove dependencies
- This is also the place to load up the phase mapping stuff, as the order really matters there
- this could also result in the master header file?
- actually this allows me to print to the console, versioning and other important info
- if people wanted this in camomile, having this be the only binary would also be pretty awesome
*/

#include "m_pd.h"

t_class *osci_class; // not static?

typedef struct _osci
{
    t_object x_obj;
} t_osci;

static int printed = 0;


void osci_about(t_osci* x)
{
    //int major = 0, minor = 0, bugfix = 0;
    //sys_getversion(&major, &minor, &bugfix);
    post("");
    post("|---------------------------------------------------------|");
    post("| OSCI - Pure Data externals for oscilloscope music & art  ");
    post("|---------------------------------------------------------|");
    post("| Author: Eric Lennartson                                  ");
    post("| Version: 0.1.0 Released February 1st 2021                ");
    post("| Repository: https://github.com/Eric-Lennartson/pd-osci   ");
    post("| License: GPL-3.0                                         ");
    post("| Build Date: February 1st 2021                            ");
    //if(major >= min_major && minor >= min_minor && bugfix >= min_bugfix)
    //     post("- OSCI 1.0.-0 %s-%d needs at least Pd %d.%d-%d (you have %d.%d-%d, you're good!)",
    //          STATUS, status_number, min_major, min_minor, min_bugfix, major, minor, bugfix);
    // else
    //     pd_error(x, "- OSCI 1.0-0 %s-%d needs at least Pd %d.%d-%d (you have %d.%d-%d, please upgrade!)",
    //         STATUS, status_number, min_major, min_minor, min_bugfix, major, minor, bugfix);
    post("|---------------------------------------------------------|");
    post("");
}

static void* osci_new(void)
{
    t_osci *x = (t_osci*)pd_new(osci_class);

    if(!printed) 
    {
        osci_about(x);
        printed = 1;
    }
    return(x);
}

void osci_setup(void)
{
    osci_class = class_new( gensym("osci"),
                            (t_newmethod)osci_new,
                            NULL, // no dtor
                            sizeof(t_osci),
                            CLASS_NOINLET, // gui appearance
                            0); // no args

    t_osci *x = (t_osci*)pd_new(osci_class);
    if(!printed)
    {
        osci_about(x);
        printed = 1;
    }
}