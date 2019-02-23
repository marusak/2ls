/*******************************************************************\

Module: Combination of heap and template polyhedra abstract domains

Author: Viktor Malik

\*******************************************************************/

#include "heap_tpolyhedra_domain.h"

/*******************************************************************\

Function: heap_tpolyhedra_domaint::initialize

  Inputs:

 Outputs:

 Purpose: Initialize abstract value.

\*******************************************************************/

void heap_tpolyhedra_domaint::initialize(domaint::valuet &value)
{
  heap_tpolyhedra_valuet &v=static_cast<heap_tpolyhedra_valuet &>(value);

  domains[0]->initialize(*(v.domain_values[0]));
  domains[1]->initialize(*(v.domain_values[1]));
}

/*******************************************************************\

Function: heap_tpolyhedra_domaint::output_value

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

void heap_tpolyhedra_domaint::output_value(
  std::ostream &out,
  const domaint::valuet &value,
  const namespacet &ns) const
{
  const heap_tpolyhedra_valuet &v=
    static_cast<const heap_tpolyhedra_valuet &>(value);

  domains[0]->output_value(out, *(v.domain_values[0]), ns);
  domains[1]->output_value(out, *(v.domain_values[1]), ns);
}

/*******************************************************************\

Function: heap_tpolyhedra_domaint::output_domain

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

void heap_tpolyhedra_domaint::output_domain(
  std::ostream &out,
  const namespacet &ns) const
{
  domains[0]->output_domain(out, ns);
  domains[1]->output_domain(out, ns);
}

/*******************************************************************\

Function: heap_tpolyhedra_domaint::project_on_vars

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

void heap_tpolyhedra_domaint::project_on_vars(
  domaint::valuet &value,
  const domaint::var_sett &vars,
  exprt &result)
{
  heap_tpolyhedra_valuet &v=static_cast<heap_tpolyhedra_valuet &>(value);

  exprt heap_result;
  domains[0]->project_on_vars(*(v.domain_values[0]), vars, heap_result);
  exprt tpolyhedra_result;
  domains[1]->project_on_vars(*(v.domain_values[1]), vars, tpolyhedra_result);

  result=heap_result;
  if(tpolyhedra_result!=true_exprt())
    result=and_exprt(result, tpolyhedra_result);
}

/*******************************************************************\

Function: heap_tpolyhedra_domaint::restrict_to_sympath

  Inputs: Symbolic path

 Outputs:

 Purpose: Restrict template to a given symbolic path.

\*******************************************************************/
void heap_tpolyhedra_domaint::restrict_to_sympath(
  const symbolic_patht &sympath)
{
  domains[0]->restrict_to_sympath(sympath);
  domains[1]->restrict_to_sympath(sympath);
}

/*******************************************************************\

Function: heap_tpolyhedra_domaint::clear_aux_symbols

  Inputs:

 Outputs:

 Purpose: Reset aux symbols to true (remove all restricitions).

\*******************************************************************/
void heap_tpolyhedra_domaint::clear_aux_symbols()
{
  domains[0]->clear_aux_symbols();
  domains[1]->clear_aux_symbols();
}

/*******************************************************************\

Function: heap_tpolyhedra_domaint::eliminate_sympaths

  Inputs: Vector of symbolic paths

 Outputs:

 Purpose: Restrict template to other paths than those specified.

\*******************************************************************/
void heap_tpolyhedra_domaint::eliminate_sympaths(
  const std::vector<symbolic_patht> &sympaths)
{
  domains[0]->eliminate_sympaths(sympaths);
  domains[1]->eliminate_sympaths(sympaths);
}

/*******************************************************************\

Function: heap_tpolyhedra_domaint::undo_restriction

  Inputs:

 Outputs:

 Purpose: Undo last restriction.

\*******************************************************************/
void heap_tpolyhedra_domaint::undo_restriction()
{
  domains[0]->undo_restriction();
  domains[1]->undo_restriction();
}

bool heap_tpolyhedra_domaint::edit_row(
  const rowt &row,
  valuet &inv,
  bool improved)
{
    return improved;
}

exprt heap_tpolyhedra_domaint::to_pre_constraints(valuet &value)
{
    return true_exprt();
}

void heap_tpolyhedra_domaint::make_not_post_constraints(
  valuet &value,
  exprt::operandst &cond_exprs)
{
}

std::vector<exprt> heap_tpolyhedra_domaint::get_required_smt_values(size_t row)
{
    std::vector<exprt> r;
    return r;
}

void heap_tpolyhedra_domaint::set_smt_values(std::vector<exprt> got_values)
{
}
