/*******************************************************************\

Module: Solver for equalities/disequalities domain

Author: Peter Schrammel

\*******************************************************************/

#ifndef CPROVER_2LS_DOMAINS_STRATEGY_SOLVER_EQUALITY_H
#define CPROVER_2LS_DOMAINS_STRATEGY_SOLVER_EQUALITY_H

#include "strategy_solver_base.h"
#include "equality_domain.h"

class strategy_solver_equalityt:public strategy_solver_baset
{
public:
  strategy_solver_equalityt(
    equality_domaint &_equality_domain,
    incremental_solvert &_solver,
    const namespacet &_ns):
    strategy_solver_baset(_solver, _ns),
    equality_domain(_equality_domain)
  {
    equality_domain.get_index_set(equality_domain.todo_equs);
  }

  virtual bool iterate(invariantt &inv);

 protected:
  equality_domaint &equality_domain;

};

#endif
