/*******************************************************************\

Module: Strategy solver for heap-tpolyhedra domain using symbolic paths

Author: Viktor Malik

\*******************************************************************/

#ifndef CPROVER_2LS_DOMAINS_STRATEGY_SOLVER_HEAP_TPOLYHEDRA_SYMPATH_H
#define CPROVER_2LS_DOMAINS_STRATEGY_SOLVER_HEAP_TPOLYHEDRA_SYMPATH_H


#include "strategy_solver_base.h"
#include "heap_tpolyhedra_sympath_domain.h"
#include "strategy_solver_combination.h"
#include "strategy_solver_binsearch.h"

class strategy_solver_heap_tpolyhedra_sympatht:public strategy_solver_baset
{
public:
  strategy_solver_heap_tpolyhedra_sympatht(
    heap_tpolyhedra_sympath_domaint &_domain,
    incremental_solvert &_solver,
    const local_SSAt &SSA,
    const exprt &precondition,
    message_handlert &message_handler,
    template_generator_baset &template_generator):
    strategy_solver_baset(_solver, SSA.ns),
    domain(_domain),
    combination_solver(
      domain.combination_domain, _solver, SSA, precondition, message_handler,
      template_generator)
  {
    build_loop_conds_map(SSA);

    combination_solver.solvers.push_back(
      new strategy_solvert(
        *static_cast<heap_domaint *>(domain.combination_domain.domains[0]),
        solver,
        SSA,
        precondition,
        get_message_handler(),
        template_generator));
    combination_solver.solvers.push_back(
      new strategy_solver_binsearcht(
        *static_cast<tpolyhedra_domaint *>(
          domain.combination_domain.domains[1]),
        solver,
        SSA.ns));
  }

  virtual bool iterate(invariantt &inv) override;

  virtual void set_message_handler(message_handlert &_message_handler) override;

  void clear_symbolic_path();

protected:
  heap_tpolyhedra_sympath_domaint &domain;
  strategy_solver_combinationt combination_solver;

  std::vector<symbolic_patht> visited_paths;
  bool new_path=true;

  // Mapping for each loop:
  // g#ls    ->    (g#lh   &&   g#le)
  // ^ loop select  ^ loop head ^ loop end
  // This is used to check feasibility of symbolic paths
  std::map<exprt, const exprt> loop_conds_map;
  void build_loop_conds_map(const local_SSAt &SSA);

  bool is_current_path_feasible(
    heap_tpolyhedra_sympath_domaint::heap_tpolyhedra_sympath_valuet &value);
};


#endif // CPROVER_2LS_DOMAINS_STRATEGY_SOLVER_HEAP_TPOLYHEDRA_SYMPATH_H
