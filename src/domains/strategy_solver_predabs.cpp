#include <util/simplify_expr.h>
#include "strategy_solver_predabs.h"
#include <goto-symex/adjust_float_expressions.h>
bool strategy_solver_predabst::iterate(invariantt &inv)
{
  bool improved=false;

  predabs_domain.pre_iterate_init(inv);
  if (predabs_domain.something_to_solve()){
      improved = true;
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
            for(std::size_t row=0; row<predabs_domain.strategy_cond_literals.size(); ++row)
            {
              if(solver.l_get(predabs_domain.strategy_cond_literals[row]).is_true())
              {
                //Find what values from solver are needed
                std::vector<exprt> required_values = predabs_domain.get_required_values(row);
                std::vector<exprt> got_values;
                for(auto &c_exprt : required_values) {
                    got_values.push_back(solver.solver->get(c_exprt));
                }
                predabs_domain.set_values(got_values);

                improved = predabs_domain.edit_row(row, inv, improved);
              }
            }
            solver.pop_context(); // THIS IS HERE SURPLUS
          }
        else  // equality holds
        {
          solver.pop_context(); // THIS IS SURPLUS
          predabs_domain.not_satisfiable(inv);
        }
        predabs_domain.post_edit();
    }
    return improved;
}
