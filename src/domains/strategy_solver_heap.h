/*******************************************************************\

Module: Strategy solver for heap shape analysis

Author: Viktor Malik

\*******************************************************************/
#ifndef CPROVER_2LS_DOMAINS_STRATEGY_SOLVER_HEAP_H
#define CPROVER_2LS_DOMAINS_STRATEGY_SOLVER_HEAP_H

#include <ssa/local_ssa.h>
#include "strategy_solver_base.h"
#include "heap_domain.h"
#include "template_generator_base.h"

class strategy_solver_heapt:public strategy_solver_baset
{
public:
  strategy_solver_heapt(
    heap_domaint &_heap_domain,
    incremental_solvert &_solver,
    const local_SSAt &SSA,
    const exprt &precondition,
    message_handlert &message_handler,
    template_generator_baset &template_generator):
    strategy_solver_baset(_solver, SSA.ns),
    domain(_heap_domain)//this
  {
    set_message_handler(message_handler);
    solver<< domain.initialize_solver(SSA, precondition, template_generator);
  }

  virtual bool iterate(invariantt &_inv) override;


protected:
  heap_domaint &domain;
};


#endif // CPROVER_2LS_DOMAINS_STRATEGY_SOLVER_HEAP_H
