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
  heap_tpolyhedra_domaint::heap_tpolyhedra_valuet &inv=
    static_cast<heap_tpolyhedra_domaint::heap_tpolyhedra_valuet &>(_inv);


  // Prepare this in domain
  std::vector<domaint*> domains = {&heap_tpolyhedra_domain.heap_domain,
                                  &heap_tpolyhedra_domain.polyhedra_domain};

  std::vector<strategy_solver_baset*> solvers = {&heap_solver,
                                                 &tpolyhedra_solver};

  std::vector<invariantt*> domain_values = {&inv.heap_value,
                                            &inv.tpolyhedra_value};

  bool improved = false;
  // Run one iteration of heap solver in the context of invariant from
  // the template polyhedra solver
  solver.new_context();
  solver << domains[1]->to_pre_constraints(*domain_values[1]);
  improved = improved || solvers[0]->iterate(*domain_values[0]);
  solver.pop_context();

  if(improved)
  {
    // If heap part was improved, restrict template polyhedra part to the found
    // symbolic path
    // domian[1].to_domin(domins[0].to_other_domain())
    symbolic_path=domains[0]->get_symbolic_path();
    static_cast<tpolyhedra_domaint &>(*domains[1]).restrict_to_sympath(symbolic_path);
  }

  // Run one interation of the template polyhedra solver in the context of
  // invariant from the heap solver
  solver.new_context();
  solver << domains[0]->to_pre_constraints(*domain_values[0]);
  improved = improved || solvers[1]->iterate(*domain_values[1]);
  solver.pop_context();

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
