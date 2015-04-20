#include "stdafx.h"
#include "combtree.h"

combtree::combtree(CombinerSource FirstSource, __int32 FirstConstant)
{
	if(((FirstConstant==0) && (FirstSource==constant))){
		ShowError("Constant is zero!");
	}
	root = new node;
	root->bIsFormula = false;
	root->value.Source = FirstSource;
	root->value.Constant = FirstConstant;
}

combtree::combtree(node *anothertree)
{
	CopyNodeWithSubelements(&root, anothertree);
}

combtree::~combtree()
{
	destroy_tree();
}

void combtree::destroy_tree()
{
	destroy_tree(root);
}

void combtree::destroy_tree(node *leaf)
{
	if(leaf!=NULL){
		if (leaf->bIsFormula){
			destroy_tree(leaf->formula.a);
			leaf->formula.a = NULL;
			destroy_tree(leaf->formula.b);
			leaf->formula.b = NULL;
			destroy_tree(leaf->formula.factor);
			leaf->formula.factor = NULL;
		}
		delete leaf;
	}
}

void combtree::AddNodeTop(CombinerOps Op, bool bRight, node *secondtree)
{
	node *NewRoot;
	NewRoot = new node;
	NewRoot->bIsFormula = true;
	NewRoot->formula.Op = Op;
	if (bRight){
		NewRoot->formula.a = root;
		CopyNodeWithSubelements(&NewRoot->formula.b, secondtree);
	} else {
		CopyNodeWithSubelements(&NewRoot->formula.a, secondtree);
		NewRoot->formula.b = root;
	}
	NewRoot->formula.factor = NULL;
	root = NewRoot;
}

void combtree::AddSecondPassCommand()
{
	node *NewRoot;
	NewRoot = new node;
	NewRoot->bIsFormula = true;
	NewRoot->formula.Op = secondpass;
	NewRoot->formula.a = root;
	NewRoot->formula.b = NULL;
	NewRoot->formula.factor = NULL;
	root = NewRoot;
}

void combtree::AddOpTop(CombinerOps Op, CombinerSource ValueForB)
{
	AddOpTop(Op, ValueForB, 0);
}

void combtree::AddOpTop(CombinerOps Op, CombinerSource ValueForB, __int32 ConstantB)
{
	if(((ConstantB==0) && (ValueForB==constant))){
		ShowError("Constant is zero!");
	}
	node *NewRoot;
	NewRoot = new node;
	NewRoot->bIsFormula = true;
	NewRoot->formula.Op = Op;
	NewRoot->formula.a = root;
	NewRoot->formula.b = new node;
	NewRoot->formula.b->bIsFormula = false;
	NewRoot->formula.b->value.Source = ValueForB;
	NewRoot->formula.b->value.Constant = ConstantB;
	NewRoot->formula.factor = NULL;
	root = NewRoot;
}

void combtree::CopyNodeWithSubelements(node **target, node *source)
{
	// creates a new top node and copies all subnodes as new instances
	node *NewNode;
	if (source!=NULL){
		NewNode = new node;
		NewNode->bIsFormula = source->bIsFormula;
		if (NewNode->bIsFormula){
			NewNode->formula.Op = source->formula.Op;
			CopyNodeWithSubelements(&NewNode->formula.a, source->formula.a);
			CopyNodeWithSubelements(&NewNode->formula.b, source->formula.b);
			CopyNodeWithSubelements(&NewNode->formula.factor, source->formula.factor);
		} else {
			NewNode->value = source->value;
		}
	} else {
		NewNode = NULL;
	}
	(*target) = NewNode;
}

void combtree::CopyNodeData(node *target, node *source)
{
	// Copies only data of the node, nothing is created or destroyed
	target->bIsFormula = source->bIsFormula;
	target->formula = source->formula;
	target->value = source->value;
}

void combtree::Optimize(node *leaf)
{
	node *old;
	// For LERP Detection (lerpfirst - lerpsecond1) * factor + lerpsecond2
	bool bLERPed;
	bool bMADed;
	// First Level
	node *lerp1_mul;        // (lerpfirst - lerpsecond1) * factor
	node *lerp1_summand;    // lerpsecond2
	// Second Level
	node *lerp2_sub;        // (lerpfirst - lerpsecond1)
	node *lerp2_factor;     // factor
	// Third Level
	node *lerp3_minuend;    // lerpfirst
	node *lerp3_subtrahend; // lerpsecond1

	// For MAD (Multiply Add) Detection (madfirst * madsecond) + madthird
	// First Level
	node *mad1_mul;         // (madfirst * madsecond)
	node *mad1_summand;		// madthird
	// Second Level
	node *mad2_factor1;     // madfirst
	node *mad2_factor2;		// madsecond


	if (leaf!=NULL){
		if (leaf->bIsFormula){
			Optimize(leaf->formula.a);
			Optimize(leaf->formula.b);
			Optimize(leaf->formula.factor);
			switch (leaf->formula.Op){
			case add:
				if (0 && (!leaf->formula.a->bIsFormula) && (leaf->formula.a->value.Source==constant) &&
					(!leaf->formula.b->bIsFormula) && (leaf->formula.b->value.Source==constant)){
					// Add constants...
					leaf->bIsFormula = false;
					leaf->value.Source = constant;
					leaf->value.Constant = AddColor(leaf->formula.a->value.Constant, leaf->formula.b->value.Constant);
					delete leaf->formula.a;
					delete leaf->formula.b;
				} else {
					if ((!leaf->formula.a->bIsFormula) && (leaf->formula.a->value.Source==zero)){
						// 0+A
						old = leaf->formula.b;
						delete leaf->formula.a;
						CopyNodeData(leaf, old);
						delete old;
					} else {
						if ((!leaf->formula.b->bIsFormula) && (leaf->formula.b->value.Source==zero)){
							// A+0
							old = leaf->formula.a;
							delete leaf->formula.b;
							CopyNodeData(leaf, old);
							delete old;
						} else {
							// LERP Detection
							// Level 1: Split Addition
							lerp1_mul = NULL;
							lerp1_summand = NULL;
							lerp2_sub = NULL;
							lerp2_factor = NULL;
							lerp3_minuend = NULL;
							lerp3_subtrahend = NULL;
							bLERPed = false;
							if ((leaf->formula.a->bIsFormula) && (leaf->formula.a->formula.Op==mul)){
								lerp1_mul = leaf->formula.a;
								lerp1_summand = leaf->formula.b;
							} else {
								if ((leaf->formula.b->bIsFormula) && (leaf->formula.b->formula.Op==mul)){
									lerp1_mul = leaf->formula.b;
									lerp1_summand = leaf->formula.a;
								}
							}
							if ((lerp1_mul!=NULL) && (lerp1_summand!=NULL)){
								// Level 2: Split Multiplication
								if ((lerp1_mul->formula.a->bIsFormula) && (lerp1_mul->formula.a->formula.Op==sub)){
									lerp2_sub = lerp1_mul->formula.a;
									lerp2_factor = lerp1_mul->formula.b;
								} else {
									if ((lerp1_mul->formula.b->bIsFormula) && (lerp1_mul->formula.b->formula.Op==sub)){
										lerp2_sub = lerp1_mul->formula.b;
										lerp2_factor = lerp1_mul->formula.a;
									}
								}
								if ((lerp2_sub!=NULL) && (lerp2_factor!=NULL)){
									// Level 3: Split Subtraction
									lerp3_minuend = lerp2_sub->formula.a;
									lerp3_subtrahend = lerp2_sub->formula.b;

									if (compare_trees(lerp3_subtrahend, lerp1_summand)){
										// This is a LERP. Replace command by LERP and get rid of all the other pointers
										leaf->formula.Op = lerp;
										leaf->formula.a = lerp3_minuend;
										leaf->formula.b = lerp1_summand;   // no change actually
										leaf->formula.factor = lerp2_factor;
										bLERPed = true;
										
										// Now get rid of the pointers we don't need anymore
										delete lerp1_mul;
										delete lerp2_sub;
										destroy_tree(lerp3_subtrahend);
									}
								}
							}
							if (!bLERPed){
								// LERP not detected
								// Look for MAD
								bMADed = false;
								mad1_mul = NULL;
								mad1_summand = NULL;
								mad2_factor1 = NULL;
								mad2_factor2 = NULL;
								if ((leaf->formula.a->bIsFormula) && (leaf->formula.a->formula.Op==mul)){
									mad1_mul = leaf->formula.a;
									mad1_summand = leaf->formula.b;
								} else {
									if ((leaf->formula.b->bIsFormula) && (leaf->formula.b->formula.Op==mul)){
										mad1_mul = leaf->formula.b;
										mad1_summand = leaf->formula.a;
									}
								}
								if ((mad1_mul!=NULL) && (mad1_mul!=NULL)){
									mad2_factor1 = mad1_mul->formula.a;
									mad2_factor2 = mad1_mul->formula.b;
									leaf->formula.Op = mad;
									leaf->formula.a = mad2_factor1;
									leaf->formula.b = mad1_summand;   // no change actually
									leaf->formula.factor = mad2_factor2;

									// Now get rid of the pointers we don't need anymore
									delete mad1_mul;
									bMADed = true;
								}
							}
						}
					}
				}
				break;
			case sub:
				if (0&&(!leaf->formula.a->bIsFormula) && (leaf->formula.a->value.Source==constant) && (leaf->formula.a->value.Constant==0) &&
					(!leaf->formula.b->bIsFormula) && (leaf->formula.b->value.Source==constant) && (leaf->formula.b->value.Constant==1)){
					// PrimColor-EnvColor...not done yet
					leaf->bIsFormula = false;
					leaf->value.Source = constant;
					leaf->value.Constant = 7;
					delete leaf->formula.a;
					delete leaf->formula.b;
				} else {
					if ((!leaf->formula.b->bIsFormula) && (leaf->formula.b->value.Source==zero)){
						old = leaf->formula.a;
						delete leaf->formula.b;
						CopyNodeData(leaf, old);
						delete old;
					} else {
						if (compare_trees(leaf->formula.a, leaf->formula.b)){
							destroy_tree(leaf->formula.a);
							destroy_tree(leaf->formula.b);
							leaf->bIsFormula = false;
							leaf->value.Source = zero;
						}
					}
				}
				break;
			case mul:
				if (0&&(!leaf->formula.a->bIsFormula) && (leaf->formula.a->value.Source==constant) &&
					(!leaf->formula.b->bIsFormula) && (leaf->formula.b->value.Source==constant)){
					// Multiply constants...
					leaf->bIsFormula = false;
					leaf->value.Source = constant;
					leaf->value.Constant = MultColor(leaf->formula.a->value.Constant, leaf->formula.b->value.Constant);
					delete leaf->formula.a;
					delete leaf->formula.b;
				} else {
					if ((!leaf->formula.a->bIsFormula) && (leaf->formula.a->value.Source==one)){
						old = leaf->formula.b;
						delete leaf->formula.a;
						CopyNodeData(leaf, old);
						delete old;
					} else {
						if ((!leaf->formula.b->bIsFormula) && (leaf->formula.b->value.Source==one)){
							old = leaf->formula.a;
							delete leaf->formula.b;
							CopyNodeData(leaf, old);
							delete old;
						} else {
							if (((!leaf->formula.a->bIsFormula) && (leaf->formula.a->value.Source==zero)) || ((!leaf->formula.b->bIsFormula) && (leaf->formula.b->value.Source==zero))){
								destroy_tree(leaf->formula.a);
								destroy_tree(leaf->formula.b);
								leaf->bIsFormula = false;
								leaf->value.Source = zero;
							}
						}
					}
				}
				break;
			} 
		}
	}
}

void combtree::Optimize()
{
	Optimize(root);
}

void combtree::getstring(char *s, node *leaf)
{
	if (leaf->bIsFormula){
		if (leaf->formula.Op==secondpass){
			strcat(s, "seconpass(");
			getstring(s, leaf->formula.a);
			strcat(s, ")");
		} else {
			if (leaf->formula.Op==lerp){
				strcat(s, "LERP(");
				getstring(s, leaf->formula.a);
				strcat(s, ", ");
				getstring(s, leaf->formula.b);
				strcat(s, ", ");
				getstring(s, leaf->formula.factor);
				strcat(s, ")");
			} else {
				if (leaf->formula.Op==mad){
					strcat(s, "MAD(");
					getstring(s, leaf->formula.factor);
					strcat(s, ",");
					getstring(s, leaf->formula.a);
					strcat(s, ",");
					getstring(s, leaf->formula.b);
					strcat(s, ")");
				} else {
					strcat(s, "(");
					getstring(s, leaf->formula.a);
					switch (leaf->formula.Op){
					case add:
						strcat(s, "+");
						break;
					case sub:
						strcat(s, "-");
						break;
					case mul:
						strcat(s, "*");
						break;
					}
					getstring(s, leaf->formula.b);
					strcat(s, ")");
				}
			}
		}
	} else {
		switch (leaf->value.Source){
		case zero:
			strcat(s, "zero");
			break;
		case one:
			strcat(s, "one");
			break;
		case tex0:
			strcat(s, "tex0");
			break;
		case tex0alpha:
			strcat(s, "tex0alpha");
			break;
		case tex1:
			strcat(s, "tex1");
			break;
		case tex1alpha:
			strcat(s, "tex1alpha");
			break;
		case constant:
			char s2[100];
			sprintf(s2, "constant(%i)", leaf->value.Constant);
			strcat(s, s2);
			break;
		case diffuse:
			strcat(s, "diffuse");
			break;
		case diffusealpha:
			strcat(s, "diffusealpha");
			break;
		case combinedalpha:
			strcat(s, "combinedalpha");
			break;
		}
	}
}

void combtree::getstring(char *s)
{
	strcpy(s, "");
	getstring(s, root);
}

void strinsert1(char *s, char *part)
{
	char *t;
	if (strlen(s)){
		t = (char*)malloc(strlen(s)+1);
		strcpy(t, s);
		sprintf(s, "%s\n%s", part, t);
		free(t);
	} else {
		strcpy(s, part);
	}
}

void getpixelshaderregister(CombinerSource Source, __int32 Constant, char *s, bool *bUseTex1, bool *bUseTex2)
{
	switch (Source){
	case zero:
		strcpy(s, "c4");
		break;
	case one:
		strcpy(s, "c5");
		break;
	case tex0:
		strcpy(s, "t0");
		*bUseTex1 = true;
		break;
	case tex0alpha:
		strcpy(s, "t0.a");
		*bUseTex1 = true;
		break;
	case tex1:
		strcpy(s, "t1");
		*bUseTex2 = true;
		break;
	case tex1alpha:
		strcpy(s, "t1.a");
		*bUseTex2 = true;
		break;
	case constant:
		if (Constant<0){
			sprintf(s, "c%i.a", -Constant-1);
		} else {
			sprintf(s, "c%i", Constant-1);
		}
		break;
	case diffuse:
		strcpy(s, "v0");
		break;
	case diffusealpha:
		strcpy(s, "v0.a");
		break;
	case combinedalpha:
		strcpy(s, "r0.a");
		break;
	}
}

void combtree::getpixelshader(char *s, node *leaf, int iAlternateIndex, bool alphamode, bool alphaphase1, bool *bUseTex1, bool *bUseTex2)
{
	char sOutputLine[256];
	char sFrom[10];
	char sTarget[4];
	if (!alphamode){
		sTarget[0] = 'r';
		sTarget[1] = 'g';
		sTarget[2] = 'b';
		sTarget[3] = 0;
	} else {
		sTarget[0] = 'a';
		sTarget[1] = 0;
	}
	if (leaf->bIsFormula){
		if ((Sidechain!=NULL) && (Sidechain!=leaf) && compare_trees(Sidechain, leaf)){
			return;
		}
		switch (leaf->formula.Op){
		case add:
			sprintf(sOutputLine, "add r%i.%s,", iAlternateIndex, sTarget);
			break;
		case sub:
			sprintf(sOutputLine, "sub r%i.%s,", iAlternateIndex, sTarget);
			break;
		case mul:
			sprintf(sOutputLine, "mul r%i.%s,", iAlternateIndex, sTarget);
			break;
		case lerp:
			sprintf(sOutputLine, "lrp r%i.%s,", iAlternateIndex, sTarget);

			if (leaf->formula.factor->bIsFormula){
				strcpy(sFrom, "r1");
			} else {
				getpixelshaderregister(leaf->formula.factor->value.Source, leaf->formula.factor->value.Constant, sFrom, bUseTex1, bUseTex2);
			}
			strcat(sOutputLine, sFrom);
			strcat(sOutputLine, ",");
			break;
		case mad:
			sprintf(sOutputLine, "mad r%i.%s,", iAlternateIndex, sTarget);

			if (leaf->formula.factor->bIsFormula){
				strcpy(sFrom, "r1");
			} else {
				getpixelshaderregister(leaf->formula.factor->value.Source, leaf->formula.factor->value.Constant, sFrom, bUseTex1, bUseTex2);
			}
			strcat(sOutputLine, sFrom);
			strcat(sOutputLine, ",");
			break;
		}
		if (leaf->formula.a->bIsFormula){
			if (compare_trees(Sidechain, leaf->formula.a)){
				strcpy(sFrom, "r1");
			} else {
				strcpy(sFrom, "r0");
			}
		} else {
			getpixelshaderregister(leaf->formula.a->value.Source, leaf->formula.a->value.Constant, sFrom, bUseTex1, bUseTex2);
		}
		strcat(sOutputLine, sFrom);
		strcat(sOutputLine, ",");

		if (leaf->formula.b->bIsFormula){
			strcpy(sFrom, "r1");
		} else {
			getpixelshaderregister(leaf->formula.b->value.Source, leaf->formula.b->value.Constant, sFrom, bUseTex1, bUseTex2);
		}
		strcat(sOutputLine, sFrom);
		strinsert1(s, sOutputLine);
		if (leaf->formula.factor!=NULL){
			if (leaf->formula.factor->bIsFormula){
				getpixelshader(s, leaf->formula.factor, 1, alphamode, alphaphase1, bUseTex1, bUseTex2);
			}
		}
		if (leaf->formula.b!=NULL){
			if (leaf->formula.b->bIsFormula){
				getpixelshader(s, leaf->formula.b, 1, alphamode, alphaphase1, bUseTex1, bUseTex2);
			}
		}
		if (leaf->formula.a!=NULL){
			if (leaf->formula.a->bIsFormula){
				getpixelshader(s, leaf->formula.a, ((compare_trees(Sidechain, leaf->formula.a))?1:0), alphamode, alphaphase1, bUseTex1, bUseTex2);
			}
		}
	} else {
		// Should only be called if everything simplified down to one single Source
		getpixelshaderregister(leaf->value.Source, leaf->value.Constant, sFrom, bUseTex1, bUseTex2);
		sprintf(sOutputLine, "mul r0.%s,%s,c5", sTarget, sFrom);
		strinsert1(s, sOutputLine);
	}
}

node *combtree::findsidechain(node *leaf, node *found)
{
	if (leaf->bIsFormula){
		if (found!=NULL){
			if (compare_trees(leaf, found)){
				return leaf;
			}
		} else {
			if ((leaf->formula.b!=NULL) && (leaf->formula.b->bIsFormula)){
				found = leaf->formula.b;
			} else {
				if ((leaf->formula.factor!=NULL) && (leaf->formula.factor->bIsFormula)){
					found = leaf->formula.factor;
				}
			}
		}
		return findsidechain(leaf->formula.a, found);
	} else {
		return found;
	}
}

void combtree::getpixelshader(char *s, bool alphamode, bool alphaphase1, bool *bUseTex1, bool *bUseTex2)
{
	// First, find out if there is a sidechain (formula) in b or factor in any element of the tree
	Sidechain = findsidechain(root, NULL);
	s[0] = 0;
	getpixelshader(s, root, 0, alphamode, alphaphase1, bUseTex1, bUseTex2);
}

bool combtree::compare_trees(node *tree1, node *tree2)
{
	if ((tree1 == NULL) || (tree2 == NULL)){
		return ((tree1==NULL) && (tree2==NULL));
	} else {
		if (tree1->bIsFormula){
			if (tree2->bIsFormula){
				return (tree1->formula.Op == tree2->formula.Op)
					&& compare_trees(tree1->formula.a, tree2->formula.a)
					&& compare_trees(tree1->formula.b, tree2->formula.b)
					&& compare_trees(tree1->formula.factor, tree2->formula.factor);
			} else {
				return false;
			}
		} else {
			if (tree2->bIsFormula){
				return false;
			} else {
				if (tree1->value.Source==constant){
					return (tree1->value.Source==tree2->value.Source)
						&& (tree1->value.Constant==tree2->value.Constant);
				} else {
					return (tree1->value.Source==tree2->value.Source);
				}
			}
		}
	}
}

