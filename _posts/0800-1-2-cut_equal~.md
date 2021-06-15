---
category: Phase Cutting
url_path: null
title: 'cut_equal~'
type: 'SIGNAL'
layout: null
---

### Split phase into N equal cuts

**[cut_equal~]** splits a phasor up into equal parts to draw multiple images at the same time. **[cut_equal~]** is special in that it must be the first modifier in the chain as it becomes the new intial phase in order to draw multiple objects onto the scope. Each output should also be sent to **[cut_idx~]** so that particular part of the phase is turned off when the cut has moved on to another thing it is drawing on the screen. 

### Inlets

| Inlet | Data Type    | Use           |
|:------|:-------------|:--------------|
| 1     | signal       | phase         |

### Outlets

| Outlet | Data Type | Return         |
|:-------|:----------|:---------------|
| 1-N    | signal    | adjusted phase |

### Arguments

| Arg | Data Type | Use            |
|:----|:----------|:---------------|
| 1   | int       | number of cuts |
