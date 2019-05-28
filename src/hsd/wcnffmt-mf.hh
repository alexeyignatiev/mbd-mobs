//jpms:bc
/*----------------------------------------------------------------------------*\
 * File:        wcnffmt-mf.hh
 *
 * Description: Class definitions for WCNF parser with observations, based
 *              extensively on MiniSAT parser, but using the STL.
 *              NOTE: When linking, option -lz *must* be used
 *
 * Author:      aign
 *
 *                                     Copyright (c) 2016, Alexey Ignatiev
\*----------------------------------------------------------------------------*/
//jpms:ec

#ifndef _WCNFFMT_MF_HH
#define _WCNFFMT_MF_HH 1

#include <ctime>
#include <cmath>
#include <unistd.h>
#include <signal.h>
#include <zlib.h>
#include <vector>

#include "globals.hh"
#include "vid_manager.hh"
#include "fmtutils.hh"

using namespace std;


//jpms:bc
/*----------------------------------------------------------------------------*\
 * WCNF Parser: (This borrows **extensively** from the MiniSAT parser)
\*----------------------------------------------------------------------------*/
//jpms:ec

/** Template parameters: B - input stream
 */
template<class B>
static void read_wcnf_clause(B& in,XLINT& clw,ULINT& mxid,vector<LINT>& lits) {
  clw = FMTUtils::parseLongInt(in);  // Read clause weight
  LINT parsed_lit;
  lits.clear();
  for (;;) {
    parsed_lit = FMTUtils::parseInt(in);
    if (parsed_lit == 0) break;
    if (fabs(parsed_lit) > mxid) { mxid = fabs(parsed_lit); }
    lits.push_back(parsed_lit);
  }
}

template<class B>
static void read_observations(B& in, vector<int>& obs) {
  ++in;  // skip 'o'
  FMTUtils::skipTabSpace(in);
  int parsed_o;
  obs.clear();
  for (;;) {
    parsed_o = FMTUtils::parseInt(in);
    if (parsed_o == 0) break;
    obs.push_back(parsed_o);
  }
}

/** Template parameters: B - input stream, CSet - consumer of clauses
 */
template<class B, class CSet>
static void parse_wcnf_file(B& in, CSet& cset, vector<vector<int>>& obs) {
  VarIDManager& imgr = VarIDManager::instance();
  ULINT mxid = 0;
  ULINT clid = 0;       // clause's index in the input file
  vector<LINT> lits;
  bool hasfmt = false;
  for (;;){
    FMTUtils::skipWhitespace(in);
    if (*in == EOF)
      break;
    else if (*in == 'c')
      FMTUtils::skipLine(in);
    else if (*in == 'o') {
      vector<int> o;
      read_observations(in, o);
      obs.push_back(o);
    }
    else if (*in == 'p' && !hasfmt) {
      ++in;
      // Either read 2 or 3 integers, depending on new line
      FMTUtils::skipTabSpace(in);
      string fmt = FMTUtils::readString(in);
      if (fmt != "wcnf") {
        cerr << "PARSE ERROR! Unexpected format: " << fmt << endl; exit(3); }
      FMTUtils::skipTabSpace(in);
      LINT intcnt = 1;
      while (*in != '\n' && *in != '\r') {
        XLINT ival = FMTUtils::parseLongInt(in);
        if (intcnt == 1)      { cset.set_num_vars(ToLint(ival)); }
        else if (intcnt == 2) { cset.set_num_cls(ToLint(ival)); }
        else if (intcnt == 3) { cset.set_topw(ival); }    // Set top weight
        else {
          LINT lval = ToLint(ival);
          fprintf(stderr, "PARSE ERROR! Unexpected int: %ld\n", (long int)lval), exit(3);
        } ++intcnt; FMTUtils::skipTabSpace(in); }
      ++in;
      hasfmt = true;
      if (not cset.has_topw()) { cset.set_topw(MAXLINT); }
    } else {
      XLINT clw;
      read_wcnf_clause(in, clw, mxid, lits);
      if (cset.get_topw() && clw > cset.get_topw()) { clw = cset.get_topw(); }
      cset.register_clause(lits, ++clid, clw);
    }
  }
  imgr.set_id(mxid);   // Register used IDs
}

/** Template parameters: CSet -- consumer of clauses
 */
template<class CSet>
class MWCNFParserTmpl {
public:
  inline void load_wcnf_file(gzFile input_stream, CSet& cset, vector<vector<int>>& obs) {
    StreamBuffer in(input_stream);
    parse_wcnf_file(in, cset, obs);
  }
};

// definition of CNFParser, for backward compatibility
class SimpleClauseSet;
typedef MWCNFParserTmpl<SimpleClauseSet> MWCNFParser;

#endif /* _WCNFFMT_MF_HH */

/*----------------------------------------------------------------------------*/
