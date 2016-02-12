/*******************************************************************\

Module: Implication graph implementation

Author: Rajdeep Mukherjee, Peter Schrammel

\*******************************************************************/

#include "acdl_implication_graph.h"

/*******************************************************************\

Function: acdl_implication_grapht::add_deductions

  Inputs:

 Outputs:

 Purpose:

 \*******************************************************************/
void acdl_implication_grapht::add_deductions
  (const acdl_domaint::deductionst &m_ir)
{
  current_level = 0;
}

/*******************************************************************\

Function: acdl_implication_grapht::add_decision

  Inputs:

 Outputs:

 Purpose:

 \*******************************************************************/
void acdl_implication_grapht::add_decision
  (const acdl_domaint::meet_irreduciblet & m_ir)
{
  current_level = 1;
}
 

/*******************************************************************\

Function: acdl_implication_grapht::first_uip

  Inputs:

 Outputs:

 Purpose: 

 \*******************************************************************/
void acdl_implication_grapht::first_uip(nodest &cut)
{
  assert(false);
}

/*******************************************************************\

Function: acdl_implication_grapht::to_value

  Inputs:

 Outputs:

 Purpose: flatten all node expressions into a vector

 \*******************************************************************/
void acdl_implication_grapht::to_value
  (acdl_domaint::valuet &value) const
{
}
