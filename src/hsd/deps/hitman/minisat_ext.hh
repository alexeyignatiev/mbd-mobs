/*
 * minisat_ext.hh
 *
 *  Created on: Dec 11, 2012
 *      Author: Alexey S. Ignatiev
 *      E-mail: aignatiev@ciencias.ulisboa.pt
 */

#ifndef MINISAT_EXT_HH_
#define	MINISAT_EXT_HH_

#include <vector>
#include "solver.hh"

// #define MINISAT_GH

using namespace std;

using Minisat::lbool;
using Minisat::Lit;
using Minisat::mkLit;
using Minisat::Solver;
using Minisat::Var;
using Minisat::vec;

namespace Minisat {
	class MiniSatExt : public Solver {
	public:
		inline void bump(const Var var) { varBumpActivity(var); }
		void new_variables(const int max_id) {
			while (nVars() < max_id + 1)
				newVar();
		}

		void add_clause(const vec<Lit>& cl)
		{
			int max_id = var(cl[0]);
			for (int i = 1; i < cl.size(); ++i) {
				if (var(cl[i]) > max_id)
					max_id = var(cl[i]);
			}

			new_variables(max_id);
			addClause(cl);
		}

		void add_clause(const vector<int>& cl)
		{
			vec<Lit> cl_new(cl.size());

			int max_id = 0;
			for (size_t i = 0; i < cl.size(); ++i) {
				cl_new[i] = cl[i] >= 0 ? mkLit(cl[i]) : ~mkLit(-cl[i]);

				if (var(cl_new[i]) > max_id)
					max_id = var(cl_new[i]);
			}

			new_variables(max_id);
			addClause(cl_new);
		}

		void dump()
		{
			vec<Lit> dummy;
			toDimacs(stdout, dummy);
		}

		bool solve_ext(const vec<Lit>& assumps, const vec<Lit>& extra_assumps)
		{
			vec<Lit> ass;
			assumps.copyTo(ass);

			for (int i = 0; i < extra_assumps.size(); ++i)
				ass.push(extra_assumps[i]);

			return solve(ass);
		}

		void set_polarity(Var v, bool p)
		// p == true  => negative literal
		// p == false => positive literal
		{
			#ifdef MINISAT_GH
				setPolarity(v, p == true ? l_True : l_False);
			#else
				setPolarity(v, p);
			#endif
		}

		void get_ucore(vec<Lit>& ucore)
		{
			// assert(conflict.size());

			ucore.clear();
			for (int i = 0; i < conflict.size(); ++i)
				ucore.push(conflict[i]);
		}
	};
}

#endif	/* MINISAT_EXT_HH_ */
