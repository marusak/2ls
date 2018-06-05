#include <util/simplify_expr.h>
#include "strategy_solver_predabs.h"
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

//--------------------
    exprt strategy_cond_expr;
    strategy_cond_expr=
      predabs_domain.get_row_post_constraint(*(predabs_domain.e_it), true_exprt());

    literalt cond_literal=solver.convert(not_exprt(strategy_cond_expr));
    solver << literal_exprt(cond_literal);

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
