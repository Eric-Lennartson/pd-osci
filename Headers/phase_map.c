#include "Phase_Map.h"

typedef struct _node
{
	t_node* parent;
	t_node* left;
	t_node* right;
	bool isLeaf;
	float value;
} t_node;

#define NEW_NODE (t_node){NULL, NULL, NULL, false, 0}

t_node node(t_node* parent, t_node* left, t_node* right, bool isLeaf, float value) 
{ 
	return (t_node){parent, left, right, isLeaf, value};
}

int num_nodes(int N)
{
    if(N > 1)
        return N + num_nodes(N/2 + (N%2 == 0 ? 0 : 1));
    else
        return 1;
}

int tree_depth(int N)
{
    if(N > 1)
        return 1 + tree_depth(N/2 + (N%2 == 0 ? 0 : 1));
    else
        return 1;
}

t_pmap pmap(int len)
{
	int n_nodes = num_nodes(len);
	int depth = tree_depth(len);
	t_node* root = NULL;
	t_node* nodes = (t_node*)malloc(sizeof(t_node) * n_nodes);	

    for(int i = 0; i < n_nodes; ++i) {
        nodes[i] = NEW_NODE;
    }

	int numElements = len;
	int offset = 0;
	for(int j = 0; j < depth; j++)
	{
		int nextOffset = offset + numElements;
		for( int i = 0; i < numElements; i++)
		{   // the first time through we get the first node
			t_node* node = &nodes[offset+i]; // nodes has n_nodes elements
			
			// If you're the first node set isLeaf to true
			if(j == 0) { node->isLeaf = true; }
			
			//If j is not right before the end
			if(j < depth-1)
			{
				node->parent = &nodes[nextOffset + i/2];
				if(i % 2 == 0)
					node->parent->left = node;
				else
					node->parent->right = node;
			}
			else
			{ // j is at the end
				// parent is NULL by default, but we're being explicit about it
				node->parent = NULL;
				root = node;
			}
			
			if(node->isLeaf)
			{ // if you're a leaf then your value is 1
				node->value = 1;
			}
			else
			{ 	// you're not a leaf and your value is the left + the right's value
				// right may not exist!
				node->value = node->left->value + ( node->right ? node->right->value : 0);
			}
		}
		offset = nextOffset;
		numElements = numElements / 2 + (numElements % 2); // reduce numElements because we're going 'up' in depth
	}
	return (t_pmap){len, n_nodes, depth, root, nodes};
}	
	
// Remaps the time `t` according to the applied weights
float remap(t_pmap* pmap, float t)
{
    t = mod1(t);
    float sum = pmap->root->value; // get value of the root
    float wantT = t*sum; // the T that we're looking for
    
	// are parens redundant here?
    t_node* node = pmap->root; // point to the root
    
    while(!node->isLeaf) // while it's NOT a leaf
    {
        // if the value of the node to the left is >= wantT OR
        // the node to the right is NOT NULL
        if(node->left->value >= wantT || !node->right) 
            node = node->left; // go left
        else
        {	// subtract wantT from the left node's value
            // and then go right
            wantT -= node->left->value;
            node = node->right;
        }
    }
    
    // aha, we arrived at a node!
    // and it's a leaf!
    
    // how far is the node we're on from the
    // some value in the nodes array... ???
    int i1 = (int)(node - pmap->nodes);
    int i2 = i1 + 1; // we add one because?
    float t1 = i1 / (float)pmap->LEN;
    float t2 = i2 / (float)pmap->LEN;
    return mod1( map(wantT, 0, node->value, t1, t2) );
}

//Sets the weight for a particular index. index is 0..N-1
void setWeightWithIndex(t_pmap* pmap, int index, float w )
{
	if(index < 0 || index >= pmap->LEN) return;
	pmap->nodes[index].value = w;
	
	// now propagate to the top
	t_node * node = &(pmap->nodes[index]);
	while(node->parent)
	{
		if(node->parent->left == node) //right may not exist!
			node->parent->value = node->value + ( node->parent->right ? node->parent->right->value : 0);
		else //left must always exist
			node->parent->value = node->value + node->parent->left->value;
		node = node->parent;
	}
}
	
//Sets the weight for a particular time `t`. t must be 0 (inclusive) to 1 (exclusive)
void setWeightWithT(t_pmap* pmap, float t, float w)
{
	if(t < 0 || t >= 1) return;
	int index = t * pmap->LEN;
	setWeightWithIndex(pmap, index, w);
}

// is this how I should be doing it?
void free_pmap(t_pmap* pmap) { free(pmap->nodes); pmap->nodes = NULL; }
