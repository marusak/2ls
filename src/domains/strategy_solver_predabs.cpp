#include <util/simplify_expr.h>
#include "strategy_solver_predabs.h"
#include <goto-symex/adjust_float_expressions.h>
bool strategy_solver_predabst::iterate(invariantt &inv)
{
  bool improved=false;

  predabs_domain.pre_iterate_init(inv);
  if (predabs_domain.nothing_to_solve())
      return improved;

  solver.new_context();

  // Entry value constraints
  exprt pre_expr=predabs_domain.to_pre_constraints(inv);
  solver << pre_expr;

  exprt::operandst strategy_cond_exprs;

  predabs_domain.make_not_post_constraints(inv, strategy_cond_exprs);

  predabs_domain.strategy_cond_literals.resize(strategy_cond_exprs.size());

  for(std::size_t i=0; i<strategy_cond_exprs.size(); ++i)
  {
    predabs_domain.strategy_cond_literals[i]=solver.convert(strategy_cond_exprs[i]);
  }

  exprt cond=disjunction(strategy_cond_exprs);
  adjust_float_expressions(cond, ns);
  solver << cond;

    if(solver()==decision_proceduret::D_SATISFIABLE)
    {
      debug() << "SAT" << eom;
      predabs_domain.todo_notpreds.insert(*(predabs_domain.e_it));

      solver.pop_context();
    }
    else
    {
      debug() << "UNSAT" << eom;
      predabs_domain.set_row_value(*(predabs_domain.e_it), true_exprt(), inv);
      solver.pop_context();
      solver << pre_expr; // make permanent
      predabs_domain.todo_preds.insert(predabs_domain.todo_notpreds.begin(), predabs_domain.todo_notpreds.end());
      predabs_domain.todo_notpreds.clear();
    }

    predabs_domain.todo_preds.erase(predabs_domain.e_it);

    return true;
}
