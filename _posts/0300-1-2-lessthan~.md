---
category: Binary Operators
url_path: null
title: '<~'
type: 'SIGNAL'
layout: null
---

### less than for signals

**[<~]** checks if the first inlet is less than the second inlet. 0 is returned for false, 1 is returned for true. If a float is provided as an argument, the second inlet will take floats instead of signals.

### Inlets

| Inlet | Data Type    | Use      |
|:------|:-------------|:---------|
| 1     | signal       | a        |
| 2     | float/signal | b        |

### Outlets

| Outlet | Data Type | Return    |
|:-------|:----------|:----------|
| 1      | signal    | 0 or 1    |

### Arguments

| Arg | Data Type | Use                  |
|:----|:----------|:---------------------|
| 1   | float     | value to compare to  |