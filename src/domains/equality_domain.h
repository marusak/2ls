/*******************************************************************\

Module: Equalities/Disequalities domain

Author: Peter Schrammel

\*******************************************************************/

#ifndef CPROVER_2LS_DOMAINS_EQUALITY_DOMAIN_H
#define CPROVER_2LS_DOMAINS_EQUALITY_DOMAIN_H

#include <set>

#include <util/std_expr.h>
#include <util/union_find.h>

#include "domain.h"

class equality_domaint:public domaint
{
public:
  typedef std::pair<vart, vart> var_pairt;
  typedef std::set<var_pairt> var_pairst;
  typedef std::set<unsigned> index_sett;

equality_domaint(
  unsigned _domain_number,
  replace_mapt &_renaming_map,
  const var_specst &var_specs,
  const namespacet &ns):
  domaint(_domain_number, _renaming_map, ns)
  {
    make_template(var_specs, ns);
  }

  class equ_valuet:public valuet
  {
  public:
    union_find<vart> equs;
    index_sett disequs;
  };

  typedef struct
  {
    guardt pre_guard;
    guardt post_guard;
    equality_domaint::var_pairt var_pair;
    exprt aux_expr;
    kindt kind;
  } template_rowt;

  typedef std::vector<template_rowt> templatet;

  virtual void initialize(valuet &value);

  const exprt initialize_solver(
    const local_SSAt &SSA,
    const exprt &precondition,
    template_generator_baset &template_generator);

  virtual void pre_iterate_init(valuet &value);

  bool edit_row(const rowt &row, exprt &value, valuet &inv, bool improved);

  exprt to_pre_constraints(valuet &_value);

  void make_not_post_constraints(
    valuet &_value,
    exprt::operandst &cond_exprs);

  virtual void not_satisfiable();

  exprt get_pre_equ_constraint(unsigned index);
  exprt get_post_not_equ_constraint(unsigned index);
  exprt get_pre_disequ_constraint(unsigned index);
  exprt get_post_not_disequ_constraint(unsigned index);

  void set_equal(unsigned index, equ_valuet &value);
  void set_disequal(unsigned index, equ_valuet &value);

  virtual void output_value(
    std::ostream &out,
    const valuet &value,
    const namespacet &ns) const;

  virtual void output_domain(std::ostream &out, const namespacet &ns) const;

  virtual void project_on_vars(
    valuet &value,
    const var_sett &vars,
    exprt &result);

  void get_index_set(index_sett &indices);
  const var_pairt &get_var_pair(unsigned index);

protected:
  templatet templ;

  void make_template(
    const var_specst &var_specs,
    const namespacet &ns);

  bool adapt_types(exprt &v1, exprt &v2);
};

#endif // CPROVER_2LS_DOMAINS_EQUALITY_DOMAIN_H
