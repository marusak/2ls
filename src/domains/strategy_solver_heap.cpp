#include <ssa/ssa_inliner.h>
#include <algorithm>
#include "strategy_solver_heap.h"
#include <goto-symex/adjust_float_expressions.h>

bool strategy_solver_heapt::iterate(invariantt &_inv)
{
  heap_domaint::heap_valuet &inv=static_cast<heap_domaint::heap_valuet &>(_inv);

  bool improved=false;
  heap_domain.pre_iterate_init(_inv);
  if (heap_domain.something_to_solve()) {

      solver.new_context();

      // Entry value constraints
      exprt pre_expr=heap_domain.to_pre_constraints(_inv);
      solver << pre_expr;

      // Exit value constraints
      exprt::operandst strategy_cond_exprs;
      heap_domain.make_not_post_constraints(_inv, strategy_cond_exprs);

      heap_domain.strategy_cond_literals.resize(strategy_cond_exprs.size());

      for(unsigned i=0; i<strategy_cond_exprs.size(); ++i)
      {
        heap_domain.strategy_cond_literals[i]=solver.convert(strategy_cond_exprs[i]);
      }
      exprt cond=disjunction(strategy_cond_exprs);
      adjust_float_expressions(cond, ns);
      solver << cond;

      if(solver()==decision_proceduret::D_SATISFIABLE)  // improvement check
      {
        for(unsigned row=0; row<strategy_cond_literals.size(); ++row)
        {
          if(solver.l_get(strategy_cond_literals[row]).is_true())
          {

            //Find what values from solver are needed
            std::vector<exprt> required_values = heap_domain.get_required_values(row);
            std::vector<exprt> got_values;
            for(auto &c_exprt : required_values) {
                got_values.push_back(solver.solver->get(c_exprt));
            }
            heap_domain.set_values(got_values);

            //improved = heap_domain.edit_row(row, _inv, improved);

            ////////////////////////////////////////////

            const heap_domaint::template_rowt &templ_row=heap_domain.templ[row];

            const exprt loop_guard=to_and_expr(
              heap_domain.templ[row].pre_guard).op1();
            find_symbolic_path(loop_guards, loop_guard);

            if(templ_row.expr.id()==ID_and)
            {
              // Handle template row with a pair of variables in the expression
              exprt points_to1=heap_domain.get_points_to_dest(
                strategy_value_exprs[row].op0(), templ_row.expr.op0(), solver);
              exprt points_to2=heap_domain.get_points_to_dest(
                strategy_value_exprs[row].op1(), templ_row.expr.op1(), solver);

              if(points_to1.is_nil() || points_to2.is_nil())
              {
                if(heap_domain.set_nondet(row, inv))
                {
                  improved=true;
                }
              }
              else
              {
                if(heap_domain.add_points_to(
                  row, inv, and_exprt(points_to1, points_to2)))
                {
                  improved=true;
                  const std::string info=
                    templ_row.mem_kind==heap_domaint::STACK ? "points to "
                                                            : "path to ";
                }
              }
              continue;
            }

            int actual_loc=heap_domain.get_symbol_loc(templ_row.expr);

            exprt points_to=heap_domain.get_points_to_dest(
              strategy_value_exprs[row], templ_row.expr, solver);

            if(points_to.is_nil())
            {
              if(heap_domain.set_nondet(row, inv))
              {
                improved=true;
              }
              continue;
            }
            else
            {
              if(heap_domain.add_points_to(row, inv, points_to))
              {
                improved=true;
                const std::string info=
                  templ_row.mem_kind==heap_domaint::STACK ? "points to "
                                                          : "path to ";
              }
            }

            // If the template row is of heap kind, we need to ensure the
            // transitive closure over the set of all paths
            if(templ_row.mem_kind==heap_domaint::HEAP &&
               points_to.type().get_bool("#dynamic") &&
               points_to.id()==ID_symbol &&
               id2string(to_symbol_expr(points_to).get_identifier()).find(
                 "$unknown")==
               std::string::npos)
            {
              // Find row with corresponding member field of the pointed object
              // (obj.member)
              int member_val_index;
              member_val_index=
                heap_domain.find_member_row(
                  points_to,
                  templ_row.member,
                  actual_loc,
                  templ_row.kind);
              if(member_val_index>=0 && !inv[member_val_index].nondet)
              {
                // Add all paths from obj.next to p
                if(heap_domain.add_transitivity(
                  row,
                  static_cast<unsigned>(member_val_index),
                  inv))
                {
                  improved=true;
                  const std::string expr_str=
                    from_expr(ns, "", heap_domain.templ[member_val_index].expr);
                }
              }
            }

            // Recursively update all rows that are dependent on this row
            if(templ_row.mem_kind==heap_domaint::HEAP)
            {
              updated_rows.clear();
              if(!inv[row].nondet)
                heap_domain.update_rows_rec(row, inv);
              else
                heap_domain.clear_pointing_rows(row, inv);
            }
          }
        }
      }

      else
      {
        debug() << "UNSAT" << eom;

      }
      solver.pop_context();
  }

  return improved;
}
