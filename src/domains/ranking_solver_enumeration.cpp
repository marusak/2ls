/*******************************************************************\

Module: Enumeration-based solver for linear ranking functions

Author: Peter Schrammel

\*******************************************************************/

#ifdef DEBUG
#include <iostream>
#endif

#include <util/simplify_expr.h>
#include <util/cprover_prefix.h>
#include <goto-symex/adjust_float_expressions.h>

#include "ranking_solver_enumeration.h"
#include "util.h"

/*******************************************************************\

Function: ranking_solver_enumerationt::iterate

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

bool ranking_solver_enumerationt::iterate(invariantt &_rank)
{
  linrank_domaint::templ_valuet &rank=
    static_cast<linrank_domaint::templ_valuet &>(_rank);

  bool improved=false;
  domain.pre_iterate_init(_rank);

  solver.new_context();

  exprt pre_expr=domain.to_pre_constraints(_rank);
  solver << pre_expr;

  // handles on values to retrieve from model
  exprt::operandst strategy_cond_exprs;

  domain.make_not_post_constraints(_rank, strategy_cond_exprs);

  domain.strategy_cond_literals.resize(strategy_cond_exprs.size());
  for(std::size_t i=0; i<domain.strategy_cond_literals.size(); ++i)
  {
    domain.strategy_cond_literals[i]=solver.convert(strategy_cond_exprs[i]);
  }

  exprt cond=disjunction(strategy_cond_exprs);
  adjust_float_expressions(cond, ns);
  solver << cond;

  if(solver()==decision_proceduret::D_SATISFIABLE)
  {
    debug() << "SAT" << eom;

    for(std::size_t row=0; row<domain.strategy_cond_literals.size(); ++row)
    {
      if(solver.solver->l_get(domain.strategy_cond_literals[row]).is_true())
      {
        //Find what values from solver are needed
        std::vector<exprt> required_values = domain.get_required_values(row);
        std::vector<exprt> got_values;
        for(auto &c_exprt : required_values) {
            got_values.push_back(solver.solver->get(c_exprt));
        }
        domain.set_values(got_values);

        exprt rounding_mode=symbol_exprt(CPROVER_PREFIX "rounding_mode", signedbv_typet(32));
        linrank_domaint::row_valuet symb_values;
        exprt constraint;
        exprt refinement_constraint;

        // generate the new constraint
        constraint=domain.get_row_symb_constraint(
          symb_values, row, refinement_constraint);
        simplify_expr(constraint, ns);
        debug() << "Inner Solver: " << row << " constraint "
                << from_expr(ns, "", constraint) << eom;

        inner_solver << equal_exprt(
          rounding_mode, from_integer(mp_integer(0), signedbv_typet(32)));
        inner_solver << constraint;

        // refinement
        if(!refinement_constraint.is_true())
        {
          inner_solver.new_context();
          inner_solver << refinement_constraint;
        }

        // solve
        debug() << "inner solve()" << eom;
        solver_calls++;
        if(inner_solver()==decision_proceduret::D_SATISFIABLE &&
           number_inner_iterations<max_inner_iterations)
        {
          debug() << "inner solver: SAT" << eom;

          std::vector<exprt> c=symb_values.c;

          // new_row_values will contain the new values for c
          linrank_domaint::row_valuet new_row_values;

          // get the model for all c
          for(const auto &e : c)
          {
            exprt v=inner_solver.solver->get(e);
            new_row_values.c.push_back(v);
            debug() << "Inner Solver: " << row << " c value for "
                    << from_expr(ns, "", e) << ": "
                    << from_expr(ns, "", v)  << eom;
          }
          exprt rmv=inner_solver.solver->get(rounding_mode);
          debug() << "Rounding mode: " << from_expr(ns, "", rmv) << eom;

          // update the current template
          domain.set_row_value(row, new_row_values, rank);

          improved=true;
        }
        else
        {
          debug() << "inner solver: UNSAT" << eom;

          if(domain.refine())
          {
            debug() << "refining..." << eom;
            improved=true; // refinement possible
          }
          else
          {
            // no ranking function for the current template
            domain.set_row_value_to_true(row, rank);
            domain.reset_refinements();
          }
        }

        if(!refinement_constraint.is_true())
          inner_solver.pop_context();
      }
    }
  }
  else
  {
    debug() << "UNSAT" << eom;
    domain.reset_refinements();
  }

  solver.pop_context();

  return improved;
}
