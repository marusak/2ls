#include <util/simplify_expr.h>
#include <util/cprover_prefix.h>

#include <goto-symex/adjust_float_expressions.h>
#include "lexlinrank_solver_enumeration.h"
#include "util.h"
bool lexlinrank_solver_enumerationt::iterate(invariantt &_rank)
{

  lexlinrank_domaint::templ_valuet &rank=
    static_cast<lexlinrank_domaint::templ_valuet &>(_rank);

  bool improved=false;

  solver.new_context();

  domain.pre_iterate_init(_rank);

  // Entry value constraints
  exprt pre_expr=domain.to_pre_constraints(_rank);
  solver << pre_expr;

  exprt::operandst strategy_cond_exprs;
  domain.make_not_post_constraints(_rank,strategy_cond_exprs);

  domain.strategy_cond_literals.resize(strategy_cond_exprs.size());

  for(std::size_t i=0; i<strategy_cond_exprs.size(); i++)
  {
    domain.strategy_cond_literals[i]=solver.convert(strategy_cond_exprs[i]);
  }

  exprt cond=disjunction(strategy_cond_exprs);
  adjust_float_expressions(cond, ns);
  solver << cond;

  if(solver()==decision_proceduret::D_SATISFIABLE)
  {
    for(std::size_t row=0; row<domain.strategy_cond_literals.size(); row++)
    {
      if(solver.solver->l_get(domain.strategy_cond_literals[row]).is_true())
      {
        //get and edit_row from now on
        std::vector<exprt> required_values = domain.get_required_values(row);
        std::vector<exprt> got_values;
        for(auto &c_exprt : required_values) {
            got_values.push_back(solver.solver->get(c_exprt));
        }
        domain.set_values(got_values);
        exprt value=got_values[0];
        improved = domain.edit_row(row, value, _rank, improved);
      }
    }
  }
  else
  {
    debug() << "Outer solver: UNSAT!!" << eom;
    domain.not_satisfiable();
  }
  solver.pop_context();
  return improved;
}
