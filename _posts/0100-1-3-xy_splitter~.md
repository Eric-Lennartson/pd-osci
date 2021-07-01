---
category: Miscellaneous
url_path: null
title: 'xy_splitter~'
type: 'SIGNAL'
layout: null
---

Offset a 3D vector

### Inlets

| Inlet | Data Type    | Use      |
|:------|:-------------|:---------|
| 1     | float/signal | x input  |
| 2     | float/signal | y input  |
| 3     | float/signal | z input  |
| 4     | float/signal | x offset |
| 5     | float/signal | y offset |
| 6     | float/signal | z offset |

### Outlets

| Outlet | Data Type | Return       |
|:-------|:----------|:-------------|
| 1      | signal    | x translated |
| 2      | signal    | y translated |
| 3      | signal    | z translated |

### Arguments

| Arg | Data Type | Use      |
|:----|:----------|:---------|
| 1   | float     | x offset |
| 2   | float     | y offset |
| 3   | float     | z offset |
