// Code ported from Hansi Raber's RemappingTree
// Created by Hansi on 19.08.17.
// Copyright © 2017-18 hansi raber. All rights reserved.

#ifndef __PHASE_MAP_H
#define __PHASE_MAP_H

#include "Audio_Math.h" // mod1, map
#include "stdbool.h"
#include <stdlib.h> // malloc and friends

// Private structures for that file should go in the .c file, 
// with a declaration in the .h file if they are used by any functions in the .h
// Public structures should go in the .h file.

//might have errors with this forward declaration
typedef struct _node t_node;

t_node node(t_node* parent, t_node* left, t_node* right, bool isLeaf, float value);

typedef struct _p_map
{
	int LEN;
	int n_nodes;
	int depth;
	t_node* root;
	t_node* nodes;
} t_pmap;

#define NEW_PMAP (t_pmap){0,0,0,NULL,NULL}

int num_nodes(int N);
int tree_depth(int N);
// init pmap, given a length
t_pmap pmap(int len);
// Remaps the time `t` according to the applied weights
float remap(t_pmap* pmap, float t);
//Sets the weight for a particular index. index is 0..N-1
void setWeightWithIndex(t_pmap* pmap, int index, float w );
//Sets the weight for a particular time `t`. t must be 0 (inclusive) to 1 (exclusive)
void setWeightWithT(t_pmap* pmap, float t, float w);
// is this how I should be doing it?
void free_pmap(t_pmap* pmap);

#endif /* __PHASE_MAP_H */
