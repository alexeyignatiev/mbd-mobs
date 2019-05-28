/*
 * coex_finder.cc
 *
 *  Created on: Aug 9, 2016
 *      Author: aign
 */

#include <algorithm>
#include <math.h>
#include "coex_finder.hh"
#include "minisat_ext.hh"

using namespace Minisat;

//
//=============================================================================
CoexFinder::CoexFinder(
	ClauseSet& system,
	vector<int>& selectors_,
	vector<vector<int>>& obs_,
	IDManager& id_man,
	Options& options_
)
: options (options_)
{
	oracle = new MiniSatExt();
	oracle->new_variables(id_man.top_id());

	// adding system clauses as they are
	for (size_t i = 0; i < system.size(); ++i)
		oracle->add_clause(system[i]);

	// saving selectors
	for (size_t i = 0; i < selectors_.size(); ++i) {
		selectors.push_back(selectors_[i]);
		components.insert(selectors_[i]);
	}
	sort(selectors.begin(), selectors.end());

	// saving observations
	for (size_t i = 0; i < obs_.size(); ++i) {
		vec<Lit> *o = new vec<Lit>(obs_[i].size());

		for (size_t j = 0; j < obs_[i].size(); ++j) {
			int lit = obs_[i][j];
			(*o)[j] = lit >= 0 ? mkLit(lit) : ~mkLit(-lit);
		}

		obs.push_back(o);
	}
}

//
//=============================================================================
CoexFinder::~CoexFinder()
{
	if (oracle) {
		delete oracle;
		oracle = nullptr;
	}
}

// test system with or without observations
//=============================================================================
bool CoexFinder::check(bool test_observations)
{
	vec<Lit> assumps;

	for (size_t i = 0; i < selectors.size(); ++i)
		assumps.push(mkLit(selectors[i]));

	if (test_observations == false)
		return oracle->solve(assumps);
	else {
		size_t i, j;
		for (i = j = 0; i < obs.size(); ++i) {
			vec<Lit> *o = (vec<Lit> *)obs[i];
			if (oracle->solve_ext(assumps, *o) == false)
				obs[j++] = obs[i];
			else
				delete o;
		}

		if (j < obs.size()) {
			cout << "c # of failures: " << j << endl;
			obs.resize(j);
		}

		return (j == 0) ? true : false;
	}
}

//
//=============================================================================
vector<int> CoexFinder::get(vector<int>& cand)
{
	size_t j = 0;
	vec<Lit> assumps;
	for (size_t i = 0; i < cand.size(); ++i) {
		for (; selectors[j] < cand[i]; ++j)
			assumps.push(mkLit(selectors[j]));

		j++;
	}

	for (; j < selectors.size(); ++j)
		assumps.push(mkLit(selectors[j]));

	// cout << "cand ";
	// for (int i = 0; i < assumps.size(); ++i)
	// 	cout << (sign(assumps[i]) ? "-" : "") << var(assumps[i]) << " ";
	// cout << endl;

	vector<int> coex;

	// testing observatioins one by one
	for (size_t i = 0; i < obs.size(); ++i) {
		vec<Lit> *o = (vec<Lit> *)obs[i];
		if (oracle->solve_ext(assumps, *o) == false) {
			vec<Lit> ucore; oracle->get_ucore(ucore);
			if (ucore.size() > 1 && options.get_ctrimming())
				trim_unsat_core(&ucore);

			if (options.get_reduce_coex() == "none") {
				// do not reduce counterexamples
				// cout << "coex ";
				for (int j = 0; j < ucore.size(); ++j) {
					int v = var(ucore[j]);
					// cout << (sign(ucore[j]) ? "-" : "") << v << " ";
					if (components.find(v) != components.end())
						coex.push_back(v);
				}
				// cout << endl;
			}
			else
				reduce_coex(&ucore, coex);

			failure_id = i + 1;
			break;
		}
	}

	sort(coex.begin(), coex.end());
	return coex;
}

// block a given diagnosis and check if there are other diagnoses
//=============================================================================
bool CoexFinder::block(vector<int>& diag)
{
	vec<Lit> cl(diag.size());
	for (size_t i = 0; i < diag.size(); ++i)
		cl[i] = mkLit(diag[i]);

	oracle->add_clause(cl);

	for (size_t i = 0; i < obs.size(); ++i) {
		vec<Lit> *o = (vec<Lit> *)obs[i];

		if (oracle->solve(*o) == false)
			return false;
	}

	return true;
}

//
//=============================================================================
size_t CoexFinder::getf()
{
	return failure_id;
}

// trims complete cores including selectors and observations
// it might make more sense to target trimming selectors only
//=============================================================================
void CoexFinder::trim_unsat_core(void *confl)
{
	vec<Lit> *ucore = (vec<Lit> *)confl;
	int nof_times = options.get_ctrimming();

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
void CoexFinder::reduce_coex(void *confl, vector<int>& coex)
{
	vec<Lit> assumps_c;
	vec<Lit> assumps_o;

	// initializing assumptions
	vec<Lit> *core = (vec<Lit> *)confl;
	for (int i = 0; i < core->size(); ++i) {
		if (components.find(var((*core)[i])) != components.end())
			assumps_c.push(~(*core)[i]);
		else
			assumps_o.push(~(*core)[i]);
	}

	if (options.get_reduce_coex() == "lin")
		reduce_lin(&assumps_c, (options.get_exhaustive_reduction() ? NULL : &assumps_o));
	else  // "qxp" is expected
		reduce_qxp(&assumps_c, (options.get_exhaustive_reduction() ? NULL : &assumps_o));

	// creating counterexample
	for (int i = 0; i < assumps_c.size(); ++i)
		coex.push_back(var(assumps_c[i]));
}

// simple linear search
//=============================================================================
void CoexFinder::reduce_lin(void *ac, void *ao)
{
	vec<Lit> *assumps_c = (vec<Lit> *)ac;
	vec<Lit> *assumps_o = (vec<Lit> *)ao;

	for (int i = 0; i < assumps_c->size();) {
		Lit l = (*assumps_c)[i];
		(*assumps_c)[i] = (*assumps_c)[assumps_c->size() - 1];
		assumps_c->shrink(1);
		// cout << "trying " << var(l) << endl;

		if (check_reduction(assumps_c, assumps_o)) {
			assumps_c->push((*assumps_c)[i]);
			(*assumps_c)[i++] = l;
		}
		// if (oracle->solve_ext(*assumps_c, *assumps_o)) {
		// 	assumps_c->push((*assumps_c)[i]);
		// 	(*assumps_c)[i++] = l;
		// }
	}
}

// quickxplain-like search
//=============================================================================
void CoexFinder::reduce_qxp(void *ac, void *ao)
{
	vcache.clear();
	vec<Lit> *assumps_c = (vec<Lit> *)ac;

	vector<bool> bin_repr = vector<bool>(assumps_c->size(), true);
	vector<vector<int>> stack;

	// 2nd half
	vector<int> range(2);
	range[0] = bin_repr.size() / 2;
	range[1] = bin_repr.size() - bin_repr.size() / 2;
	stack.push_back(range);

	// 1st half
	range[0] = 0;
	range[1] = bin_repr.size() / 2;
	stack.push_back(range);

	while (stack.size()) {
		range = stack.back();
		stack.resize(stack.size() - 1);

		// cout << "trying range [" << range[0] << ", " << range[0] + range[1] - 1 << "]" << endl;
		if (check_range(ac, ao, bin_repr, range) && range[1] > 1) {
			// 2nd half
			int bg = range[0];
			int sz = range[1];

			range[0]  = bg + sz / 2;
			range[1]  = sz - sz / 2;
			stack.push_back(range);

			// 1st half
			range[0] = bg;
			range[1] = sz / 2;
			stack.push_back(range);
		}
	}

	int i, j;
	for (i = j = 0; i < assumps_c->size(); ++i) {
		if (bin_repr[i])
			(*assumps_c)[j++] = (*assumps_c)[i];
	}
	assumps_c->shrink(assumps_c->size() - j);
}

//
//=============================================================================
bool CoexFinder::check_range(
	void *ac,
	void *ao,
	vector<bool>& bin_repr,
	vector<int>& range
)
{
	vec<Lit> *assumps_c = (vec<Lit> *)ac;
	vec<Lit> *assumps_o = (vec<Lit> *)ao;

	for (int i = range[0]; i < range[0] + range[1]; ++i)
		bin_repr[i] = false;

	vec<Lit> assumps;
	for (int i = 0; i < assumps_c->size(); ++i) {
		if (bin_repr[i])
			assumps.push((*assumps_c)[i]);
	}

	// cout << "bin:";
	// for (size_t i = 0; i < bin_repr.size(); ++i)
	// 	cout << " " << bin_repr[i];
	// cout << endl;

	auto got = vcache.find(bin_repr);
	if (got != vcache.end()) {
		for (int i = range[0]; i < range[0] + range[1]; ++i)
			bin_repr[i] = true;

		return got->second;
	}

	if (check_reduction(&assumps, assumps_o) == true) {
		for (int i = range[0]; i < range[0] + range[1]; ++i)
			bin_repr[i] = true;

		// cout << "bin back:";
		// for (size_t i = 0; i < bin_repr.size(); ++i)
		// 	cout << " " << bin_repr[i];
		// cout << endl;

		vcache[bin_repr] = true;
		return true;
	}
	// if (oracle->solve_ext(assumps, *assumps_o) == true) {
	// 	for (int i = range[0]; i < range[0] + range[1]; ++i)
	// 		bin_repr[i] = true;

	// 	// cout << "bin back:";
	// 	// for (size_t i = 0; i < bin_repr.size(); ++i)
	// 	// 	cout << " " << bin_repr[i];
	// 	// cout << endl;

	// 	vcache[bin_repr] = true;
	// 	return true;
	// }

	vcache[bin_repr] = false;
	return false;
}

//
//=============================================================================
bool CoexFinder::check_reduction(void *ac, void *ao)
{
	vec<Lit> *assumps_c = (vec<Lit> *)ac;
	vec<Lit> *assumps_o = (vec<Lit> *)ao;

	if (assumps_o)
		return oracle->solve_ext(*assumps_c, *assumps_o);
	else {
		for (size_t i = 0; i < obs.size(); ++i) {
			vec<Lit> *o = (vec<Lit> *)obs[i];
			if (oracle->solve_ext(*assumps_c, *o) == false)
				return false;
		}

		return true;
	}
}
