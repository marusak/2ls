#include <ssa/ssa_inliner.h>
#include "strategy_solver_heap.h"

bool strategy_solver_heapt::iterate(invariantt &_inv)
{
  heap_domaint::heap_valuet &inv=static_cast<heap_domaint::heap_valuet &>(_inv);

  bool improved=false;

  solver.new_context();

  // Entry value constraints
  exprt pre_expr=heap_domain.to_pre_constraints(inv);
  solver << pre_expr;

  // Exit value constraints
  exprt::operandst strategy_cond_exprs;
  exprt not_post = heap_domain.to_post_not_equ_constraints(inv,
                                                           strategy_cond_exprs,
                                                           strategy_value_exprs,
                                                           solver.convert,
                                                           strategy_cond_literals);

  solver << not_post;

  if(solver()==decision_proceduret::D_SATISFIABLE)  // improvement check
  {
    for(unsigned row=0; row<strategy_cond_literals.size(); ++row)
    {
      if(solver.l_get(strategy_cond_literals[row]).is_true())
      {
        debug() << "updating row: " << row << eom;

        const heap_domaint::template_rowt &templ_row=heap_domain.templ[row];

        int actual_loc=heap_domain.get_symbol_loc(templ_row.expr);

        exprt pointer=strategy_value_exprs[row];
        exprt value=solver.get(pointer);
        // Value from the solver must be converted into an expression
        exprt ptr_value=heap_domain.value_to_ptr_exprt(value);

        if((ptr_value.id()==ID_constant &&
            to_constant_expr(ptr_value).get_value()==ID_NULL) ||
           ptr_value.id()==ID_symbol)
        {
          // Add equality p == NULL or p == symbol
          if(heap_domain.add_points_to(row, inv, ptr_value))
          {
            improved=true;
            const std::string info=
              templ_row.mem_kind==heap_domaint::STACK?"points to ":"path to ";
            debug() << "Add " << info << from_expr(ns, "", ptr_value) << eom;
          }
        }
        else if(ptr_value.id()==ID_address_of)
        {
          // Template row pointer points to the heap (p = &obj)
          debug() << from_expr(ns, "", ptr_value) << eom;
          assert(ptr_value.id()==ID_address_of);
          if(to_address_of_expr(ptr_value).object().id()!=ID_symbol)
          {
            // If solver did not return address of a symbol, it is considered
            // as nondet value.
            if(heap_domain.set_nondet(row, inv))
            {
              improved=true;
              debug() << "Set nondet" << eom;
            }
            continue;
          }

          symbol_exprt obj=to_symbol_expr(
            to_address_of_expr(ptr_value).object());

          if(obj.type()!=templ_row.expr.type() &&
             ns.follow(templ_row.expr.type().subtype())!=ns.follow(obj.type()))
          {
            // If types disagree, it's a nondet (solver assigned random value)
            if(heap_domain.set_nondet(row, inv))
            {
              improved=true;
              debug() << "Set nondet" << eom;
            }
            continue;
          }

          // Add equality p == &obj
          if(heap_domain.add_points_to(row, inv, obj))
          {
            improved=true;
            const std::string info=
              templ_row.mem_kind==heap_domaint::STACK?"points to ":"path to ";
            debug() << "Add " << info << from_expr(ns, "", obj) << eom;
          }

          // If the template row is of heap kind, we need to ensure the
          // transitive closure over the set of all paths
          if(templ_row.mem_kind==heap_domaint::HEAP &&
             obj.type().get_bool("#dynamic") &&
             id2string(obj.get_identifier()).find("$unknown")==
             std::string::npos)
          {
            // Find row with corresponding member field of the pointed object
            // (obj.member)
            int member_val_index;
            member_val_index=
              find_member_row(
                obj,
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
                debug() << "Add all paths: " << expr_str
                        << ", through: " << from_expr(ns, "", obj) << eom;
              }
            }
          }
        }
        else
        {
          if(heap_domain.set_nondet(row, inv))
          {
            improved=true;
            debug() << "Set nondet" << eom;
          }
        }

        // Recursively update all rows that are dependent on this row
        if(templ_row.mem_kind==heap_domaint::HEAP)
        {
          updated_rows.clear();
          if(!inv[row].nondet)
            update_rows_rec(row, inv);
        }
      }
    }
  }

  else
  {
    debug() << "UNSAT" << eom;
  }
  solver.pop_context();

  return improved;
}

/*******************************************************************\

Function: strategy_solver_heapt::find_member_row

  Inputs: object
          field
          actual location

 Outputs: Row number for obj.member#loc with maximal loc less than actual_loc
          -1 if such template row does not exist

 Purpose: Find the template row that contains specified member field
          of a dynamic object at the given location.

\*******************************************************************/

int strategy_solver_heapt::find_member_row(
  const exprt &obj,
  const irep_idt &member,
  int actual_loc,
  const domaint::kindt &kind)
{
  assert(obj.id()==ID_symbol);
  std::string obj_id=id2string(
    ssa_inlinert::get_original_identifier(to_symbol_expr(obj)));

  int result=-1;
  int max_loc=-1;
  for(unsigned i=0; i<heap_domain.templ.size(); ++i)
  {
    heap_domaint::template_rowt &templ_row=heap_domain.templ[i];
    if(templ_row.kind==kind && templ_row.member==member &&
       templ_row.mem_kind==heap_domaint::HEAP)
    {
      std::string id=id2string(to_symbol_expr(templ_row.expr).get_identifier());
      if(id.find(obj_id)!=std::string::npos)
      {
        int loc=heap_domain.get_symbol_loc(templ_row.expr);
        if(loc>max_loc &&
           (kind==domaint::OUT || kind==domaint::OUTHEAP || loc<=actual_loc))
        {
          max_loc=loc;
          result=i;
        }
      }
    }
  }
  return result;
}

/*******************************************************************\

Function: strategy_solver_heapt::update_rows_rec

  Inputs:

 Outputs:

 Purpose: Recursively update rows that point to given row.

\*******************************************************************/

bool strategy_solver_heapt::update_rows_rec(
  const heap_domaint::rowt &row,
  heap_domaint::heap_valuet &value)
{
  heap_domaint::heap_row_valuet &row_value=
    static_cast<heap_domaint::heap_row_valuet &>(value[row]);
  const heap_domaint::template_rowt &templ_row=heap_domain.templ[row];

  updated_rows.insert(row);
  bool result=false;
  for(const heap_domaint::rowt &ptr : row_value.pointed_by)
  {
    if(heap_domain.templ[ptr].mem_kind==heap_domaint::HEAP &&
       heap_domain.templ[ptr].member==templ_row.member)
    {
      if(heap_domain.add_transitivity(ptr, row, value))
        result=true;

      debug() << "recursively updating row: " << ptr << eom;
      debug() << "add all paths: " << from_expr(ns, "", templ_row.expr)
              << ", through: "
              << from_expr(ns, "", templ_row.dyn_obj) << eom;
      // Recursive update is called for each row only once
      if(updated_rows.find(ptr)==updated_rows.end())
        result=update_rows_rec(ptr, value) || result;
    }
  }
  return result;
}

/*******************************************************************\

Function: strategy_solver_heapt::print_solver_expr

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

void strategy_solver_heapt::print_solver_expr(const exprt &expr)
{
  debug() << from_expr(ns, "", expr) << ": "
          << from_expr(ns, "", solver.get(expr)) << eom;
  forall_operands(it, expr)
    print_solver_expr(*it);
}

/*******************************************************************\

Function: strategy_solver_heapt::initialize

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

void strategy_solver_heapt::initialize(
  const local_SSAt &SSA,
  const exprt &precondition,
  template_generator_baset &template_generator)
{
  heap_domain.initialize_domain(SSA, precondition, template_generator);

  const exprt input_bindings=heap_domain.get_input_bindings();
  if(!input_bindings.is_true())
  {
    solver << input_bindings;
    debug() << "Input bindings:" << eom;
    debug() << from_expr(ns, "", input_bindings) << eom;
  }

  if(!heap_domain.new_heap_row_specs.empty())
  {
    debug() << "New template:" << eom;
    heap_domain.output_domain(debug(), ns);
  }
}
