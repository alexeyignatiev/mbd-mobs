/*
 * coex_finder.hh
 *
 *  Created on: Aug 9, 2016
 *      Author: aign
 */

#ifndef COEX_FINDER_HH_
#define COEX_FINDER_HH_

#include <unordered_map>
#include <unordered_set>
#include "clset_ext.hh"
#include "id_manager.hh"
#include "options.hh"

namespace Minisat {
	class MiniSatExt;
}

class CoexFinder {
public:
	CoexFinder(ClauseSet&, vector<int>&, vector<vector<int>>&, IDManager&,
		Options&);
	~CoexFinder();

	bool        check (bool test_observations = true);
	vector<int> get   (vector<int>&);
	bool        block (vector<int>&);
	size_t      getf  ();
protected:
	void trim_unsat_core(void *);
	void reduce_coex(void *, vector<int>&);
	void reduce_lin(void *, void *);
	void reduce_qxp(void *, void *);
	bool check_range(void *, void *, vector<bool>&, vector<int>&);
	bool check_reduction(void *, void *);
private:
	Minisat::MiniSatExt *oracle;
	vector<int> selectors;
	unordered_set<int> components;
	vector<void *> obs;
	size_t failure_id;
	unordered_map<vector<bool>,bool> vcache;
	Options& options;
};

#endif	/* COEX_FINDER_HH_ */

