---
category: Glue
url_path: null
title: 'project~'
type: 'SIGNAL'
layout: null
---

### Project from N to M dimensions

WARNING: this object currently doesn't do anything. 

some features I might include in the future:
- flag for projection mode
- proj~ alias

### Inlets

| Inlet | Data Type    | Use                     |
|:------|:-------------|:------------------------|
| 1     | float/signal | components of Rn vector |

### Outlets

| Outlet | Data Type | Return                  |
|:-------|:----------|:------------------------|
| 1      | signal    | components of Rm vector |

### Arguments

| Arg | Data Type | Use          |
|:----|:----------|:-------------|
| 1   | float     | N dimensions |
| 2   | float     | M dimensions |

### Alias 

proj~
