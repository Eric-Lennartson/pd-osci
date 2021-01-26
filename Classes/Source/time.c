/*
 * time:: gets the current time from the system
 *
 * (c) 1999-2011 IOhannes m zmölnig, forum::für::umläute, institute of electronic music and acoustics (iem)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * 1506:forum::für::umläute:2003: use timeb only if needed (like on windoze)
 */
#include "m_pd.h"

#if (defined __WIN32__)
# if (defined __i386__) && (defined __MINGW32__)
/* unless compiling under mingw/32bit, we want USE_TIMEB in redmond-land */
# else
#  define USE_TIMEB
# endif
#endif

#ifdef __APPLE__
#include <sys/types.h>
/* typedef     _BSD_TIME_T_    time_t;                */
#endif


#include <time.h>

#ifdef USE_TIMEB
#include <sys/timeb.h>
#else
#include <sys/time.h>
#endif


/* ----------------------- time --------------------- */

static t_class *time_class=NULL;

typedef struct _time {
  t_object x_obj;

  int GMT;

  t_outlet *x_outlet1;
  t_outlet *x_outlet2;
  t_outlet *x_outlet3;
  t_outlet *x_outlet4;
} t_time;

static void *time_new(t_floatarg f)
{
  t_time *x = (t_time *)pd_new(time_class);

  x->GMT = ((int)f != 0) ? 1 : 0;

  x->x_outlet1 = outlet_new(&x->x_obj, gensym("float"));
  x->x_outlet2 = outlet_new(&x->x_obj, gensym("float"));
  x->x_outlet3 = outlet_new(&x->x_obj, gensym("float"));
  x->x_outlet4 = outlet_new(&x->x_obj, gensym("float"));

  return (x);
}

static void time_bang(t_time *x)
{
  struct tm *resolvetime;
  t_float  ms = 0.f;
#ifdef USE_TIMEB
  struct timeb mytime;
  ftime(&mytime);
  resolvetime = (x->GMT)?gmtime(&mytime.time):localtime(&mytime.time);
  ms=mytime.millitm;
#else
  struct timeval tv;
  gettimeofday(&tv, 0);
  resolvetime = (x->GMT)?gmtime(&tv.tv_sec):localtime(&tv.tv_sec);
  ms = tv.tv_usec*0.001;
#endif
  outlet_float(x->x_outlet4, (t_float)(ms));
  outlet_float(x->x_outlet3, (t_float)resolvetime->tm_sec);
  outlet_float(x->x_outlet2, (t_float)resolvetime->tm_min);
  outlet_float(x->x_outlet1, (t_float)resolvetime->tm_hour);
}

void time_setup(void)
{
    time_class = class_new(gensym("time"),
                            (t_newmethod)time_new, //ctor
                            0, //dtor
                            sizeof(t_time), // data space
                            CLASS_DEFAULT, // gui apperance
                            A_DEFFLOAT, // gmt or not
                            0); // no more args
    
    class_sethelpsymbol(time_class, gensym("date")); // date and time will have the same help patch

    class_addbang(time_class, time_bang);
}
