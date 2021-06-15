---
category: Parametric Equations
url_path: null
title: 'hypotrochoid~'
type: 'SIGNAL'
layout: null
---

### Draws a hypotrochoid

A hypotrochoid is a roulette traced by a point attached to a circle of radius r rolling around the inside of a fixed circle of radius R, where the point is a distance d from the center of the interior circle. The classic Spirograph toy traces out hypotrochoid and epitrochoid curves.

- [more about hypotrochoids](https://en.wikipedia.org/wiki/Hypotrochoid)

### Inlets

| Inlet | Data Type    | Use                                |
|:------|:-------------|:-----------------------------------|
| 1     | signal       | phase                              |
| 2     | float/signal | radius (R) of exterior circle      |
| 3     | float/signal | radius (r) of interior circle      |
| 4     | float/signal | distance (d) from center of circle |


### Outlets

| Outlet | Data Type | Return |
|:-------|:----------|:-------|
| 1      | signal    | x      |
| 2      | signal    | y      |

### Arguments

| Arg | Data Type | Use          |
|:----|:----------|:-------------|
| 1   | float     | radius (R)   |
| 2   | float     | radius (r)   |
| 3   | float     | distance (d) |
