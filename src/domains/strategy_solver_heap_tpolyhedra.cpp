/*******************************************************************\

Module: Strategy solver for combination of shape and template
        polyhedra domains.

Author: Viktor Malik

\*******************************************************************/

#include "strategy_solver_heap_tpolyhedra.h"

/*******************************************************************\

Function: strategy_solver_heap_tpolyhedrat::iterate

  Inputs:

 Outputs:

 Purpose: Run iteration of each solver separately, but every time
          in the context of the current invariant found by the other
          solver. The template polyhedra solving is also restricted to
          a symbolic path found by the heap solver.

\*******************************************************************/

bool strategy_solver_heap_tpolyhedrat::iterate(
  strategy_solver_baset::invariantt &_inv)
{

  // MOVE TO DOMAIN
  heap_tpolyhedra_domaint::heap_tpolyhedra_valuet &inv=
    static_cast<heap_tpolyhedra_domaint::heap_tpolyhedra_valuet &>(_inv);


  std::vector<domaint*> domains = {&heap_tpolyhedra_domain.heap_domain,
                                  &heap_tpolyhedra_domain.polyhedra_domain};

  std::vector<strategy_solver_baset*> solvers = {&heap_solver,
                                                 &tpolyhedra_solver};

  std::vector<invariantt*> domain_values = {&inv.heap_value,
                                            &inv.tpolyhedra_value};
  // UNTIL THIS

  bool improved = false;

  for (unsigned i = 0; i < domains.size(); i++){
      solver.new_context();
      // From all other domains get pre-constraints
      for (unsigned j = 0; j < domains.size(); j++){
          if (i == j)
              continue;
        solver << domains[j]->to_pre_constraints(*domain_values[j]);
      }
      // Run one iteration in the context of invariant from templates of others solvers
      improved = improved || solvers[i]->iterate(*domain_values[i]);
      solver.pop_context();

      // IF FIRST TRUE, ALWAYS TRUE
      if(improved)
      {
        // HOW SHOULD THIS WORK - EVERYONE PROVIDES? EVERYONE WRITES?
        // DOES NOT WORK WHEN i = 1
        symbolic_path=domains[i]->get_symbolic_path();
        for (unsigned j = 0; j < domains.size(); j++){
            if (i == j)
                continue;
            domains[j]->restrict_to_sympath(symbolic_path);
        }
      }
  }

  // WHAT WAS THIS?
  /*
  if(heap_improved)
    heap_tpolyhedra_domain.polyhedra_domain.undo_restriction();
  */

  return improved;
}

/*******************************************************************\

Function: strategy_solver_heap_tpolyhedrat::set_message_handler

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

void strategy_solver_heap_tpolyhedrat::set_message_handler(
  message_handlert &_message_handler)
{
  heap_solver.set_message_handler(_message_handler);
  tpolyhedra_solver.set_message_handler(_message_handler);
}

void strategy_solver_heap_tpolyhedrat::clear_symbolic_path()
{
  heap_tpolyhedra_domain.heap_domain.symbolic_path.clear();
  tpolyhedra_solver.symbolic_path.clear();
}
