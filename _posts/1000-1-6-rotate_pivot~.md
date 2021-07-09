---
category: Transformations
url_path: null
title: 'rotate_pivot~'
type: 'SIGNAL'
layout: null
---

### Rotate a 3D vector around a pivot and an axis

- [more about gimbal lock](https://en.wikipedia.org/wiki/Gimbal_lock)
- [more about euler angles](https://en.wikipedia.org/wiki/Euler_angles)

### Inlets

| Inlet | Data Type    | Use     |
|:------|:-------------|:--------|
| 1     | float/signal | x       |
| 2     | float/signal | y       |
| 3     | float/signal | z       |
| 4     | float/signal | angle   |
| 5     | float/signal | pivot x |
| 6     | float/signal | pivot y |
| 7     | float/signal | pivot z |
| 8     | float/signal | axis x  |
| 9     | float/signal | axis y  |
| 10    | float/signal | axis z  |

### Outlets

| Outlet | Data Type | Return |
|:-------|:----------|:-------|
| 1      | signal    | x      |
| 2      | signal    | y      |
| 3      | signal    | z      |

### Messages

Optional messages to send to the first inlet. Messages take the format \<message\> \<symbol\>.

| Message | Use |
|:--------|:----|
| Mode    | Sets the angle mode to be used. Options are degrees or radians. |

### Arguments

| Arg | Data Type | Use     |
|:----|:----------|:--------|
| 1   | float     | angle   |
| 2   | float     | axis x  |
| 3   | float     | axis y  |
| 4   | float     | axis z  |
| 5   | float     | pivot x |
| 6   | float     | pivot y |
| 7   | float     | pivot z |

### Alias

rotate3~
