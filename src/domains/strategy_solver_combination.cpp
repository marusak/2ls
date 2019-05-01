/*******************************************************************\

Module: Strategy solver for combination of domains.

Author: Matej Marusak

\*******************************************************************/

#include "strategy_solver_combination.h"

/*******************************************************************\

Function: strategy_solver_combinationt::iterate

  Inputs:

 Outputs:

 Purpose: Run iteration of each solver separately, but every time
          in the context of the current invariant found by the other
          solver. The template polyhedra solving is also restricted to
          a symbolic path found by the heap solver.

\*******************************************************************/

bool strategy_solver_combinationt::iterate(
  strategy_solver_baset::invariantt &_inv)
{
  bool improved=false;
  bool last_improved=false;
  int set_symbolic_path=-1;
  combination_domaint::combination_valuet &inv=
    static_cast<combination_domaint::combination_valuet &>(_inv);

  for(unsigned i=0; i<domain.domains.size(); i++)
  {
    solver.new_context();
    // From all other domains get pre-constraints
    for(unsigned j=0; j<domain.domains.size(); j++)
    {
      if(i==j)
        continue;
      solver<<domain.domains[j]->to_pre_constraints(*(inv.domain_values[j]));
    }
    // Run iteration in the context of invariant from other solvers' templates
    last_improved=solvers[i]->iterate(*(inv.domain_values[i]));
    solver.pop_context();

    if(last_improved && set_symbolic_path==-1 && i < domain.domains.size() - 1)
    {
      set_symbolic_path=i;
      symbolic_path=solvers[i]->symbolic_path;
      for(unsigned j=i+1; j<domain.domains.size(); j++)
      {
        domain.domains[j]->restrict_to_sympath(symbolic_path);
      }
    }
    improved=improved || last_improved;
  }

  if(improved)
  {
    for(unsigned j=set_symbolic_path+1; j<domain.domains.size(); j++)
    {
      domain.domains[j]->undo_restriction();
    }
  }
  return improved;
}

/*******************************************************************\

Function: strategy_solver_combinationt::set_message_handler

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

void strategy_solver_combinationt::set_message_handler(
  message_handlert &_message_handler)
{
  for(unsigned  i=0; i<solvers.size(); i++)
    solvers[i]->set_message_handler(_message_handler);
}

void strategy_solver_combinationt::clear_symbolic_path()
{
  for(unsigned  i=0; i<solvers.size(); i++)
  {
    solvers[i]->symbolic_path.clear();
  }
}