# pd-osci
A collection of useful externals for making oscilloscope music and art

--------------------------------------------------------------------------

   This work is free. You can redistribute it and/or modify it under the
   terms of the GNU General Public License v3.0. See LICENSE for more details

--------------------------------------------------------------------------

###   About osci

​	This version of osci needs **Pd 0.51-4* or above, download Pure Data from: http://msp.ucsd.edu/software.html

​	osci is a library for Pure Data that provides a system for creating oscilloscop music and art.

​	This project is mostly for myself, but I put it on github incase others wished to contribute.

​	I'm a amateur programmer, but I do take suggestions and will make an effort to implement them if I can.

​	This library's repository resides at <https://github.com/Eric-Lennartson/pd-osci/>.

--------------------------------------------------------------------------

### Downloading osci:

​	You can get osci from https://github.com/Eric-Lennartson/pd-osci/releases - where all releases are available, but osci is also found via Pd's external manager (In Pd, just go for Help => Find Externals and search for 'osci').  In any case, you should download the folder to a place Pd automatically searches for, and the common place is the ~/documents/pd/externals folder.

Some of the abstractions are dependent on objects from the ELSE library. ELSE can be installed either from Pd's external manager or from the command line. ELSE can be found at <https://github.com/porres/pd-else>.

​	Instructions on how to build osci are provided below.

--------------------------------------------------------------------------

#### Building osci for Pd Vanilla:

osci relies on the build system called "pd-lib-builder" by Katja Vetter (check the project in: <https://github.com/pure-data/pd-lib-builder>). PdLibBuilder tries to find the Pd source directory at several common locations, but when this fails, you have to specify the path yourself using the pdincludepath variable. Example:

<pre>make pdincludepath=~/pd-0.51-1/src/  (for Windows/MinGW add 'pdbinpath=~/pd-0.51-1/bin/)</pre>

* Installing with pdlibbuilder

Go to the pd-osci folder and use "objectsdir" to set a relative path for your build, something like:

<pre>make install objectsdir=../osci-build</pre>
Then move it to your preferred install folder for Pd and add it to the path.

Cross compiling is also possible with something like this

<pre>make CC=arm-linux-gnueabihf-gcc target.arch=arm7l install objectsdir=../</pre>

--------------------------------------------------------------------------

### Acknowledgements

Alexandre Torres Porres, Almost all of my Pd external coding knowledge comes from looking at the code for ELSE.

Hansi Raber, Most concepts and code relating to oscilloscopes comes from using/being inspired by OsciStudio.

Pd Community/Dev-list, For answering my questions which may or may not have made any sense at the time.

--------------------------------------------------------------------------

## Current Object list (61 objects):

**2D Primitives: [08]**

- [point~]
- [line~]
- [circle~]
- [ellipse~]
- [rectangle~]
- [square~]
- [triangle~]
- [polygon~]

**3D Primitives: [05]**

- [cuboid~]
- [dodecahedron~]
- [icosahedron~]
- [octahedron~]
- [tetrahedron~]

**Parameteric Equations: [04]**

- [heart~]
- [selipse~]
- [supershape~]
- [hypotrochoid~]

**Transformations: [04]**

- [rotate~]
- [scale~]
- [shear~]
- [translate~]

**Phase Manipulation: [04]**

- [bright~]
- [dash~]
- [knee~]
- [trace~]

**Phase Cutting: [04]**

- [cut_mix~]
- [cut_idx~]
- [cut_equal~]
- [cut_weights~]

**Effects: [03]**

- [ball~]
- [grid~]
- [chris_clip~]

**Glue: [04]**

- [lerp~]
- [ramp]
- [ramp~]
- [project~]

**Binary Operators: [09]**

- [==~]
- [!=~]
- [<~]
- [>~]
- [<=~]
- [>=~]
- [||]
- [&&]
- [%~]

**Value Mapping: [04]**

- [map]
- [map~]
- [skew]
- [skew~]

**Brightness Control: [01]**

- [gamma~]

**Signal send and receive: [04]**

- [thrower~]
- [catcher~]
- [snd~]
- [rcv~]

**Misc: [08]**

- [n~]
- [n]
- [text~]
- [bezier~]
- [bezigon~]
- [shroom~]
- [xy_splitter~]
- [out~]

--------------------------------------------------------------------------
