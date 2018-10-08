#include "strategy_solver_heap.h"

#include <ssa/ssa_inliner.h>
#include "strategy_solver.h"
#include <goto-symex/adjust_float_expressions.h>

bool strategy_solver_heapt::iterate(invariantt &inv)
{
  bool improved=false;

  heap_domain.pre_iterate_init(inv);
  if (heap_domain.something_to_solve()) {

      solver.new_context();

      // Entry value constraints
      exprt pre_expr=heap_domain.to_pre_constraints(inv);
      solver << pre_expr;

      // Exit value constraints
      exprt::operandst strategy_cond_exprs;
      heap_domain.make_not_post_constraints(inv, strategy_cond_exprs);

      heap_domain.strategy_cond_literals.resize(strategy_cond_exprs.size());

      for(std::size_t i=0; i<strategy_cond_exprs.size(); ++i)
      {
        heap_domain.strategy_cond_literals[i]=solver.convert(strategy_cond_exprs[i]);
      }

      exprt cond=disjunction(strategy_cond_exprs);
      adjust_float_expressions(cond, ns);
      solver << cond;

      if(solver()==decision_proceduret::D_SATISFIABLE)
      {
        for(std::size_t row=0; row<heap_domain.strategy_cond_literals.size(); ++row)
        {
          if(solver.l_get(heap_domain.strategy_cond_literals[row]).is_true())
          {
            //Find what values from solver are needed
            std::vector<exprt> required_values = heap_domain.get_required_values(row);
            std::vector<exprt> got_values;
            for(auto &c_exprt : required_values) {
                got_values.push_back(solver.solver->get(c_exprt));
            }
            heap_domain.set_values(got_values);

            improved = heap_domain.edit_row(row, inv, improved, solver);
          }
        }
      }
      else
      {
        debug() << "Outer solver: UNSAT!!" << eom;
        improved = heap_domain.not_satisfiable(inv, improved);
      }
    solver.pop_context();
    solver << heap_domain.make_permanent(inv);
    heap_domain.post_edit();
  }
  return improved;
}
