/*******************************************************************\

Module: Combination of  abstract domains

Author: Matej Marusak

\*******************************************************************/

#include "combination_domain.h"

/*******************************************************************\

Function: combination_domaint::initialize

  Inputs:

 Outputs:

 Purpose: Initialize abstract value.

\*******************************************************************/

void combination_domaint::initialize(domaint::valuet &value)
{
  //heap_tpolyhedra_valuet &v=static_cast<heap_tpolyhedra_valuet &>(value);
  //TODO
  //heap_domain.initialize(v.heap_value);
}

/*******************************************************************\

Function: combination_domaint::output_value

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

void combination_domaint::output_value(
  std::ostream &out,
  const domaint::valuet &value,
  const namespacet &ns) const
{
  //const heap_tpolyhedra_valuet &v=
  //  static_cast<const heap_tpolyhedra_valuet &>(value);

  //TODO
  //heap_domain.output_value(out, v.heap_value, ns);
}

/*******************************************************************\

Function: combination_domaint::output_domain

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

void combination_domaint::output_domain(
  std::ostream &out,
  const namespacet &ns) const
{
  // TODO
  //  heap_domain.output_domain(out, ns);
}

/*******************************************************************\

Function: combination_domaint::project_on_vars

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

void combination_domaint::project_on_vars(
  domaint::valuet &value,
  const domaint::var_sett &vars,
  exprt &result)
{
  //TODO
  /*
    heap_tpolyhedra_valuet &v=static_cast<heap_tpolyhedra_valuet &>(value);

  exprt heap_result;
  heap_domain.project_on_vars(v.heap_value, vars, heap_result);
  exprt tpolyhedra_result;
  polyhedra_domain.project_on_vars(v.tpolyhedra_value, vars, tpolyhedra_result);

  result=heap_result;
  if(tpolyhedra_result!=true_exprt())
    result=and_exprt(result, tpolyhedra_result);
    */
}

/*******************************************************************\

Function: combination_domaint::restrict_to_sympath

  Inputs: Symbolic path

 Outputs:

 Purpose: Restrict template to a given symbolic path.

\*******************************************************************/
void combination_domaint::restrict_to_sympath(
  const symbolic_patht &sympath)
{
  // TODO for each 
  //heap_domain.restrict_to_sympath(sympath);
}

/*******************************************************************\

Function: combination_domaint::clear_aux_symbols

  Inputs:

 Outputs:

 Purpose: Reset aux symbols to true (remove all restricitions).

\*******************************************************************/
void combination_domaint::clear_aux_symbols()
{
  // TODO FOR each
  //heap_domain.clear_aux_symbols();
}

/*******************************************************************\

Function: combination_domaint::eliminate_sympaths

  Inputs: Vector of symbolic paths

 Outputs:

 Purpose: Restrict template to other paths than those specified.

\*******************************************************************/
void combination_domaint::eliminate_sympaths(
  const std::vector<symbolic_patht> &sympaths)
{
  // TODO for each eliminate
  //heap_domain.eliminate_sympaths(sympaths);
}

/*******************************************************************\

Function: combination_domaint::undo_restriction

  Inputs:

 Outputs:

 Purpose: Undo last restriction.

\*******************************************************************/
void combination_domaint::undo_restriction()
{
  // TODO for each undo_restirciton
}

bool combination_domaint::edit_row(
  const rowt &row,
  valuet &inv,
  bool improved)
{
    return improved;
}

exprt combination_domaint::to_pre_constraints(valuet &value)
{
    return true_exprt();
}

void combination_domaint::make_not_post_constraints(
  valuet &value,
  exprt::operandst &cond_exprs)
{
}

std::vector<exprt> combination_domaint::get_required_smt_values(size_t row)
{
    std::vector<exprt> r;
    return r;
}

void combination_domaint::set_smt_values(std::vector<exprt> got_values)
{
}
