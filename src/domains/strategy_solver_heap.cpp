#include <ssa/ssa_inliner.h>
#include "strategy_solver_heap.h"

bool strategy_solver_heapt::iterate(invariantt &_inv)
{
  heap_domaint::heap_valuet &inv=static_cast<heap_domaint::heap_valuet &>(_inv);

  bool improved=false;

  solver.new_context();

  // Entry value constraints
  exprt pre_expr=heap_domain.to_pre_constraints(inv);
  solver << pre_expr;

  // Exit value constraints
  exprt::operandst strategy_cond_exprs;
  heap_domain.make_not_post_constraints(inv,
                                        strategy_cond_exprs,
                                        strategy_value_exprs);

  strategy_cond_literals.resize(strategy_cond_exprs.size());

  for(std::size_t i=0; i<strategy_cond_exprs.size(); ++i)
  {
    strategy_cond_literals[i]=solver.convert(strategy_cond_exprs[i]);
    strategy_cond_exprs[i]=literal_exprt(strategy_cond_literals[i]);
  }
  solver << disjunction(strategy_cond_exprs);

  if(solver()==decision_proceduret::D_SATISFIABLE)
  {
    for(std::size_t row=0; row<strategy_cond_literals.size(); ++row)
    {
      if(solver.l_get(strategy_cond_literals[row]).is_true())
      {
        exprt value=solver.get(strategy_value_exprs[row]);
        improved = heap_domain.edit_row(row, value, inv, improved);
      }
    }
  }
  solver.pop_context();

  return improved;
}
