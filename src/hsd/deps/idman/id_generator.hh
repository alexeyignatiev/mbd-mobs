//jpms:bc
/*----------------------------------------------------------------------------*\
 * File:        id_generator.hh
 *
 * Description: 
 *
 * Author:      jpms
 * 
 *                                     Copyright (c) 2014, Joao Marques-Silva
\*----------------------------------------------------------------------------*/
//jpms:ec

#ifndef _ID_GENERATOR_H
#define _ID_GENERATOR_H 1

#include "globals.hh"


/*----------------------------------------------------------------------------*\
 * Class: IDGenerator
 *
 * Purpose: Abstract generator of var IDs.
\*----------------------------------------------------------------------------*/

class IDGenerator {

public:

  IDGenerator() { }

  virtual ~IDGenerator() { }

  virtual ULINT new_id() = 0;

};

#endif /* _ID_GENERATOR_H */

/*----------------------------------------------------------------------------*/
