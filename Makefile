############
# Makefile #
############

# library name
lib.name = osci

#add /Headers to the search path
cflags = -I Headers

A_Math  := Headers/audio_math.c
A_Funcs := Headers/audio_functions.c
cut     := Headers/phase_cut.c
utils   := Headers/utils.c
vec3    := Headers/vec3.c
text    := Headers/text.c

#########################################################################
# Sources: ##############################################################
#########################################################################

# control:
m_wrap.class.sources := Classes/Source/m_wrap.c
map.class.sources    := Classes/Source/map.c
skew.class.sources   := Classes/Source/skew.c

# I REALLY need to clean up my header files to make more sense
# is one giant header the solution?
# signal:
bezier~.class.sources       := Classes/Source/bezier~.c
bezigon~.class.sources      := Classes/Source/bezigon~.c
chris_clip~.class.sources   := Classes/Source/chris_clip~.c
circle~.class.sources       := Classes/Source/circle~.c 
ellipse~.class.sources      := Classes/Source/ellipse~.c 
grid~.class.sources         := Classes/Source/grid~.c  $(A_Math) $(utils) $(vec3)
heart~.class.sources        := Classes/Source/heart~.c $(A_Math)
lerp~.class.sources         := Classes/Source/lerp~.c
map~.class.sources          := Classes/Source/map~.c 
selipse~.class.sources      := Classes/Source/selipse~.c
skew~.class.sources         := Classes/Source/skew~.c 
supershape~.class.sources   := Classes/Source/supershape~.c
super~.class.sources        := Classes/Aliases/super~.c
translate~.class.sources    := Classes/Source/translate~.c
trans~.class.sources    	:= Classes/Aliases/trans~.c
ball~.class.sources         := Classes/Source/ball~.c $(A_Math) $(vec3)
bright~.class.sources       := Classes/Source/bright~.c $(A_Math)
cuboid~.class.sources       := Classes/Source/cuboid~.c $(A_Math) $(vec3)
dash~.class.sources         := Classes/Source/dash~.c $(A_Math)
dodecahedron~.class.sources := Classes/Source/dodecahedron~.c $(A_Math)
dodeca~.class.sources 		:= Classes/Aliases/dodeca~.c $(A_Math)
icosahedron~.class.sources  := Classes/Source/icosahedron~.c $(A_Math)
icosa~.class.sources  		:= Classes/Aliases/icosa~.c $(A_Math)
knee~.class.sources         := Classes/Source/knee~.c $(A_Math)
octahedron~.class.sources   := Classes/Source/octahedron~.c $(A_Math)
octa~.class.sources   		:= Classes/Aliases/octa~.c $(A_Math)
rectangle~.class.sources    := Classes/Source/rectangle~.c $(A_Math)
rect~.class.sources    		:= Classes/Aliases/rect~.c $(A_Math)
rotate~.class.sources       := Classes/Source/rotate~.c $(A_Math) $(vec3)
square~.class.sources       := Classes/Source/square~.c $(A_Math)
tetrahedron~.class.sources  := Classes/Source/tetrahedron~.c $(A_Math)
tetra~.class.sources  		:= Classes/Aliases/tetra~.c $(A_Math)
trace~.class.sources        := Classes/Source/trace~.c $(A_Math)
triangle~.class.sources     := Classes/Source/triangle~.c $(A_Math)
tri~.class.sources     		:= Classes/Aliases/tri~.c $(A_Math)
polygon~.class.sources      := Classes/Source/polygon~.c $(A_Funcs) $(cut)
gon~.class.sources      	:= Classes/Aliases/gon~.c $(A_Funcs) $(cut)
cut_equal~.class.sources    := Classes/Source/cut_equal~.c $(cut) $(utils)
cut_mix~.class.sources      := Classes/Source/cut_mix~.c $(cut) $(utils)
cut_weights~.class.sources  := Classes/Source/cut_weights~.c $(cut)
hypotrochoid~.class.sources := Classes/Source/hypotrochoid~.c $(utils)
scale~.class.sources        := Classes/Source/scale~.c $(vec3)
shear~.class.sources        := Classes/Source/shear~.c $(vec3)
text~.class.sources         := Classes/Source/text~.c $(text)

#########################################################################

# extra files

datafiles = \
$(wildcard Classes/Abstractions/*.pd) \
$(wildcard Help-files/*.pd) \
README.md

#########################################################################

# Directory where Pd API m_pd.h should be found, and other Pd header files.
PDINCLUDE_DIR=Headers/

# include Makefile.pdlibbuilder from submodule directory 'pd-lib-builder'
PDLIBBUILDER_DIR=pd-lib-builder/
include $(PDLIBBUILDER_DIR)/Makefile.pdlibbuilder