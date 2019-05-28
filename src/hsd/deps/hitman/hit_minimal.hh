/*
 * hit_minimal.hh
 *
 *  Created on: Nov 26, 2015
 *      Author: Alexey S. Ignatiev
 *      E-mail: aignatiev@ciencias.ulisboa.pt
 */

#ifndef HIT_MINIMAL_HH_
#define	HIT_MINIMAL_HH_

#include "hitman.hh"

class HitMinimal : public Hitman {
public:
	enum MCSAlgoType {
		atype_lbx = 0,
		atype_ucd,
		atype_ubs,
		atype_lopz
	};

	HitMinimal();

	virtual ~HitMinimal()
	{}

	virtual vector<int> get   ();
	virtual void        clear ();

	// set MCS extraction algorithm (default is LBX)
	void set_param_algo(string algo)
	{
		if      (algo.compare("lbx" ) == 0)
			param_atype = atype_lbx;
		else if (algo.compare("ucd" ) == 0)
			param_atype = atype_ucd;
		else if (algo.compare("ubs" ) == 0)
			param_atype = atype_ubs;
		else if (algo.compare("lopz") == 0)
			param_atype = atype_lopz;
	}

	// make D calls while performing MCS extraction
	void set_param_dcall()
	{
		param_dcall = true;
	}
protected:
	// compute a minimal hitting set
	void compute ();

	// one of the following methods is actually used
	void lbx_compute  ();
	void ucd_compute  ();
	void ubs_compute  ();
	void lopz_compute ();

	// compute an overapproximation of an MCS
	// this is done by calling a SAT solver on
	// the hard part of the MaxSAT formula
	void overapprox ();

	// do clause D check (check disjunction of literals in set D)
	void do_cld_check (vec<Lit>&);

	// save the oracle's model to the local "model" variable
	void get_model()
	{
		model.resize(oracle->model.size());

		for (size_t i = 0; i < model.size(); ++i)
			model[i] = oracle->model[i];
	}

	MCSAlgoType   param_atype;  // algorithm type (lbx, ucd, ubs, or lopz)
	bool          param_dcall;  // use clause D calls
	vec<Lit>      bb_assumps;   // backbone literal assumptions
	                            // basically, this set represents a solution
	vec<Lit>      ss_assumps;   // satisfied soft clause assumptions
	vector<Lit>   setd;         // current state of literals under question
	vector<lbool> model;        // current model of the hard part
private:
};

#endif	/* HIT_MINIMAL_HH_ */
