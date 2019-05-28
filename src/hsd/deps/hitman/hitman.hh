/*
 * hitman.hh
 *
 *  Created on: Nov 26, 2015
 *      Author: Alexey S. Ignatiev
 *      E-mail: aignatiev@ciencias.ulisboa.pt
 */

#ifndef HITMAN_HH_
#define HITMAN_HH_

#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "minisat_ext.hh"

using namespace std;
using Minisat::MiniSatExt;

class Hitman {
public:
	Hitman()
	: cost_    (0)
	, top_vid  (0)
	, has_hard (false)
	{
		soft.clear();
		vars.clear();

		oracle = new MiniSatExt();
	}

	~Hitman()
	{
		if (oracle) {
			delete oracle;
			oracle = nullptr;
		}
	}

	virtual vector<int> get () = 0;

	virtual void hit(const vector<int>& set)
	{
		if (set.empty())
			return;

		vec<Lit> to_hit;

		for (size_t i = 0; i < set.size(); ++i) {
			// create a new internal var for
			// this element, if it does not exist yet
			map_var(set[i]);

			// process the newly introduced variable
			process_var(varmap_dir[set[i]]);

			to_hit.push(mkLit(varmap_dir[set[i]]));
		}

		// adding hard clause to hit
		oracle->add_clause(to_hit);
	}

	virtual void block(const vector<int>& set)
	{
		if (set.empty())
			return;

		vec<Lit> to_block;

		for (size_t i = 0; i < set.size(); ++i) {
			// create a new internal var for
			// this element, if it does not exist yet
			map_var(set[i]);

			// process the newly introduced variable
			process_var(varmap_dir[set[i]]);

			to_block.push(~mkLit(varmap_dir[set[i]]));
		}

		// adding hard clause to block
		oracle->add_clause(to_block);
	}

	// this method can be used to utilize additional (hard) constraints
	// it is applicable only if sets to hit contain only positive integers
	// otherwise, the mapping between vars will not work correctly
	virtual void add_hard(const vector<int>& cl)
	{
		if (cl.empty())
			return;

		vec<Lit> to_add;

		for (size_t i = 0; i < cl.size(); ++i) {
			// create internal Boolean variable
			// for each variable of the clause
			map_var(abs(cl[i]));

			int v = varmap_dir[abs(cl[i])];
			to_add.push(cl[i] > 0 ? mkLit(v) : ~mkLit(v));
		}

		has_hard = true;

		// adding new hard clause
		oracle->add_clause(to_add);
	}

	// add a unit soft clause directly (without calling hit() or block())
	void add_soft(int el)
	{
		// create an internal Boolean variable
		map_var(el);

		// add a soft clause;
		process_var(varmap_dir[el]);
	}

	vector<int> enumerate()
	{
		get();
		block_previous();

		return solution;
	}

	size_t cost()
	{
		return cost_;
	}

	virtual void clear()
	{
		cost_    = 0;
		top_vid  = 0;
		has_hard = false;

		varmap_dir.clear();
		varmap_opp.clear();
		solution  .clear();

		if (oracle) {
			delete oracle;
			oracle = new MiniSatExt();
		}
	}
private:
protected:
	void block_previous()
	{
		if (solution.empty())
			return;

		vec<Lit> block_cl(solution.size());

		for (size_t i = 0; i < solution.size(); ++i)
			block_cl[i] = ~mkLit(varmap_dir[solution[i]]);

		oracle->add_clause(block_cl);
	}

	void map_var(int el)
	{
		if (varmap_dir.find(el) == varmap_dir.end()) {
			varmap_dir[el     ] = ++top_vid;
			varmap_opp[top_vid] = el;
		}
	}

	// process (only) variable introduced for a set element (i.e. soft var)
	virtual void process_var(int var)
	{
		if (varset.find(var) == varset.end()) {
			soft.push(~mkLit(var));
			vars.push_back  (var);
			varset.insert   (var);
		}
	}

	size_t cost_;
	int top_vid;       // MiniSat can't handle more than 2 ** 32 variables
	bool has_hard;     // whether the oracle has hard clauses or not
	vec<Lit> soft;     // soft clauses represented as assumps
	vector<int> vars;  // set elements are mapped to these vars

	unordered_map<int, int> varmap_dir;  // direct mapping
	unordered_map<int, int> varmap_opp;  // opposite mapping
	unordered_set<int> varset;           // auxiliary set to test whether
	                                     // variables are already processed

	vector<int> solution;
	MiniSatExt *oracle;
};

#endif  // HITMAN_HH_
