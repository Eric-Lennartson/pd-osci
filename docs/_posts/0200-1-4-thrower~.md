---
category: Send and Receive
url_path: null
title: 'thrower~'
type: 'SIGNAL'
layout: null
---

### Many to one 3D vector send

**[thrower~]** is a wrapper for pd's built in **[throw~]**. Internally, it creates three unique catchs based on the symbol argument provided on creation. For example, foo-x foo-y foo-z. Any number of **[thrower~]** objects can add into one **[catcher~]** object. But each **[catcher~]** must have a unique name.

### Inlets

| Outlet | Data Type | Return |
|:-------|:----------|:-------|
| 1      | signal    | x      |
| 2      | signal    | y      |
| 3      | signal    | z      |

### Outlets

none

### Arguments

| Arg | Data Type | Use              |
|:----|:----------|:-----------------|
| 1   | symbol    | name to throw to |


### Alias 

none
