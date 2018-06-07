#include <util/simplify_expr.h>
#include "strategy_solver_equality.h"
#include <goto-symex/adjust_float_expressions.h>
bool strategy_solver_equalityt::iterate(invariantt &_inv)
{
  // THIS THEN REMOVE
  equality_domaint::equ_valuet &inv=
    static_cast<equality_domaint::equ_valuet &>(_inv);

  bool improved=false;

  equality_domain.pre_iterate_init(_inv);
  if (equality_domain.something_to_solve()){
      improved = true;

      solver.new_context();

      // Entry value constraints
      exprt pre_expr=equality_domain.to_pre_constraints(inv);
      solver << pre_expr;

      exprt::operandst strategy_cond_exprs;

      equality_domain.make_not_post_constraints(inv, strategy_cond_exprs);

      equality_domain.strategy_cond_literals.resize(strategy_cond_exprs.size());

      for(std::size_t i=0; i<strategy_cond_exprs.size(); ++i)
      {
        equality_domain.strategy_cond_literals[i]=solver.convert(strategy_cond_exprs[i]);
      }

      exprt cond=disjunction(strategy_cond_exprs);
      adjust_float_expressions(cond, ns);
      solver << cond;

        if(solver()==decision_proceduret::D_SATISFIABLE)
          {
            for(std::size_t row=0; row<equality_domain.strategy_cond_literals.size(); ++row)
            {
              if(solver.l_get(equality_domain.strategy_cond_literals[row]).is_true())
              {
                //Find what values from solver are needed
                std::vector<exprt> required_values = equality_domain.get_required_values(row);
                std::vector<exprt> got_values;
                for(auto &c_exprt : required_values) {
                    got_values.push_back(solver.solver->get(c_exprt));
                }
                equality_domain.set_values(got_values);

                improved = equality_domain.edit_row(row, inv, improved);
              }
            }
            if (!equality_domain.check_dis)
                solver.pop_context(); // THIS IS HERE SURPLUS
          }
        else  // equality holds
        {
          if (!equality_domain.check_dis){
            solver.pop_context(); // THIS IS HERE SURPLUS
          }
            solver << equality_domain.not_satisfiable(_inv);
        }

        //make it domian.post_edit()
        if (!equality_domain.check_dis)
            equality_domain.todo_equs.erase(equality_domain.e_it);
        else{
            solver.pop_context();
            equality_domain.todo_disequs.erase(equality_domain.e_it);
        }
    }
  return improved;
}
