---
category: Effects
url_path: null
title: 'chris_clip~'
type: 'SIGNAL'
layout: null
---

### Multi-Purpose Clipping

**[chris_clip~]** is a object based off of OsciStudio's modifier of the same name. In addition to normal clipping, it has a modulo option where the signal is wrapped from the minimum to the maximum. Mirror mode will mirror the clipped signal. When in mirror mode, only the maximum value will be considered for clipping. Mirror doesn't take values <= 0, and will instead stop clipping altogether. Both mod and mirror modes can be active at the same time. A gui version can be created by appending '-g'. **[chris_clip~-g]**

### Inlets

| Inlet | Data Type    | Use            |
|:------|:-------------|:---------------|
| 1     | signal       | signal to clip |
| 2     | float/signal | minimum value  |
| 3     | float/signal | maximum value  |

### Outlets

| Outlet | Data Type | Return        |
|:-------|:----------|:--------------|
| 1      | signal    | clipped value |

### Messages

Optional messages to send to the first inlet. Messages take the format \<message\> \<float\>. For example the "mod" message allows the user to toggle modulo clipping on or off. Using the correct format it might look something like this, [mod 1(.

| Message | Use                     |
|:--------|:------------------------|
| mod     | toggle modulo clipping. |
| mirror  | toggle mirror clipping. |

### Arguments

| Arg | Data Type | Use           |
|:----|:----------|:--------------|
| 1   | float     | minimum value |
| 2   | float     | maximum value |

### Alias 

none
