#include <ssa/ssa_inliner.h>
#include "strategy_solver.h"

bool strategy_solver::iterate(invariantt &inv)
{
  bool improved=false;

  solver.new_context();

  // Entry value constraints
  exprt pre_expr=domain.to_pre_constraints(inv);
  solver << pre_expr;

  // Exit value constraints
  exprt::operandst strategy_cond_exprs;
  domain.make_not_post_constraints(inv, strategy_cond_exprs);

  domain.strategy_cond_literals.resize(strategy_cond_exprs.size());

  for(std::size_t i=0; i<strategy_cond_exprs.size(); ++i)
  {
    domain.strategy_cond_literals[i]=solver.convert(strategy_cond_exprs[i]);
    strategy_cond_exprs[i]=literal_exprt(domain.strategy_cond_literals[i]);
  }
  solver << disjunction(strategy_cond_exprs);

  if(solver()==decision_proceduret::D_SATISFIABLE)
  {
    for(std::size_t row=0; row<domain.strategy_cond_literals.size(); ++row)
    {
      if(solver.l_get(domain.strategy_cond_literals[row]).is_true())
      {
        exprt value=solver.get(domain.strategy_value_exprs[row]);
        improved = domain.edit_row(row, value, inv, improved);
      }
    }
  }
  solver.pop_context();

  return improved;
}
