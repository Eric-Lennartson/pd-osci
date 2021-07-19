---
category: Brightness Control
url_path: null
title: 'spotkiller~'
type: 'SIGNAL'
layout: null
---

### Set spot killing db level

**[spotkiller~]** is an optional brightness control object. It sets a cut off level, in decibels, where the brightness of an image will be set to 0. This can be useful for avoiding burn out in the center of your oscilloscope, vectrex, etc. It should come at the end of the brightness signal chain. Some scopes have brightness inverted, in which case, use the invert toggle on **[out~]**. In order to make use of this object, your scope must have a brightness input, otherwise there will not be anywhere to send the signal.

### Inlets

| Inlet | Data Type    | Use        |
|:------|:-------------|:-----------|
| 1     | signal       | x          |
| 2     | signal       | y          |
| 3     | signal       | z          |
| 3     | signal       | brightness |

### Outlets

| Outlet | Data Type | Return     |
|:-------|:----------|:-----------|
| 1      | signal    | brightness |

### Messages

Optional message to send to first inlet.

| Message | Use                                                             |
|:--------|:----------------------------------------------------------------|
| 2D      | set the expected incoming signal vector to be two dimensional   |
| 3D      | set the expected incoming signal vector to be three dimensional |

### Arguments

none
