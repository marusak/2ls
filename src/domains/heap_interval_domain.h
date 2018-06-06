/*******************************************************************\

Module: Combination of heap and interval abstract domains

Author: Viktor Malik

\*******************************************************************/
#ifndef CPROVER_2LS_DOMAINS_HEAP_INTERVAL_DOMAIN_H
#define CPROVER_2LS_DOMAINS_HEAP_INTERVAL_DOMAIN_H


#include "domain.h"
#include "tpolyhedra_domain.h"
#include "heap_domain.h"

class heap_interval_domaint:public domaint
{
public:
  heap_domaint heap_domain;
  tpolyhedra_domaint interval_domain;

  heap_interval_domaint(
    unsigned int _domain_number,
    replace_mapt &_renaming_map,
    const var_specst &var_specs,
    const namespacet &ns):
    domaint(_domain_number, _renaming_map, ns),
    heap_domain(_domain_number, _renaming_map, var_specs, ns),
    interval_domain(_domain_number, _renaming_map, ns)
  {
    interval_domain.add_interval_template(var_specs, ns);
    iterate_count = 1;
  }

  class heap_interval_valuet:public valuet
  {
  public:
    heap_domaint::heap_valuet heap_value;
    tpolyhedra_domaint::templ_valuet interval_value;
  };

  virtual void initialize(valuet &value) override;

  const exprt initialize_solver(
    const local_SSAt &SSA,
    const exprt &precondition,
    template_generator_baset &template_generator);

  virtual void pre_iterate_init(valuet &value);
  virtual bool something_to_solve();

  std::vector<exprt> get_required_values(size_t row);
  void set_values(std::vector<exprt> got_values);

  bool edit_row(const rowt &row, valuet &inv, bool improved);

  exprt to_pre_constraints(valuet &_value);

  void make_not_post_constraints(
    valuet &_value,
    exprt::operandst &cond_exprs);

  virtual exprt not_satisfiable(valuet &value);

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
protected:
  exprt value;
  int iterate_count;
};

#endif // CPROVER_2LS_DOMAINS_HEAP_INTERVAL_DOMAIN_H
