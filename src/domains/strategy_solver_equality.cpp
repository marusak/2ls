#include <util/simplify_expr.h>
#include "strategy_solver_equality.h"
bool strategy_solver_equalityt::iterate(invariantt &_inv)
{
  // THIS THEN REMOVE
  equality_domaint::equ_valuet &inv=
    static_cast<equality_domaint::equ_valuet &>(_inv);

  bool improved=false;

  equality_domain.pre_iterate_init(_inv);
  if (!equality_domain.nothing_to_solve()){

  solver.new_context();



  // Entry value constraints
  exprt pre_expr=equality_domain.to_pre_constraints(inv);
  solver << pre_expr;

  /*
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
*/



///
    exprt post_expr=equality_domain.get_post_not_equ_constraint(*(equality_domain.e_it));
    literalt cond_literal=solver.convert(post_expr);

    solver << literal_exprt(cond_literal);
///

    if(solver()==decision_proceduret::D_SATISFIABLE)
    {
      equality_domain.todo_disequs.insert(*(equality_domain.e_it));

      solver.pop_context();
    }
    else  // equality holds
    {
      equality_domain.set_equal(*(equality_domain.e_it), inv);

      solver.pop_context();

      solver << pre_expr; // make permanent

      // due to transitivity, we have to recheck equalities
      //   that did not hold
      equality_domain.todo_equs.insert(equality_domain.todo_disequs.begin(), equality_domain.todo_disequs.end());
      equality_domain.todo_disequs.clear();
    }

    equality_domain.todo_equs.erase(equality_domain.e_it);
  }
  else // check disequalities
  {
    equality_domain.e_it=equality_domain.todo_disequs.begin();
    if(equality_domain.e_it==equality_domain.todo_disequs.end())
      return false; // done

    solver.new_context();

    exprt pre_expr=equality_domain.get_pre_disequ_constraint(*(equality_domain.e_it));

    solver << pre_expr;

    exprt post_expr=equality_domain.get_post_not_disequ_constraint(*(equality_domain.e_it));
    literalt cond_literal=solver.convert(post_expr);

    solver << literal_exprt(cond_literal);
    if(solver()==decision_proceduret::D_SATISFIABLE)
    {
    }
    else  // equality holds
    {
      equality_domain.set_disequal(*(equality_domain.e_it), inv);
      solver << pre_expr; // make permanent
    }

    solver.pop_context();

    equality_domain.todo_disequs.erase(equality_domain.e_it);
  }
  return true;
}
