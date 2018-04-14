/*******************************************************************\

Module: Enumeration-based solver for lexicographic linear ranking 
        functions

Author: Peter Schrammel

\*******************************************************************/

#ifndef CPROVER_2LS_DOMAINS_LEXLINRANK_SOLVER_ENUMERATION_H
#define CPROVER_2LS_DOMAINS_LEXLINRANK_SOLVER_ENUMERATION_H

#include <ssa/local_ssa.h>
#include "strategy_solver_base.h"
#include "../domains/incremental_solver.h"
#include "lexlinrank_domain.h"
#include <solvers/sat/satcheck.h>
#include <solvers/flattening/bv_pointers.h>


class lexlinrank_solver_enumerationt:public strategy_solver_baset
{
 public:
  explicit lexlinrank_solver_enumerationt(
    lexlinrank_domaint &_domain,
    incremental_solvert &_solver,
    const local_SSAt &SSA,
    const exprt &precondition,
    message_handlert &message_handler,
    template_generator_baset &template_generator):
    strategy_solver_baset(_solver, SSA.ns),
    domain(_domain)
  {
    set_message_handler(message_handler);
    solver << domain.initialize_solver(SSA, precondition, template_generator);
    delete domain.inner_solver;
    domain.inner_solver=incremental_solvert::allocate(ns);
   }

  virtual bool iterate(invariantt &inv);

 protected:
  lexlinrank_domaint &domain;
};

#endif
