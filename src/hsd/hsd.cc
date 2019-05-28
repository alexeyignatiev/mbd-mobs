/*
 * dex.cc
 *
 *  Created on: Aug 8, 2016
 *      Author: aign
 */

#include <algorithm>
#include <stdio.h>
#include <zlib.h>
#include "clset_ext.hh"
#include "coex_finder.hh"
#include "hit_minimum.hh"
#include "hit_minimal.hh"
#include "id_manager.hh"
#include "options.hh"
#include "simple_clset.hh"
#include "wcnffmt-mf.hh"

const int alarm_time = 250;
int global_timeout;

//
//=============================================================================
void load_file(
	Options& options,
	SimpleClauseSet& clset,
	vector<vector<int>>& obs
)
{
	gzFile in;
	char *fname = NULL;
	bool std_in = true;

	if (options.get_rest().size() != 0) {
		// reading filename from the option args if any
		std_in = false;
		fname = (char *)malloc(500 * sizeof(char));
		strcpy(fname, options.get_rest()[0].c_str());
	}

	// opening
	if (!std_in)
		in = gzopen(fname, "r");
	else
		in = gzdopen(fileno(stdin), "r");

	if (in == Z_NULL) {
		string msg = "Unable to open file";
		if (fname) {
			msg += ": ";
			msg += fname;
		}

		tool_abort(msg.c_str());
	}

	// parsing formula
	MWCNFParser parser;
	parser.load_wcnf_file(in, clset, obs);

	free(fname);
	gzclose(in);
}

//
//=============================================================================
void process_clset(
	SimpleClauseSet& cls,
	ClauseSet& system,
	vector<int>& selectors,
	IDManager& id_man
)
{
	long topw = cls.get_topw();
	if (topw == 1)
		topw = 0;  // a small trick for the plain case

	for (auto it = cls.vect_begin(); it != cls.vect_end(); ++it) {
		vector<int> cl((*it)->clits().size());
		for (size_t i = 0; i < (*it)->clits().size(); ++i)
			cl[i] = (*it)->clits()[i];
		long cl_w = (*it)->clw();

		if (cl_w == topw)
			system.add_clause(cl);
		else {
			int sel = cl[0];

			if (cl.size() > 1 || cl[0] < 0) {
				// relax if necessary
				sel = id_man.new_id();
				cl.push_back(-sel);
				system.add_clause(cl);
			}

			selectors.push_back(sel);
		}
	}
}

//
//=============================================================================
void report_problem_data(
	ClauseSet& system,
	vector<int>& selectors,
	vector<vector<int>>& obs,
	int verb
)
{
	if (verb) {
		cout << "c # of system cls: " << system.size() << endl;

		if (verb > 2) {
			for (size_t i = 0; i < system.size(); ++i) {
				cout << "c ";
				for (size_t j = 0; j < system[i].size(); ++j)
					cout << system[i][j] << " ";
				cout << "0" << endl;
			}
		}

		cout << "c # of components: " << selectors.size() << endl;

		if (verb > 2) {
			cout << "c ";
			for (size_t i = 0; i < selectors.size(); ++i)
				cout << selectors[i] << " ";
			cout << "0" << endl;
		}

		cout << "c # of observations: " << obs.size() << endl;

		if (verb > 2) {
			for (size_t i = 0; i < obs.size(); ++i) {
				cout << "c ";
				for (size_t j = 0; j < obs[i].size(); ++j)
					cout << obs[i][j] << " ";
				cout << "0" << endl;
			}
		}
	}
}

//
//=============================================================================
void report_set(vector<int>& set, map<int, size_t>& id_map, string prefix)
{
	cout << prefix;

	for (size_t i = 0; i < set.size(); ++i)
		cout << id_map[set[i]] << " ";

	cout << "0" << endl;
}

//
//=============================================================================
Hitman *prepare_hitman(
	ClauseSet& system,
	vector<int>& selectors,
	vector<vector<int>>& obs,
	map<int, size_t> id_map,
	CoexFinder& c,
	size_t& disj,
	Options& options
)
{
	Hitman *q;
	if (options.get_sorted()) {
		q = new HitMinimum();

		if (options.get_trimming())
			((HitMinimum *)q)->set_param_trim(options.get_trimming());
	}
	else
		q = new HitMinimal();

	if (options.get_bootstrap()) {
		disj = 0;
		vector<int> sels;

		while (true) {
			// note: c.get(sels) uses the complement of sels
			vector<int> expl = c.get(sels);

			if (expl.empty())
				break;

			if (options.get_verb() > 1) {
				report_set(expl, id_map, "c E: ");
				cout << "c O: " << c.getf() << endl;
			}

			// updating selectors
			for (size_t i = 0; i < expl.size(); ++i)
				sels.push_back(expl[i]);
			sort(sels.begin(), sels.end());

			// hit counterexample next time
			q->hit(expl);
			disj++;
		}

		if (options.get_verb())
			cout << "c # of disj explns: " << disj << endl;
	}

	return q;
}

//
//=============================================================================
void delete_hitman(Hitman *q, bool sorted)
{
	if (sorted)
		delete ((HitMinimum *)q);
	else
		delete ((HitMinimal *)q);
}

//
//=============================================================================
void solve(
	ClauseSet& system,
	vector<int>& selectors,
	vector<vector<int>>& obs,
	IDManager& id_man,
	Options& options
)
{
	// report input problem data
	report_problem_data(system, selectors, obs, options.get_verb());

	// unsat checker (computes counterexamples)
	CoexFinder c(system, selectors, obs, id_man, options);

	// check if the system itself is consistent
	if (c.check(false) == false) {
		cout << "s NO SOLUTION" << endl;
		return;
	}
	// and if the system is not consistent with the observations
	else if (c.check(true) == true) {
		cout << "s OPTIMUM FOUND" << endl << "o 0" << endl;
		return;
	}

	// a mapping back from selectors to their ids
	map<int, size_t> id_map;
	for (size_t i = 0; i < selectors.size(); ++i)
		id_map[selectors[i]] = i + 1;

	// number of iterations and solutions found
	size_t iters = 0, diags = 0, expls = 0;

	// minimum hitting set enumerator (left-hand side)
	Hitman *q = prepare_hitman(system, selectors, obs, id_map, c, expls,
			options);

	// minimum cost solution, currently uninitialized
	size_t best_cost = 0, curr_cost = 0;

	bool best_only = false;
	size_t to_compute = 0;
	if (options.get_enumerate() == "best")
		best_only = true;
	else if (options.get_enumerate() != "all")
		to_compute = stoi(options.get_enumerate());

	while (true) {
		vector<int> cand = q->get();

		if (q->cost() == SIZE_MAX || (best_only && best_cost != 0 && q->cost() > best_cost))
			// stop if either there is no candidate
			// or its cost is greater than best cost
			break;

		if (options.get_sorted() && q->cost() > curr_cost) {
			curr_cost = q->cost();

			if (options.get_verb()) {
				cout << "c cost: " << curr_cost;
				cout << "; iters: " << iters << endl;
			}
		}

		// candidates need to be sorted
		sort(cand.begin(), cand.end());

		if (options.get_verb() > 1)
			report_set(cand, id_map, "c C: ");

		iters++;
		vector<int> coex = c.get(cand);

		if (coex.empty()) {
			// no counterexample exists
			// report solution
			if (diags == 0 && options.get_sorted()) {
				cout << "s OPTIMUM FOUND" << endl;
				cout << "o " << curr_cost << endl;
			}

			report_set(cand, id_map, "c D: ");
			diags++;

			if (best_only && diags == 1)
				// recording best cost
				best_cost = q->cost();

			// either continue enumerating or stop
			if (to_compute == 0 || diags < to_compute)
				q->block(cand);
			else
				break;

			if (options.get_check_more() && c.block(cand) == false)
				break;
		}
		else {
			if (options.get_verb() > 1) {
				report_set(coex, id_map, "c E: ");
				cout << "c O: " << c.getf() << endl;
			}

			// hit counterexample next time
			q->hit(coex);
			expls++;
		}

		if (options.get_verb() > 1)
			cout << "c" << endl;
	}

	if (diags == 0)
		// no solution was found
		cout << "s NO SOLUTION" << endl;

	if (options.get_verb()) {
		cout << "c # of iters: " << iters << endl;
		cout << "c # of diags: " << diags << endl;
		cout << "c # of expls: " << expls << endl;
	}

	delete_hitman(q, options.get_sorted());
}

//
//=============================================================================
void run_tool(Options& options)
{
	vector<vector<int>> obs;
	SimpleClauseSet clset;
	load_file(options, clset, obs);

	IDManager id_man;
	id_man.reg_ids(VarIDManager::instance().top_id());

	ClauseSet system;
	vector<int> selectors;
	process_clset(clset, system, selectors, id_man);

	solve(system, selectors, obs, id_man, options);
}

//
//=============================================================================
void init_alarm(int timeout)
{
	global_timeout = timeout;
	alarm((timeout > alarm_time) ? alarm_time : timeout);
}

//
//=============================================================================
static void SIG_alarm_handler(int signum)
{
	int trest = int(global_timeout - RUSAGE::read_cpu_time() + 0.5);
	trest = (trest >= 0) ? trest : 0;

	if(trest >= 1) {
		int tnext = (trest > alarm_time) ? alarm_time : trest;
		alarm(tnext);
	}
	else {
		cout << "c program timed out; terminating..." << endl;
		exit(EXIT_FAILURE);
	}
}

//
//=============================================================================
void show_info(string c)
{
	cout << c << "Implicit hitting set based approach to MBD with multiple observations" << endl;
	cout << c << "author(s):      Alexey Ignatiev     [email:aignatiev@ciencias.ulisboa.pt]" << endl;
	cout << c << "contributor(s): Joao Marques-Silva  [email:jpms@ciencias.ulisboa.pt]" << endl;
	cout << c << "contributor(s): Antonio Morgado     [email:ajmorgado@ciencias.ulisboa.pt]" << endl;
	cout << c << "contributor(s): Georg Weissenbacher [email:georg.weissenbacher@tuwien.ac.at]" << endl;
	cout << c << endl;
}

//
//=============================================================================
int main(int argc, char *argv[])
{
	Options options;
	options.parse(argc, argv);

	if (options.get_sorted() == false and options.get_enumerate() == "best") {
		cerr << "error: cannot compute best solutions in the non-sorted mode!" << endl;
		exit(EXIT_FAILURE);
	}

	if (options.get_help_flag()) {
		show_info("");
		options.print(cout, argv[0]);
		exit(EXIT_SUCCESS);
	}

	if (options.get_timeout()) {
		init_alarm(options.get_timeout());
		signal(SIGALRM, SIG_alarm_handler);
	}

	if (options.get_verb())
		show_info("c ");

	run_tool(options);

	exit(EXIT_SUCCESS);
}
