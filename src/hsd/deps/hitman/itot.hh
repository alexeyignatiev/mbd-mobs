/*
 * itot.hh
 *
 *  Created on: May 29, 2015
 *      Author: Antonio Morgado, Alexey S. Ignatiev
 *      E-mail: {ajmorgado,aignatiev}@ciencias.ulisboa.pt
 */

#ifndef ITOT_HH_
#define ITOT_HH_

#include <algorithm>
#include <cmath>
#include <vector>
#include <deque>

using namespace std;

typedef struct TotTree {
	vector<int> vars;
	unsigned nof_input;
	TotTree *left;
	TotTree *right;
} TotTree;

//
//=============================================================================
template <class CS, class INTV>
void itot_new_ua(
	int& top_vid,
	CS& clset,
	INTV& ov,
	unsigned k,
	INTV& av,
	INTV& bv
)
{
	// i = 0
	unsigned kmin = std::min(k, (unsigned)bv.size());
	for (unsigned j = 0; j < kmin; ++j)
		clset.create_binary_clause(-bv[j], ov[j]);

	// j = 0
	kmin = std::min(k, (unsigned)av.size());
	for (unsigned i = 0; i < kmin; ++i)
		clset.create_binary_clause(-av[i], ov[i]);

	// i, j > 0
	for (unsigned i = 1; i <= kmin; ++i) {
		unsigned minj = std::min(k - i, (unsigned)bv.size());
		for (unsigned j = 1; j <= minj; ++j)
			clset.create_ternary_clause(-av[i - 1], -bv[j - 1], ov[i + j - 1]);
	}
}

//
//=============================================================================
template <class CS, class INTV>
TotTree *itot_new(int& top_vid, CS& clset, INTV& invars, unsigned k)
{
	unsigned n = invars.size();
	deque<TotTree *> nqueue;

	for (unsigned i = 0; i < n; ++i) {
		TotTree *tree = new TotTree();

		tree->vars.resize(1);
		tree->vars[0]   = invars[i];
		tree->nof_input = 1;
		tree->left      = 0;
		tree->right     = 0;

		nqueue.push_back(tree);
	}

	while (nqueue.size() > 1) {
		TotTree *l = nqueue.front();
		nqueue.pop_front();
		TotTree *r = nqueue.front();
		nqueue.pop_front();

		TotTree *node = new TotTree();
		node->nof_input = l->nof_input + r->nof_input;
		node->left      = l;
		node->right     = r;

		unsigned kmin = std::min(k + 1, node->nof_input);

		node->vars.resize(kmin);
		for (unsigned i = 0; i < kmin; ++i)
			node->vars[i] = ++top_vid;

		itot_new_ua(top_vid, clset, node->vars, kmin, l->vars, r->vars);
		nqueue.push_back(node);
	}

	return nqueue.front();
}

//
//=============================================================================
template <class CS, class INTV>
void itot_increase_ua(
	int& top_vid,
	CS& clset,
	INTV& ov,
	INTV& av,
	INTV& bv,
	unsigned k
)
{
	unsigned last = ov.size();

	for (unsigned i = last; i < k; ++i)
		ov.push_back(++top_vid);

	// add the constraints
	// i = 0
	unsigned maxj = std::min(k, (unsigned)bv.size());
	for (unsigned j = last; j < maxj; ++j)
		clset.create_binary_clause(-bv[j], ov[j]);

	// j = 0
	unsigned maxi = std::min(k, (unsigned)av.size());
	for (unsigned i = last; i < maxi; ++i)
		clset.create_binary_clause(-av[i], ov[i]);

	// i, j > 0
	for (unsigned i = 1; i <= maxi; ++i) {
		unsigned maxj = std::min(k - i, (unsigned)bv.size());
		unsigned minj = std::max((int)last - (int)i + 1, 1);
		for (unsigned j = minj; j <= maxj; ++j)
			clset.create_ternary_clause(-av[i - 1], -bv[j - 1], ov[i + j - 1]);
	}
}

//
//=============================================================================
template <class CS>
void itot_increase(int& top_vid, CS& clset, TotTree *tree, unsigned k)
{
	unsigned kmin = std::min(k + 1, tree->nof_input);

	if (kmin <= tree->vars.size())
		return;

	itot_increase   (top_vid, clset, tree->left,  k);
	itot_increase   (top_vid, clset, tree->right, k);
	itot_increase_ua(top_vid, clset, tree->vars, tree->left->vars, tree->right->vars, kmin);
}

//
//=============================================================================
template <class CS>
TotTree *itot_merge(
	int& top_vid,
	CS& clset,
	TotTree *ta,
	TotTree *tb,
	unsigned k
)
{
	itot_increase(top_vid, clset, ta, k);
	itot_increase(top_vid, clset, tb, k);

	unsigned n    = ta->nof_input + tb->nof_input;
	unsigned kmin = std::min(k + 1, n);

	TotTree *tree = new TotTree();
	tree->nof_input = n;
	tree->left      = ta;
	tree->right     = tb;

	tree->vars.resize(kmin);
	for (unsigned i = 0; i < kmin; ++i)
		tree->vars[i] = ++top_vid;

	itot_new_ua(top_vid, clset, tree->vars, kmin, ta->vars, tb->vars);
	return tree;
}

//
//=============================================================================
template <class CS, class INTV>
TotTree *itot_merge(
	int& top_vid,
	CS& clset,
	INTV& newin,
	TotTree *ta,
	unsigned k
)
{
	TotTree *tb = itot_new(top_vid, clset, newin, k);
	return itot_merge(top_vid, clset, ta, tb, k);
}

// recursive destruction of the tree
//=============================================================================
static void itot_destroy(TotTree *tree)
{
	if (tree->left )
		itot_destroy(tree->left );
	if (tree->right)
		itot_destroy(tree->right);

	tree->vars.clear();
	delete tree;
}

#endif // ITOT_HH_
