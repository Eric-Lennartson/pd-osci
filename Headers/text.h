#ifndef text_h
#define text_h

#include "vec3.h"
#include "Audio_Math.h"
#include "stdbool.h"

#define FLT_EPSILON 1.19209290E-07F

#define clamp(val,min,max) ((val) < (min) ? (min) : ((val > max) ? (max) : (val)))
#define MAX(x,y) (((x) > (y)) ? (x) : (y))

// assigning numbers isn't strictly necessary here
typedef enum Align { Default = 0, Left = 1, Right = 2, Center = 3 } Align;
typedef enum vAlign { None = 0, Top = 1, Bottom = 2, Middle = 3 } vAlign;

typedef struct OsciText // add in alignment variables
{
    const char* text;
    int length;
    int numSpaces;
    int numLines;
    int longest_line;
    Align horz_align;
    vAlign vert_align;
    int text_map[100]; // TODO: use some template crazyness instead
    int letter_pos[100]; // templates don't exist in c land. Guess this doesn't need to be changed?
    int letter_line[100]; // "
    int line_length[100]; // "
    
} OsciText;
#define NEW_OSCITEXT (OsciText){NULL, 0, 0, 0, 0, Default, None, {0}, {0}, {0}, {0} }

t_float scale_lin(t_float value, t_float inputMin, t_float inputMax, t_float outputMin, t_float outputMax, bool clamp);
t_float tri(t_float value);
t_float tri_skew(t_float value, t_float skew);
//======================LETTERS===========================
t_vec3 letter_A(float t);
t_vec3 letter_B(float t);
t_vec3 letter_C(float t);
t_vec3 letter_D(float t);
t_vec3 letter_E(float t);
t_vec3 letter_F(float t);
t_vec3 letter_G(float t);
t_vec3 letter_H(float t);
t_vec3 letter_I(float t);
t_vec3 letter_J(float t);
t_vec3 letter_K(float t);
t_vec3 letter_L(float t);
t_vec3 letter_M(float t);
t_vec3 letter_N(float t);
t_vec3 letter_O(float t);
t_vec3 letter_P(float t);
t_vec3 letter_Q(float t);
t_vec3 letter_R(float t);
t_vec3 letter_S(float t);
t_vec3 letter_T(float t);
t_vec3 letter_U(float t);
t_vec3 letter_V(float t);
t_vec3 letter_W(float t); 
t_vec3 letter_X(float t); 
t_vec3 letter_Y(float t);
t_vec3 letter_Z(float t);

//======================NUMBERS===========================
t_vec3 number_0(float t);
t_vec3 number_1(float t);
t_vec3 number_2(float t);
t_vec3 number_3(float t);
t_vec3 number_4(float t);
t_vec3 number_5(float t);
t_vec3 number_6(float t);
t_vec3 number_7(float t);
t_vec3 number_8(float t);
t_vec3 number_9(float t);

t_vec3 drawLetter(char letter, float t);

// These were originally private functions used to for various helpful things regarding text

// length of string
static int len(OsciText *o_text);
// count spaces given some text
static int count_spaces(OsciText *o_text);
//count the lines given some text
static int count_lines(OsciText *o_text);
void positions(OsciText *o_text, const char *text, int *map, int *pos, int *line, int *line_length);
static inline int pos(OsciText *o_text, int i);
static inline int line(OsciText *o_text, int i);
static inline int map(OsciText *o_text, int i);
static inline int longestLine(OsciText *o_text);
static inline int lineLength(OsciText *o_text, int i);
static inline void set_horz_align(OsciText *o_text, Align alignment);
static inline void set_vert_align(OsciText *o_text, vAlign alignment);
static void setText(OsciText *o_text, const char *text);

//============== Gen Function

t_vec3 gen(OsciText* o_text, t_float t, t_float line_height, t_float letter_spacing, t_float ratio);


#endif /* text_h */
