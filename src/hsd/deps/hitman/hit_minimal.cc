/*
 * hit_minimal.cc
 *
 *  Created on: Nov 26, 2015
 *      Author: Alexey S. Ignatiev
 *      E-mail: aignatiev@ciencias.ulisboa.pt
 */

#include "hit_minimal.hh"

//
//=============================================================================
HitMinimal::HitMinimal()
: Hitman      ()
, param_atype (atype_lbx)
, param_dcall (false)
{
}

//
//=============================================================================
vector<int> HitMinimal::get()
{
	bb_assumps.clear();
	model     .clear();
	setd      .clear();
	solution  .clear();
	ss_assumps.clear();

	if (oracle->solve()) {
		// hard part is satisfiable => there is a solution
		overapprox();
		compute();

		// MCS comprises backbone literals
		solution.resize(bb_assumps.size());
		for (int i = 0; i < bb_assumps.size(); ++i)
			solution[i] = varmap_opp[var(bb_assumps[i])];
	}
	else
		cost_ = SIZE_MAX;

	return solution;
}

//
//=============================================================================
void HitMinimal::overapprox()
{
	get_model();

	for (int i = 0; i < soft.size(); ++i) {
		if (model[var(soft[i])] == l_False)
			// soft clauses contain negated literals
			// so if var == false then the clause is satisfied
			ss_assumps.push(soft[i]);
		else
			setd.push_back (soft[i]);
	}
}

//
//=============================================================================
void HitMinimal::compute()
{
	switch (param_atype) {
	case atype_lbx:
		lbx_compute();
		return;
	case atype_ucd:
		ucd_compute();
		return;
	case atype_ubs:
		ubs_compute();
		return;
	case atype_lopz:
		lopz_compute();
		return;
	}
}

//
//=============================================================================
void HitMinimal::lbx_compute()
{
	// unless clause D checks are used,
	// test one literal at a time and
	// add it either to satisfied
	// of backbone assumptions
	for (size_t i = 0; i < setd.size(); ++i) {
		if (param_dcall) {
			vec<Lit> cld(setd.size() - i);
			for (int j = 0; j < cld.size(); ++j)
				cld[j] = setd[i + j];

			do_cld_check(cld);
			i = 0;
		}

		if (setd.size()) {
			// it may be empty after the clause D check

			ss_assumps.push(setd[i]);
			if (oracle->solve_ext(ss_assumps, bb_assumps) == false) {
				ss_assumps.shrink(1);
				bb_assumps.push(~setd[i]);
			}
		}
	}
}

//
//=============================================================================
void HitMinimal::ucd_compute()
{
	cerr << "UCD is currently unimplemented" << endl;
	exit(1);
}

//
//=============================================================================
void HitMinimal::ubs_compute()
{
	cerr << "UBS is currently unimplemented" << endl;
	exit(1);
}

//
//=============================================================================
void HitMinimal::lopz_compute()
{
	cerr << "LOPZ is currently unimplemented" << endl;
	exit(1);
}

//
//=============================================================================
void HitMinimal::do_cld_check(vec<Lit>& cld)
{
	// adding a selector literal to clause D
	// selector literals for clauses D currently
	// cannot be reused, but this may change later
	Lit sel = ~mkLit(++top_vid);
	cld.push(sel);

	// adding clause D
	oracle->add_clause(cld);
	ss_assumps.push(~sel);

	setd.clear();
	if (oracle->solve_ext(ss_assumps, bb_assumps)) {
		ss_assumps.shrink(1);
		get_model();

		for (int i = 0; i < cld.size() - 1; ++i) {
			// filtering all satisfied (l_False) literals
			if (model[var(cld[i])] == l_False)
				ss_assumps.push(cld[i]);
			else
				setd.push_back (cld[i]);
		}
	}
	else {
		// clause D is unsatisfiable => all literals are backbones
		for (int i = 0; i < cld.size() - 1; ++i)
			bb_assumps.push(~cld[i]);

		ss_assumps.shrink(1);
	}

	// deactivating clause D
	vec<Lit> cld_block(1, sel);
	oracle->add_clause(cld_block);
}

//
//=============================================================================
void HitMinimal::clear()
{
	Hitman::clear();

	bb_assumps.clear();
	model     .clear();
	setd      .clear();
	solution  .clear();
	ss_assumps.clear();
}
