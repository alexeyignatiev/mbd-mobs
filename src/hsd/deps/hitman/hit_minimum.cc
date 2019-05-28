/*
 * hit_minimum.cc
 *
 *  Created on: Nov 26, 2015
 *      Author: Alexey S. Ignatiev
 *      E-mail: aignatiev@ciencias.ulisboa.pt
 */

#include <set>
#include <stdio.h>
#include <string.h>
#include <unordered_set>
#include "hit_minimum.hh"
#include "itot.hh"

//
//=============================================================================
HitMinimum::HitMinimum()
: Hitman ()
{
	weighted = false;
	nsums    = 0;

	// default parameter values
	param_trim  = 0;
	param_1call = false;
}

//
//=============================================================================
vector<int> HitMinimum::get()
{
	if (vars.size() == 0)
		return solution;  // it is supposed to be empty here

	// MaxSAT-based hitting set enumeration
	while (compute() == false) {
		calc_unsat_core();
		if (soft_core.size() == 0 && sum_core.size() == 0) {
			// core is empty, i.e. hard part is unsatisfiable
			cost_ = SIZE_MAX;
			solution.clear();
			return solution;
		}

		process_unsat_core();
		filter_assumps();
	}

	return solution;
}

//
//=============================================================================
void HitMinimum::hit(const vector<int>& set)
{
	Hitman::hit(set);

	// remembering previosly added set,
	// if at least one solution is already computed
	// i.e. we are now hitting a counterexample
	if (!solution.empty())
		prev_added = set;
}

// one can call this method more than once; however and for the sake of
// correctness, weights of the already existing elements cannot be updated
//=============================================================================
void HitMinimum::weigh(map<int, long>& weights)
{
	for (auto it = weights.begin(); it != weights.end(); ++it) {
		int el = it->first;
		long w = it->second;

		// map element into an internal Boolean variable
		map_var(el);

		// process new soft variable
		int v = varmap_dir[el];
		process_var(v);

		// do not update weights of the already existing elements
		if (weighted == false || soft_wghts.find(v) == soft_wghts.end())
			soft_wghts[varmap_dir[el]] = w;
	}

	weighted = true;
}

//
//=============================================================================
void HitMinimum::clear()
{
	Hitman::clear();
	weighted = false;
	nsums    = 0;

	// we need to clear all the totalizer trees explicitly
	for (auto it = sumsdata.begin(); it != sumsdata.end(); ++it)
		itot_destroy(it->second);

	bounds     .clear();
	prev_added .clear();
	soft_core  .clear();
	soft_lits  .clear();
	soft_wghts .clear();
	sum_assumps.clear();
	sum_inputs .clear();
	sums       .clear();
	sumsdata   .clear();
	sum_core   .clear();
	sum_wghts  .clear();
}

//
//=============================================================================
void HitMinimum::add_soft(int el, long weight)
{
	// create an internal Boolean variable
	map_var(el);

	// add a soft clause;
	process_var(varmap_dir[el]);
	soft_wghts[varmap_dir[el]] = weight;

	if (weight > 1)
		weighted = true;
}

//
//=============================================================================
bool HitMinimum::compute()
{
	static bool res = true;

	if (!param_1call || has_hard || res || solution.empty() || prev_added.empty()) {
		oracle->new_variables(top_vid);
		res = oracle->solve_ext(soft, sum_assumps);

		if (res) {
			solution.clear();
			for (size_t i = 0; i < vars.size(); ++i) {
				int v = vars[i];

				if (oracle->model[v] == l_True)
					solution.push_back(varmap_opp[v]);
			}
		}
	}
	else {
		// we assume that the formula was already relaxed
		// and there is a previously found solution
		// thus, we can obtain a new hitting set
		// without making a SAT call

		unordered_set<int> sol_set(solution.begin(), solution.end());
		for (size_t i = 0; i < prev_added.size(); ++i) {
			if (sol_set.find(prev_added[i]) == sol_set.end()) {
				solution.push_back(prev_added[i]);
				break;
			}
		}

		res = true;
	}

	return res;
}

//
//=============================================================================
void HitMinimum::trim_unsat_core(vec<Lit> *ucore)
{
	int nof_times = param_trim;

	vec<Lit> assumps; assumps.growTo(ucore->size());
	for (int j = 0; j < ucore->size(); ++j)
		assumps[j] = ~(*ucore)[j];

	for (int i = 0; i < nof_times && assumps.size() > 1; ++i) {
		oracle->solve(assumps);
		oracle->get_ucore(*ucore);

		if (assumps.size() == ucore->size())
			// cannot improve anymore
			break;
		else {
			// assumps.size() > ucore->size()
			assumps.clear();
			for (int j = 0; j < ucore->size(); ++j)
				assumps[j] = ~(*ucore)[j];
		}
	}
}

//
//=============================================================================
void HitMinimum::calc_unsat_core()
{
	core_cost = LONG_MAX;

	soft_core.clear();
	sum_core.clear();

	vec<Lit> confl;
	oracle->get_ucore(confl);

	if (confl.size() > 1 && param_trim)
		trim_unsat_core(&confl);

	for (int i = 0; i < confl.size(); ++i) {
		if (soft_lits.find(var(confl[i])) != soft_lits.end()) {
			soft_core.push(confl[i]);
			if (soft_wghts[var(confl[i])] < core_cost)
				core_cost = soft_wghts[var(confl[i])];
		}
		else {
			sum_core.push_back(var(confl[i]));
			if (sum_wghts[var(confl[i])] < core_cost)
				core_cost = sum_wghts[var(confl[i])];
		}
	}
}

//
//=============================================================================
bool HitMinimum::process_unsat_core()
{
	cost_ += core_cost;  // each core contributes to the total cost
	relax_harden_split(sum_inputs);

	if (sum_inputs.size() > 1) {
		ClauseSet sum_cls;
		create_sum(sum_cls, ++nsums, sum_inputs);

		for(size_t i = 0; i < sum_cls.size(); i++)
			oracle->add_clause(sum_cls[i]);

		sum_cls.clear();

		BoundInfo binfo = {
			nsums,  // sum id
			1       // out id
		};

		int svar     = sums[nsums][1];
		bounds[svar] = binfo;

		sum_assumps.push(~mkLit(svar));
		sum_wghts[svar] = core_cost;
	}

	return true;
}

//
//=============================================================================
void HitMinimum::relax_harden_split(vector<int>& rel_vars)
{
	rel_vars.clear();
	core_set.clear();

	// processing the standard soft part of the core
	for (int i = 0; i < soft_core.size(); ++i) {
		long w = soft_wghts[var(soft_core[i])] - core_cost;

		if (w == 0)
			// marking variable as being a part of the core
			// so that next time it is not used as an assump
			core_set.insert(var(soft_core[i]));
		else
			// do not remove this var from assumps
			// since it has a remaining non-zero weight
			soft_wghts[var(soft_core[i])] = w;

		// unit cores are treated differently
		// (their negation is added to the hard part)
		if (soft_core.size() == 1 && sum_core.size() == 0) {
			vec<Lit> lit(1, mkLit(var(soft_core[i])));
			oracle->add_clause(lit);

			return;
		}

		// it is an unrelaxed soft clause,
		// a new relaxed copy of which we add to the solver
		vec<Lit> cl_new(2);
		cl_new[0] = ~mkLit(var(soft_core[i]));
		cl_new[1] = mkLit(++top_vid);  // relaxation variable

		// we are going to use this relaxation variable in the new sum
		rel_vars.push_back(top_vid);

		// making the new clause hard
		oracle->add_clause(cl_new);
	}

	// processing the part of the core containing relaxable constraints
	set<int> sa_set;
	for (size_t i = 0; i < sum_core.size(); ++i) {
		int      svar = sum_core[i];
		unsigned sid  = bounds[svar].sid;
		unsigned b    = bounds[svar].oid + 1;  // increment the bound

		rel_vars.push_back(svar);

		if (sum_wghts[svar] == core_cost)
			sa_set.insert(-svar);
		else
			sum_wghts[svar] -= core_cost;

		increase_sum(sid, b);

		if (b < sums[sid].size()) {
			if (sa_set.find(-sums[sid][b]) != sa_set.end()) {
				sa_set.erase(-sums[sid][b]);
				sum_wghts[sums[sid][b]] = 0;
			}

			if (bounds.find(sums[sid][b]) == bounds.end()) {
				sum_assumps.push(~mkLit(sums[sid][b]));
				sum_wghts[sums[sid][b]] = core_cost;

				bounds[sums[sid][b]] = {
					sid, // sum id
					b    // out id
				};
			}
			else
				sum_wghts[sums[sid][b]] += core_cost;
		}
	}

	if (!sa_set.empty()) {
		int i, j;
		for (i = j = 0; i < sum_assumps.size(); ++i) {
			int slint = var(sum_assumps[i]) * (sign(sum_assumps[i]) ? -1 : 1);
			if (sa_set.find(slint) == sa_set.end())
				sum_assumps[j++] = sum_assumps[i];
			else {
				bounds   .erase(-slint);
				sum_wghts.erase(-slint);
			}
		}
		sum_assumps.shrink(sum_assumps.size() - j);
	}
}

//
//=============================================================================
void HitMinimum::filter_assumps()
{
	int i, j;

	for (i = j = 0; i < soft.size(); ++i) {
		if (core_set.find(var(soft[i])) == core_set.end())
			soft[j++] = soft[i];
		else
			soft_lits.erase(var(soft[i]));
	}

	soft.shrink(soft.size() - j);
}

//
//=============================================================================
void HitMinimum::process_var(int var)
{
	if (varset.find(var) == varset.end()) {
		soft.push(~mkLit(var));
		soft_lits.insert(var);
		vars.push_back  (var);
		varset.insert   (var);

		if (weighted == false)
			soft_wghts[var] = 1;
	}
}

//
//=============================================================================
void HitMinimum::create_sum(
	ClauseSet& clset,
	unsigned sum_id,
	vector<int>& inputs
)
{
	TotTree *tree    = itot_new(top_vid, clset, inputs, 1);
	sums    [sum_id] = tree->vars;
	sumsdata[sum_id] = tree;
}

//
//=============================================================================
void HitMinimum::increase_sum(unsigned sum_id, unsigned b)
{
	ClauseSet sum_cls;

	itot_increase(top_vid, sum_cls, sumsdata[sum_id], b);
	for (unsigned i = 0; i < sum_cls.size(); i++)
		oracle->add_clause(sum_cls[i]);

	sum_cls.clear();

	if (sums[sum_id].size() < sumsdata[sum_id]->vars.size()) {
		unsigned start = sums[sum_id].size();
		sums[sum_id].resize(sumsdata[sum_id]->vars.size());
		for (unsigned i = start; i < sumsdata[sum_id]->vars.size(); i++)
			sums[sum_id][i] = sumsdata[sum_id]->vars[i];
	}
}

//
//=============================================================================
void HitMinimum::destroy_sum_data(void *t)
{
	itot_destroy((TotTree *)t);
}
