---
category: Transformations
url_path: null
title: 'rotate_axis~'
type: 'SIGNAL'
layout: null
---

### Rotate a 3D vector around an axis

**[rotate_axis~]** rotates an 3D vector around the specified axis. It also has optional messages to set the angle mode to degrees or radians.

- [more about gimbal lock](https://en.wikipedia.org/wiki/Gimbal_lock)
- [more about euler angles](https://en.wikipedia.org/wiki/Euler_angles)

### Inlets

| Inlet | Data Type    | Use     |
|:------|:-------------|:--------|
| 1     | float/signal | x       |
| 2     | float/signal | y       |
| 3     | float/signal | z       |
| 4     | float/signal | angle   |
| 5     | float/signal | axis x  |
| 6     | float/signal | axis y  |
| 7     | float/signal | axis z  |

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

| Arg | Data Type | Use               |
|:----|:----------|:------------------|
| 1   | float     | angle             |
| 2   | float     | axis x |
| 3   | float     | axis y |
| 4   | float     | axis z |

### Alias

rotate_a~
