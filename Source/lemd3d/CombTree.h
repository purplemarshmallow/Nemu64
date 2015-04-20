#ifndef COMBINERTREEINCLUDED
#define COMBINERTREEINCLUDED

typedef enum CombinerOps
{
	add,
	sub,
	mul,
	lerp,
	mad,
	secondpass  // use if combinedalpha is used in second pass of color combiner only
} _CombinerOps;

typedef enum CombinerSource
{
	zero,
	one,
	tex0,
	tex0alpha,
	tex1,
	tex1alpha,
	constant,
	diffuse,
	diffusealpha,
	combinedalpha
} _CombinerSource;

typedef struct node{
	bool bIsFormula;
	struct {
		CombinerOps Op;
		node *a;
		node *b;
		node *factor;
	} formula;
	
	struct {
		CombinerSource Source;
		__int32 Constant;
	} value;
} _node;

class combtree
{
	void destroy_tree(node *leaf);
	void Optimize(node *leaf);
	void getstring(char *s, node *leaf);
	bool compare_trees(node *tree1, node *tree2);
	void CopyNodeWithSubelements(node **target, node *source);
	void CopyNodeData(node *target, node *source);
	void getpixelshader(char *s, node *leaf, int iAlternateIndex, bool bAlphaMode, bool alphaphase1, bool *bUseTex1, bool *bUseTex2);
	node *findsidechain(node *leaf, node *found);
	node *Sidechain;  // used for output to pixelshader
	public:
		node *root;
		combtree(CombinerSource FirstSource, __int32 FirstConstant);
		combtree(node *anothertree);
		~combtree();
		void AddOpTop(CombinerOps Op, CombinerSource ValueForB);
		void AddOpTop(CombinerOps Op, CombinerSource ValueForB, __int32 ConstantB);
		void AddSecondPassCommand();
		void destroy_tree();
		void Optimize();
		void getstring(char *s);
		void AddNodeTop(CombinerOps Op, bool bRight, node *secondtree);
		void getpixelshader(char *s, bool alphamode, bool alphaphase1, bool *bUseTex1, bool *bUseTex2);
};


#endif