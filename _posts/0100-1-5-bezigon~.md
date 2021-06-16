---
category: Miscellaneous
url_path: null
title: 'bezigon~'
type: 'SIGNAL'
layout: null
---

### N-sided polygon with cubic interpolation

[bezigon~] creates a polygon with cubic interpolation. In addition to phase, the first inlet also accepts list messages of the form (index, x, y). These set a point to be added to the bezigon. If a point already exists at the current index, it is written over by the new point. 

### Inlets

| Inlet | Data Type    | Use                                        |
|:------|:-------------|:-------------------------------------------|
| 1     | signal/list  | phase, list (index, x, y)                  |
| 2     | float        | toggle to close or not close shape (0 or 1)|


### Outlets

| Outlet | Data Type | Return  |
|:-------|:----------|:--------|
| 1      | signal    | x       |
| 2      | signal    | y       |

### Arguments

| Arg | Data Type | Use                                         |
|:----|:----------|:--------------------------------------------|
| 1   | float     | number of points (minimum 3)                |
| 2   | float     | toggle to close or not close shape (0 or 1) |
