/*******************************************************************\

Module: Combination of heap and interval abstract domains

Author: Viktor Malik

\*******************************************************************/

#include "heap_interval_domain.h"

/*******************************************************************\

Function: heap_interval_domaint::initialize

  Inputs:

 Outputs:

 Purpose: Initialize abstract value.

\*******************************************************************/

void heap_interval_domaint::initialize(domaint::valuet &value)
{
  heap_interval_valuet &v=static_cast<heap_interval_valuet &>(value);

  heap_domain.initialize(v.heap_value);
  interval_domain.initialize(v.interval_value);
}

/*******************************************************************\

Function: heap_interval_domaint::pre_iterate_init

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

void heap_interval_domaint::pre_iterate_init(valuet &value){
}

bool heap_interval_domaint::something_to_solve(){
    return true;
}

const exprt heap_interval_domaint::initialize_solver(const local_SSAt &SSA,
                                                     const exprt &precondition,
                                                     template_generator_baset &template_generator)
{
    return true_exprt();
}

std::vector<exprt> heap_interval_domaint::get_required_values(size_t row){
    std::vector<exprt> r;
    r.push_back(strategy_value_exprs[row]);
    return r;
}

void heap_interval_domaint::set_values(std::vector<exprt> got_values){
    value = got_values[0];
}

bool heap_interval_domaint::edit_row(const rowt &row, valuet &inv, bool improved)
{
    return false;
}

void heap_interval_domaint::post_edit()
{
}

exprt heap_interval_domaint::to_pre_constraints(valuet &_value)
{
    return true_exprt();
}

void heap_interval_domaint::make_not_post_constraints(valuet &_value,
                                                      exprt::operandst &cond_exprs)
{
}

/*******************************************************************\

Function: heap_interval_domaint::not_satisfiable

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

exprt heap_interval_domaint::not_satisfiable(valuet &value)
{
    return true_exprt();
}

/*******************************************************************\

Function: heap_interval_domaint::output_value

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

void heap_interval_domaint::output_value(
  std::ostream &out,
  const domaint::valuet &value,
  const namespacet &ns) const
{
  const heap_interval_valuet &v=
    static_cast<const heap_interval_valuet &>(value);

  heap_domain.output_value(out, v.heap_value, ns);
  interval_domain.output_value(out, v.interval_value, ns);
}

/*******************************************************************\

Function: heap_interval_domaint::output_domain

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

void heap_interval_domaint::output_domain(
  std::ostream &out,
  const namespacet &ns) const
{
  heap_domain.output_domain(out, ns);
  interval_domain.output_domain(out, ns);
}

/*******************************************************************\

Function: heap_interval_domaint::project_on_vars

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

void heap_interval_domaint::project_on_vars(
  domaint::valuet &value,
  const domaint::var_sett &vars,
  exprt &result)
{
  heap_interval_valuet &v=static_cast<heap_interval_valuet &>(value);

  exprt heap_result;
  heap_domain.project_on_vars(v.heap_value, vars, heap_result);
  exprt interval_result;
  interval_domain.project_on_vars(v.interval_value, vars, interval_result);

  result=heap_result;
  if(interval_result!=true_exprt())
    result=and_exprt(result, interval_result);
}
