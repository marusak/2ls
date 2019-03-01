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
  combination_valuet &v=static_cast<combination_valuet &>(value);
  for(unsigned  i=0; i<domains.size(); i++)
    domains[i]->initialize(*(v.domain_values[i]));
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
  const combination_valuet &v=static_cast<const combination_valuet &>(value);
  for(unsigned  i=0; i<domains.size(); i++)
    domains[i]->output_value(out, *(v.domain_values[i]), ns);
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
  for(unsigned  i=0; i<domains.size(); i++)
    domains[i]->output_domain(out, ns);
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
  combination_valuet &v=static_cast<combination_valuet &>(value);
  domains[0]->project_on_vars(*(v.domain_values[0]), vars, result);

  exprt tmp_result;
  for(unsigned  i=1; i<domains.size(); i++)
  {
    domains[i]->project_on_vars(*(v.domain_values[i]), vars, tmp_result);
    if(tmp_result!=true_exprt())
      result=and_exprt(result, tmp_result);
  }
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
  for(unsigned  i=0; i<domains.size(); i++)
    domains[i]->restrict_to_sympath(sympath);
}

/*******************************************************************\

Function: combination_domaint::clear_aux_symbols

  Inputs:

 Outputs:

 Purpose: Reset aux symbols to true (remove all restricitions).

\*******************************************************************/
void combination_domaint::clear_aux_symbols()
{
  for(unsigned  i=0; i<domains.size(); i++)
    domains[i]->clear_aux_symbols();
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
  for(unsigned  i=0; i<domains.size(); i++)
    domains[i]->eliminate_sympaths(sympaths);
}

/*******************************************************************\

Function: combination_domaint::undo_restriction

  Inputs:

 Outputs:

 Purpose: Undo last restriction.

\*******************************************************************/
void combination_domaint::undo_restriction()
{
  for(unsigned  i=0; i<domains.size(); i++)
    domains[i]->undo_restriction();
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
