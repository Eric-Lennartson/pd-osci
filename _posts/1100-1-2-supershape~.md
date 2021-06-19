---
category: Parametric Equations
url_path: null
title: 'supershape~'
type: 'SIGNAL'
layout: null
---

### Draw a supershape

[more about supershapes](http://paulbourke.net/geometry/supershape/)

### Inlets

| Inlet | Data Type    | Use                             |
|:------|:-------------|:--------------------------------|
| 1     | signal       | phase                           |
| 2     | float/signal | n1                              |
| 3     | float/signal | n2                              |
| 4     | float/signal | n3                              |
| 5     | float/signal | m, controls rotational symmetry |
| 6     | float/signal | a, controls height              |
| 7     | float/signal | b, controls width               |

### Outlets

| Outlet | Data Type | Return |
|:-------|:----------|:-------|
| 1      | signal    | x      |
| 2      | signal    | y      |

### Arguments

| Arg | Data Type | Use                             |
|:----|:----------|:--------------------------------|
| 1   | float     | n1                              |
| 2   | float     | n2                              |
| 3   | float     | n3                              |
| 4   | float     | m, controls rotational symmetry |
| 5   | float     | a, controls height              |
| 6   | float     | b, controls width               |

### Alias

super~