/*******************************************************************\

Module: Combination of heap and template polyhedra abstract domains

Author: Viktor Malik

\*******************************************************************/
#ifndef CPROVER_2LS_DOMAINS_HEAP_TPOLYHEDRA_DOMAIN_H
#define CPROVER_2LS_DOMAINS_HEAP_TPOLYHEDRA_DOMAIN_H


#include "domain.h"
#include "tpolyhedra_domain.h"
#include "heap_domain.h"

class heap_tpolyhedra_domaint:public domaint
{
public:
  std::vector<valuet*> domain_values;
  std::vector<domaint*> domains;

  heap_tpolyhedra_domaint(
    unsigned int _domain_number,
    replace_mapt &_renaming_map,
    const var_specst &var_specs,
    const local_SSAt &SSA,
      std::vector<domaint*> _domains,
      std::vector<domaint::valuet*> _domain_values):
    domaint(_domain_number, _renaming_map, SSA.ns)
  {
      domain_values=_domain_values;
      domains=_domains;
  }

  class heap_tpolyhedra_valuet:public valuet
  {
  public:
      std::vector<valuet*> domain_values;
      heap_tpolyhedra_valuet(std::vector<domaint::valuet*> _domain_values)
    {
        domain_values=_domain_values;
    }
  };

  virtual void initialize(valuet &value) override;

  virtual void output_value(
    std::ostream &out,
    const valuet &value,
    const namespacet &ns) const override;

  virtual void output_domain(
    std::ostream &out,
    const namespacet &ns) const override;

  virtual void project_on_vars(
    valuet &value,
    const var_sett &vars,
    exprt &result) override;

  // Restriction to symbolic paths
  void restrict_to_sympath(const symbolic_patht &sympath);
  void undo_restriction();
  void eliminate_sympaths(const std::vector<symbolic_patht> &sympaths);
  void clear_aux_symbols();

  std::vector<exprt> get_required_smt_values(size_t row);
  void set_smt_values(std::vector<exprt> got_values);

  // Value -> constraints
  exprt to_pre_constraints(valuet &_value);

  void make_not_post_constraints(
    valuet &_value,
    exprt::operandst &cond_exprs);

  bool edit_row(const rowt &row, valuet &inv, bool improved);
};

#endif // CPROVER_2LS_DOMAINS_HEAP_TPOLYHEDRA_DOMAIN_H
