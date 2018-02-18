#include <util/simplify_expr.h>
#include <util/find_symbols.h>

#include "strategy_solver_enumeration.h"
#include "util.h"

bool strategy_solver_enumerationt::iterate(invariantt &_inv)
{
  tpolyhedra_domaint::templ_valuet &inv=static_cast<tpolyhedra_domaint::templ_valuet &>(_inv);

  bool improved=false;

  solver.new_context();

  exprt pre_expr=tpolyhedra_domain.to_pre_constraints(inv);
  solver << pre_expr;

  exprt::operandst strategy_cond_exprs;
  tpolyhedra_domain.make_not_post_constraints(inv,
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
        tpolyhedra_domain.edit_row(row, value, inv);
      }
    }
    improved=true;
  }
  solver.pop_context();
  return improved;
}
