/*******************************************************************\

Module: Strategy solver for combination of domains.

Author: Matej Marusak

\*******************************************************************/

#ifndef CPROVER_2LS_DOMAINS_STRATEGY_SOLVER_COMBINATION_H
#define CPROVER_2LS_DOMAINS_STRATEGY_SOLVER_COMBINATION_H


#include "strategy_solver_base.h"
#include "combination_domain.h"
#include "strategy_solver.h"

class strategy_solver_combinationt:public strategy_solver_baset
{
public:
  std::vector<strategy_solver_baset*> solvers;
  strategy_solver_combinationt(
    combination_domaint &_combination_domain,
    incremental_solvert &_solver,
    const local_SSAt &SSA,
    const exprt &precondition,
    message_handlert &message_handler,
    std::vector<strategy_solver_baset*> _solvers,
    template_generator_baset &template_generator):
    strategy_solver_baset(_solver, SSA.ns),
    domain(_combination_domain)
  {
  }

  virtual bool iterate(invariantt &_inv) override;

  virtual void set_message_handler(message_handlert &_message_handler) override;
  virtual void clear_symbolic_path();

protected:
  combination_domaint &domain;
};


#endif
