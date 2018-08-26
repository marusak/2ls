/*******************************************************************\

Module: Abstract domain base class

Author: Peter Schrammel

\*******************************************************************/

#include "domain.h"

/*******************************************************************\

Function: domaint::merge_kinds

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

domaint::kindt domaint::merge_kinds(kindt k1, kindt k2)
{
  return
    (k1==OUT || k2==OUT ?  (k1==LOOP || k2==LOOP ?  OUTL : OUT) :
      (k1==LOOP || k2==LOOP ? LOOP :  IN));
}

/*******************************************************************\

Function: domaint::output_var_specs

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

void domaint::output_var_specs(
  std::ostream &out,
  const var_specst &var_specs,
  const namespacet &ns)
{
  for(const auto &v : var_specs)
  {
    switch(v.kind)
    {
    case LOOP:
      out << "(LOOP) [ " << from_expr(ns, "", v.pre_guard) << " | ";
      out << from_expr(ns, "", v.post_guard) << " ]: ";
      break;
    case IN:
      out << "(IN)   ";
      out << from_expr(ns, "", v.pre_guard) << ": ";
      break;
    case OUT: case OUTL:
      out << "(OUT)  ";
      out << from_expr(ns, "", v.post_guard) << ": ";
      break;
    default: assert(false);
    }
    out << from_expr(ns, "", v.var) << std::endl;
  }
}

const exprt domaint::initialize_solver(
    const local_SSAt &SSA,
    const exprt &precondition,
    template_generator_baset &template_generator)
{
    return true_exprt();
}

void domaint::pre_iterate_init(valuet &value){
}

bool domaint::something_to_solve(){
    return true;
}

bool domaint::not_satisfiable(valuet &value, bool improved)
{
    return improved;
}

void domaint::post_edit()
{
}

exprt domaint::make_permanent(valuet &value)
{
    return true_exprt();
}

/*******************************************************************\

Function: strategy_solver_baset::find_symbolic_path

  Inputs:

 Outputs:

 Purpose: Find symbolic path that is explored by the current solver
          iteration. A path is specified by a conjunction of literals
          containing loop-select guards of all loops in program.

\*******************************************************************/
void domaint::find_symbolic_path(
  std::set<symbol_exprt> &loop_guards,
  const exprt &current_guard)
{
  for(const symbol_exprt &guard : loop_guards)
  {
    if(guard==current_guard)
    {
      symbolic_path[guard]=true;
      continue;
    }
    exprt guard_value=solver.get(guard);
    if(guard_value.is_true())
      symbolic_path[guard]=true;
    else if(guard_value.is_false())
      symbolic_path[guard]=false;
  }
}
