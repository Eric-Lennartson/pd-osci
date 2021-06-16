---
category: Send and Receive
url_path: null
title: 'catcher~'
type: 'SIGNAL'
layout: null
---

### Many to one 3D vector recieve

**[catcher~]** is a wrapper for pd's built in **[catch~]**. Internally, it creates three unique catchs based on the symbol argument provided on creation. For example, foo-x foo-y foo-z. Any number of **[thrower~]** objects can add into one **[catcher~]** object. But each **[catcher~]** must have a unique name.

### Inlets

none

### Outlets

| Outlet | Data Type | Return |
|:-------|:----------|:-------|
| 1      | signal    | x      |
| 2      | signal    | y      |
| 3      | signal    | z      |

### Arguments

| Arg | Data Type | Use           |
|:----|:----------|:--------------|
| 1   | symbol    | name to catch |
