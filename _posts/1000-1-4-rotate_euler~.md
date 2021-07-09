---
category: Transformations
url_path: null
title: 'rotate_euler~'
type: 'SIGNAL'
layout: null
---

### Rotate a 3D vector
**[rotate_euler~]** is osci's basic rotation object. It performs an euler rotation using degrees. The first 3 inlets are the vector to be rotated, and the next 3 are the angles of rotation in degrees. Be careful of gimbal lock, if two axes line up then the third will have no effect. An optional message can be used to set the angle mode to degrees or radians.

- [more about gimbal lock](https://en.wikipedia.org/wiki/Gimbal_lock)
- [more about euler angles](https://en.wikipedia.org/wiki/Euler_angles)

### Inlets

| Inlet | Data Type    | Use     |
|:------|:-------------|:--------|
| 1     | float/signal | x       |
| 2     | float/signal | y       |
| 3     | float/signal | z       |
| 4     | float/signal | x angle |
| 5     | float/signal | y angle |
| 6     | float/signal | z angle |

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
| 1   | float     | x angle |
| 2   | float     | y angle |
| 3   | float     | z angle |

### Alias

rotate1~
