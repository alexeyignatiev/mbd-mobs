//jpms:bc
/*----------------------------------------------------------------------------*\
 * File:        simple_clset.hh
 *
 * Description: Simple implementation of a clause set.
 *
 * Author:      jpms
 * 
 *                                     Copyright (c) 2012, Joao Marques-Silva
\*----------------------------------------------------------------------------*/
//jpms:ec

#ifndef _SIMPLE_CLAUSE_SET_H
#define _SIMPLE_CLAUSE_SET_H 1

#include "globals.hh"
#include "vid_manager.hh"

class SimpleClauseSet;

//jpms:bc
/*----------------------------------------------------------------------------*\
 * Data associated with each clause: literals, cl ID and cl weight
\*----------------------------------------------------------------------------*/
//jpms:ec

class ClauseData {
  friend class SimpleClauseSet;

public:

  LINT clid() { return _clid; }

  IntVector& clits() { return *_clits; }

  XLINT clw() { return _clw; }

protected:

  ClauseData(LINT cid, IntVector& lits, XLINT cw=1) :
    _clid(cid), _clits(NULL), _clw(cw) {
    _clits =  new IntVector();
    _clits->resize(lits.size(), 0);
    copy(lits.begin(), lits.end(), _clits->begin());
  }

  ~ClauseData() { _clits->clear(); delete _clits; }

protected:

  LINT _clid;

  IntVector* _clits;

  XLINT _clw;

};

typedef hash_map<LINT,ClauseData*,IntHash,IntEqual> Int2ClDataMap;
typedef hash_map<LINT,ClauseData*,IntHash,IntEqual>::iterator Int2ClDataMapIterator;
typedef vector<ClauseData*> ClDataVect;
typedef vector<ClauseData*>::iterator ClDataVectIterator;


//jpms:bc
/*----------------------------------------------------------------------------*\
 * Imports clauses to data structure, with or without weights. Clauses are
 * then added assumption variables and exported to SAT solver. A map from
 * assumptions to clauses is populated and returned.
\*----------------------------------------------------------------------------*/
//jpms:ec

class SimpleClauseSet {

public:

  SimpleClauseSet() : clauses(), a2cld_map(), topw(1), topwok(false) { }

  ~SimpleClauseSet() {
    assert(clauses.size() == 0 || a2cld_map.size() == 0);
    // Delete existing map
    if (a2cld_map.size() > 0) {
      Int2ClDataMap::iterator apos = a2cld_map.begin();
      Int2ClDataMap::iterator aend = a2cld_map.end();
      for(; apos != aend; ++apos) { delete apos->second; }
      a2cld_map.clear();
    }
    // Delete any other clauses
    if (clauses.size() > 0) {
      ClDataVect::iterator vpos = clauses.begin();
      ClDataVect::iterator vend = clauses.end();
      for(; vpos!=vend; ++vpos) { delete *vpos; }
      clauses.clear();
    }
  }

  void set_num_vars(LINT nv) { }

  void set_num_cls(LINT nc) { }

  // Declare clause, without knowing the range of assumption variables
  void register_clause(vector<LINT>& lits, LINT clid, XLINT clw=1) {
    clauses.push_back(new ClauseData(clid, lits, clw));
  }

  // Add clauses to solver. For hard clauses, delete clause literals
  template <class Solver>
  void export_clauses(Solver& solver) {
    VarIDManager& imgr = VarIDManager::instance();
    DBG(cout << "Top ID: " << imgr.top_id() << endl;);
    if (topw == 1) { set_topw(clauses.size()+1); }
    ClDataVect::iterator vpos = clauses.begin();
    ClDataVect::iterator vend = clauses.end();
    for(; vpos!=vend; ++vpos) {
      if ((*vpos)->clw() >= topw) {    // Hard clauses are final
	solver.add_final_clause((*vpos)->clits());
	delete *vpos;
	*vpos = NULL;
      }
      else {
	LINT vid = imgr.new_id();
	a2cld_map[vid] = *vpos;
	solver.add_clause(vid, (*vpos)->clits());
      }
    }
    clauses.clear();
    assert(clauses.size() == 0);
  }

  // Add clause. For hard clauses, delete clause literals
  template <class Solver>
  void add_clause(Solver& solver, IntVector& clits, LINT clid=0, XLINT clw=1) {
    VarIDManager& imgr = VarIDManager::instance();
    if (clw == topw) {
      solver.add_final_clause(clits);
    }
    else {
      LINT vid = imgr.new_id();
      ClauseData* cld = new ClauseData(clid, clits, clw);
      a2cld_map[vid] = cld;
      solver.add_clause(vid, clits);
    }
    clits.clear();
  }

  // Add final hard clause to solver, keep clause literals (if keepcl)
  template <class Solver>
  void add_final_clause(Solver& solver, IntVector& clits, bool keepcl=false) {
    solver.add_final_clause(clits);
    if (keepcl) {
      clauses.push_back(new ClauseData(0, clits, topw));
    }
    clits.clear();
  }

  void attach_clause(LINT vid, ClauseData& cld) {
    a2cld_map[vid] = &cld;
  }

  ClauseData& detach_clause(LINT vid) {
    ClauseData* cld = NULL;
    Int2ClDataMap::iterator mpos = a2cld_map.find(vid);
    if (mpos != a2cld_map.end()) {
      cld = mpos->second;
      a2cld_map.erase(mpos);
    }
    return *cld;
  }

  void detach_all_clauses(SimpleClauseSet& cset2) {
    Int2ClDataMapIterator impos = map_begin();
    Int2ClDataMapIterator imend = map_end();
    for(; impos != imend; ++impos) {
      cset2.attach_clause(impos->first, *impos->second);
    }
    a2cld_map.clear();
  }

  void detach_all_clauses() { a2cld_map.clear(); }

  void move_all_clauses(SimpleClauseSet& cset2) { detach_all_clauses(cset2); }

  void copy_all_clauses(SimpleClauseSet& cset2) {
    Int2ClDataMapIterator impos = map_begin();
    Int2ClDataMapIterator imend = map_end();
    for(; impos != imend; ++impos) {
      cset2.attach_clause(impos->first, *impos->second);
    }
  }

  void copy_all_cl_refs(LINTSet& cset) {
    Int2ClDataMapIterator impos = map_begin();
    Int2ClDataMapIterator imend = map_end();
    for(; impos != imend; ++impos) { cset.insert(impos->first); }
  }

  void set_topw(XLINT mw) { topw = mw; topwok = true; }

  XLINT get_topw() { return topw; }

  bool has_topw() { return topwok; }

  IntVector& clits(LINT vid) {
    assert(a2cld_map.find(vid) != a2cld_map.end());
    return a2cld_map[vid]->clits();
  }

  XLINT clw(LINT vid) {
    assert(a2cld_map.find(vid) != a2cld_map.end());
    return a2cld_map[vid]->clw();
  }

  LINT clid(LINT vid) {
    assert(a2cld_map.find(vid) != a2cld_map.end());
    return a2cld_map[vid]->clid();
  }

  LINT map_size() { return a2cld_map.size(); }

  Int2ClDataMap& a2c_map() { return a2cld_map; }

  Int2ClDataMapIterator map_begin() { return a2cld_map.begin(); }

  Int2ClDataMapIterator map_end() { return a2cld_map.end(); }

  ClauseData& clause(LINT sid) {
    Int2ClDataMapIterator cpos = a2cld_map.find(sid);
    assert(cpos != a2cld_map.end());
    return *(cpos->second);
  }

  void map_clear() { a2cld_map.clear(); }

  ClDataVect& get_clauses() { return clauses; }

  LINT vect_size() { return clauses.size(); }

  ClDataVectIterator vect_begin() { return clauses.begin(); }

  ClDataVectIterator vect_end() { return clauses.end(); }

  void dump(ostream& outs=cout) {
    outs << "Clauses with assumptions ";
    Int2ClDataMap::iterator vpos = a2cld_map.begin();
    Int2ClDataMap::iterator vend = a2cld_map.end();
    for(; vpos != vend; ++vpos) {
      outs << vpos->second->clid()<< ": [";
      outs << vpos->first << ": [";
      IntVector& pv = vpos->second->clits();
      IntVector::iterator lpos = pv.begin();
      IntVector::iterator lend = pv.end();
      for(; lpos!=lend; ++lpos) {
	outs << *lpos << " ";
      }
      outs << "]" << endl;
      outs << "; clw=" << vpos->second->clw();
      outs << "; aid=" << vpos->first << endl;
    }
  }

  friend ostream & operator << (ostream& outs, SimpleClauseSet& clset) {
    clset.dump(outs); return outs; }

protected:

  vector<ClauseData*> clauses;

  Int2ClDataMap a2cld_map;

  XLINT topw;

  bool topwok;

};

typedef vector<SimpleClauseSet*> SimpleClSetVect;
typedef vector<SimpleClauseSet*>::iterator SimpleClauseSetIterator;

#endif /* _SIMPLE_CLAUSE_SET_H */

/*----------------------------------------------------------------------------*/
