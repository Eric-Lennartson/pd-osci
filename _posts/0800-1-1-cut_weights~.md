---
category: Phase Cutting
url_path: null
title: 'cut_weights~'
type: 'SIGNAL'
layout: null
---

### Split phase into N cuts based on weights

**[cut_weights~]** cuts a phase up into parts based on weights. **[cut_weights~]** must be the first modifier in the chain as it becomes the new intial phasor in order to draw multiple objects on the scope. Each output should also be sent to **[cut_idx~]** so that particular part of the phase is turned off when the cut has moved on to another thing it is drawing on the screen. 

### Inlets

| Inlet | Data Type    | Use           |
|:------|:-------------|:--------------|
| 1     | signal       | phase         |
| 2-N   | float/signal | weights (0-1) |

### Outlets

| Outlet | Data Type | Return         |
|:-------|:----------|:---------------|
| 2-N    | signal    | adjusted phase |

### Arguments

| Arg | Data Type | Use            |
|:----|:----------|:---------------|
| 1   | int       | number of cuts |

