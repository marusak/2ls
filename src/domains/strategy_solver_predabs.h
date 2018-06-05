/*******************************************************************\

Module: Solver for predicate abstraction domain

Author: Peter Schrammel, Cristina David

\*******************************************************************/

#ifndef CPROVER_2LS_DOMAINS_STRATEGY_SOLVER_PREDABS_H
#define CPROVER_2LS_DOMAINS_STRATEGY_SOLVER_PREDABS_H

#include "strategy_solver_base.h"
#include "predabs_domain.h"

class strategy_solver_predabst:public strategy_solver_baset
{
 public:
  explicit strategy_solver_predabst(
    predabs_domaint &_predabs_domain,
    incremental_solvert &_solver,
    const namespacet &_ns):
    strategy_solver_baset(_solver, _ns),
    predabs_domain(_predabs_domain)
  {
    predabs_domain.get_row_set(predabs_domain.todo_preds);
  }

  virtual bool iterate(invariantt &inv);

 protected:
  predabs_domaint &predabs_domain;
};

#endif
