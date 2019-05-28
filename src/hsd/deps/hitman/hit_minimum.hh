/*
 * hit_minimum.hh
 *
 *  Created on: Nov 26, 2015
 *      Author: Alexey S. Ignatiev
 *      E-mail: aignatiev@ciencias.ulisboa.pt
 */

#ifndef HIT_MINIMUM_HH_
#define	HIT_MINIMUM_HH_

#include <cstdlib>
#include <map>
#include <unordered_set>
#include "clset_ext.hh"
#include "hitman.hh"

struct TotTree;

class HitMinimum : public Hitman {
public:
	HitMinimum();
	virtual ~HitMinimum()
	{
		// we need to clear all the totalizer trees explicitly
		for (auto it = sumsdata.begin(); it != sumsdata.end(); ++it)
			destroy_sum_data(it->second);
	}

	virtual vector<int> get   ();
	virtual void        hit   (const vector<int>& set);
	virtual void        weigh (map<int, long>&);
	virtual void        clear ();

	void add_soft (int, long);

	struct BoundInfo {
		unsigned sid;  // which sum
		unsigned oid;  // which output (value of the sum)
	};

	void set_param_trim(int times)
	{
		param_trim = times;
	}

	void set_param_1call(bool value = true)
	{
		param_1call = value;
	}
protected:
	unordered_set<int> soft_lits;  // to distinguish from sum assumptions
	vector<int> prev_added;

	int  param_trim;   // number of trimming attemps per core
	bool param_1call;  // do only one SAT call per iteration

	// compute a minimum hitting set
	bool compute ();

	// do additional processing of a new soft variable
	virtual void process_var (int);

	// filter unnesessary assumptions
	void filter_assumps ();

	// fields related to the OLLITI MaxSAT algorithm
	void calc_unsat_core    ();
	void trim_unsat_core    (vec<Lit> *);
	bool process_unsat_core ();
	void relax_harden_split (vector<int>&);
	void create_sum         (ClauseSet&, unsigned, vector<int>&);
	void increase_sum       (unsigned, unsigned);
	void destroy_sum_data   (void *);

	map<int, long> soft_wghts;
	map<int, long> sum_wghts;
	long core_cost;
	map<unsigned, vector<int> > sums;
	vector<int> sum_inputs; // used for collecting input vars to the next sum
	vec<Lit> soft_core;
	unordered_set<int> core_set;
	vector<int> sum_core; // for extracting sum literals in core
	vec<Lit> sum_assumps; // all the sum output literals (set as assumptions)
	unsigned nsums;
	map<unsigned, TotTree *> sumsdata;
	map<int, BoundInfo> bounds;

	bool weighted;
private:
};

#endif	/* HIT_MINIMUM_HH_ */
