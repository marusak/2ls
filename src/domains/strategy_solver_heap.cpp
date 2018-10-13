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

            //improved = heap_domain.edit_row(row, inv, improved);

            ////////////////////////////////////////////

            const heap_domaint::template_rowt &templ_row=heap_domain.templ[row];

            const exprt loop_guard=to_and_expr(
              heap_domain.templ[row].pre_guard).op1();
            find_symbolic_path(loop_guards, loop_guard);

            if(templ_row.expr.id()==ID_and)
            {
              // Handle template row with a pair of variables in the expression
              exprt points_to1=get_points_to_dest(
                strategy_value_exprs[row].op0(), templ_row.expr.op0());
              exprt points_to2=get_points_to_dest(
                strategy_value_exprs[row].op1(), templ_row.expr.op1());

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

            exprt points_to=get_points_to_dest(
              strategy_value_exprs[row], templ_row.expr);

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
                find_member_row(
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
                update_rows_rec(row, inv);
              else
                clear_pointing_rows(row, inv);
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
      if(id.find(obj_id)!=std::string::npos &&
         id.find_first_of(".")==obj_id.length())
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
  forall_operands(it, expr)print_solver_expr(*it);
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

/*******************************************************************\

Function: strategy_solver_heapt::clear_pointing_rows

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

void strategy_solver_heapt::clear_pointing_rows(
  const heap_domaint::rowt &row,
  heap_domaint::heap_valuet &value)
{
  heap_domaint::heap_row_valuet &row_value=
    static_cast<heap_domaint::heap_row_valuet &>(value[row]);

  std::vector<heap_domaint::rowt> to_remove;
  for(auto &ptr : row_value.pointed_by)
  {
    if(ptr!=row)
    {
      debug() << "Clearing row: " << ptr << eom;
      value[ptr].clear();
      to_remove.push_back(ptr);
    }
  }
  for(auto &r : to_remove)
    row_value.pointed_by.erase(r);
}

/*******************************************************************\

Function: strategy_solver_heapt::get_points_to_dest

  Inputs:

 Outputs:

 Purpose: Get an address where the given pointer points to in the current
          solver iteration. Returns nil_exprt if the value of the pointer
          is nondet.

\*******************************************************************/

const exprt strategy_solver_heapt::get_points_to_dest(
  const exprt &pointer,
  const exprt &templ_row_expr)
{
  exprt value=solver.get(pointer);
  // Value from the solver must be converted into an expression
  exprt ptr_value=heap_domain.value_to_ptr_exprt(value);

  if((ptr_value.id()==ID_constant &&
      to_constant_expr(ptr_value).get_value()==ID_NULL) ||
     ptr_value.id()==ID_symbol)
  {
    // Add equality p == NULL or p == symbol
    return ptr_value;
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
      return nil_exprt();
    }

    symbol_exprt obj=to_symbol_expr(
      to_address_of_expr(ptr_value).object());

    if(obj.type()!=templ_row_expr.type() &&
       ns.follow(templ_row_expr.type().subtype())!=ns.follow(obj.type()))
    {
      if(!is_cprover_symbol(templ_row_expr))
      {
        // If types disagree, it's a nondet (solver assigned random value)
        return nil_exprt();
      }
    }

    // Add equality p == &obj
    return obj;
  }
  else
    return nil_exprt();
}
