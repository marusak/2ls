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
    //strategy_cond_exprs[i]=literal_exprt(domain.strategy_cond_literals[i]);
  }

  exprt cond=disjunction(strategy_cond_exprs);
  adjust_float_expressions(cond, ns);//this is more than needed
  solver << cond;

  if(solver()==decision_proceduret::D_SATISFIABLE)
  {
    for(std::size_t row=0; row<domain.strategy_cond_literals.size(); row++)
    {
      lexlinrank_domaint::pre_post_valuest values;//this is more than needed

      if(solver.solver->l_get(domain.strategy_cond_literals[row]).is_true())
      {
        //get and edit_row from now on
        for(auto &row_expr : domain.strategy_value_exprs[row])
        {
          // model for x_i
          exprt value=solver.solver->get(row_expr.first);
          debug() << "Row " << row << " Value for "
                  << from_expr(ns, "", row_expr.first)
                  << ": " << from_expr(ns, "", value) << eom;
          // model for x'_i
          exprt post_value=solver.solver->get(row_expr.second);
          debug() << "Row " << row << " Value for "
                  << from_expr(ns, "", row_expr.second)
                  << ": " << from_expr(ns, "", post_value) << eom;
          // record all the values
          values.push_back(std::make_pair(value, post_value));
        }

        lexlinrank_domaint::row_valuet symb_values;
        symb_values.resize(rank[row].size());

        // debug() << "elements: " << rank[row].size() << eom;

        exprt constraint;
        exprt refinement_constraint;

        // generate the new constraint
        constraint=domain.get_row_symb_constraint(
          symb_values,
          row, values,
          refinement_constraint);

        simplify_expr(constraint, ns);
        debug() << "Constraint sent to the inner solver: " << row
                << " constraint ";
        pretty_print_termination_argument(debug(), ns, constraint);
        debug() << eom;

        *inner_solver << constraint;

        exprt rounding_mode=symbol_exprt(CPROVER_PREFIX "rounding_mode", signedbv_typet(32));
        // set rounding mode
        *inner_solver << equal_exprt(
          rounding_mode, from_integer(mp_integer(0), signedbv_typet(32)));

        // refinement
        if(!refinement_constraint.is_true())
        {
          inner_solver->new_context();
          *inner_solver << refinement_constraint;
        }

        debug() << "Inner solve()" << eom;
        // solve
        solver_calls++;
        bool inner_solver_result=(*inner_solver)();
        if(inner_solver_result==decision_proceduret::D_SATISFIABLE &&
           number_inner_iterations<max_inner_iterations)
        {
          number_inner_iterations++;

          debug() << "Inner solver: "
                  << "SAT and the max number of iterations was not reached "
                  << eom;
          debug() << "Inner solver: "
                  << "Current number of iterations="
                  << number_inner_iterations << eom;
          debug() << "Inner solver: "
                  << "Current number of components for row "
                  << row << " is " << domain.number_elements_per_row[row]+1 << eom;

          // new_row_values will contain the new values for c and d
          lexlinrank_domaint::row_valuet new_row_values;
          new_row_values.resize(rank[row].size());

          for(std::size_t constraint_no=0;
              constraint_no<symb_values.size(); ++constraint_no)
          {
            std::vector<exprt> c=symb_values[constraint_no].c;

            // get the model for all c
            for(auto &e : c)
            {
              exprt v=inner_solver->solver->get(e);
              new_row_values[constraint_no].c.push_back(v);
              debug() << "Inner Solver: row " << row
                      << "==> c value for ";
              pretty_print_termination_argument(debug(), ns, e);
              debug() << ": ";
              pretty_print_termination_argument(debug(), ns, v);
              debug() << eom;
            }
          }

          improved=true;

          // update the current template
          domain.set_row_value(row, new_row_values, rank);

          if(!refinement_constraint.is_true())
            inner_solver->pop_context();
        }
        else
        {
          if(inner_solver_result==decision_proceduret::D_UNSATISFIABLE)
            debug() << "Inner solver: UNSAT" << eom;
          else
            debug() << "Inner solver: reached max number of iterations" << eom;

          debug() << "Inner solver: number of iterations="
                  << number_inner_iterations << eom;

#ifdef DEBUG_INNER_FORMULA
          for(std::size_t i=0; i<inner_solver->formula.size(); i++)
          {
            if(inner_solver->solver->is_in_conflict(inner_solver->formula[i]))
              debug() << "is_in_conflict: " << inner_solver->formula[i] << eom;
            else
              debug() << "not_in_conflict: " << inner_solver->formula[i] << eom;
          }
#endif

          if(domain.refine())
          {
            debug() << "refining..." << eom;
            improved=true; // refinement possible

            if(!refinement_constraint.is_true())
              inner_solver->pop_context();
          }
          else
          {
            if(domain.number_elements_per_row[row]==max_elements-1)
            {
              debug() << "Reached the max no of lexicographic components "
                      << "and no ranking function was found" << eom;
              // no ranking function for the current template
              domain.set_row_value_to_true(row, rank);
              domain.reset_refinements();
            }
            else
            {
              domain.number_elements_per_row[row]++;
              debug() << "Inner solver: increasing the number "
                      << "of lexicographic components for row " << row
                      << " to " << domain.number_elements_per_row[row]+1 << eom;
              // reset the inner solver
              debug() << "Reset the inner solver " << eom;
              delete inner_solver;
              inner_solver=incremental_solvert::allocate(ns);
              solver_instances++;
              domain.reset_refinements();

              domain.add_element(row, rank);
              number_inner_iterations=0;
              debug() << "Inner solver: "
                      << "the number of inner iterations for row " << row
                      << " was reset to " << number_inner_iterations << eom;
              improved=true;
            }
          }
        }
      }
    }
  }
  else
  {
    debug() << "Outer solver: UNSAT!!" << eom;
    domain.reset_refinements(); // fucntion for not_satisfied...
  }
  solver.pop_context();
  return improved;
}
