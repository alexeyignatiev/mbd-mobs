//jpms:bc
/*----------------------------------------------------------------------------*\
 * File:        vid_manager.hh
 *
 * Description: Manager for variable IDs.
 *
 * Author:      jpms
 * 
 *                                     Copyright (c) 2012, Joao Marques-Silva
\*----------------------------------------------------------------------------*/
//jpms:ec

#ifndef _VID_MANAGER_H
#define _VID_MANAGER_H 1

#include "globals.hh"
#include "id_manager.hh"


/*----------------------------------------------------------------------------*\
 * Class: VarIDManager
 *
 * Purpose: Manager of var IDs.
\*----------------------------------------------------------------------------*/

class VarIDManager : public IDManager {

public:

  inline ULINT new_id() { return ++_id; }

  inline void set_id(ULINT nmax) { if (_id < nmax) { _id = nmax; } }

  inline ULINT top_id() { return _id; }

  inline void clear() { _id = 0; }

  static VarIDManager& instance() { 
    if (_instance == NULL) { _instance = new VarIDManager(); }
    return *_instance;
  }

  void dump(ostream& outs=cout) {
    outs << "Var ID Manager used IDs: " << _id << endl; }

  friend ostream & operator << (ostream& outs, VarIDManager& vimgr) {
    vimgr.dump(outs); return outs; }

protected:

  VarIDManager() : _id(0) { }

  virtual ~VarIDManager() { }

private:

  ULINT _id;                       // Current id

  static VarIDManager* _instance;  // Reference to object

};

#endif /* _VID_MANAGER_H */

/*----------------------------------------------------------------------------*/
