---
category: Miscellaneous
url_path: null
title: 'splitter~'
type: 'SIGNAL'
layout: null
---

### Split a 3D vector

**[splitter~]** splits an image by summing it with a square wave. The frequencies for each vector can be linked to one specific frequency, or be set individually.

### Inlets

| Inlet | Data Type | Use                                 |
|:------|:----------|:------------------------------------|
| 1     | signal    | x frequency                         |
| 2     | signal    | y frequency                         |
| 3     | signal    | z frequency                         |
| 4-6   | signal    | x pulse width, x minimum, x maximum |
| 7-9   | signal    | y pulse width, y minimum, y maximum |
| 10-12 | signal    | z pulse width, z minimum, z maximum |

### Outlets

| Outlet | Data Type | Return |
|:-------|:----------|:-------|
| 1      | signal    | x      |
| 2      | signal    | y      |
| 3      | signal    | z      |

### Arguments

none

