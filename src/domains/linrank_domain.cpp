#include "linrank_domain.h"
#include "template_domain.h"

#include <iostream>

#include <util/find_symbols.h>
#include <util/i2string.h>
#include <util/simplify_expr.h>
#include <langapi/languages.h>

#define SYMB_BOUND_VAR "symb_bound#"

void linrank_domaint::initialize(valuet &value)
{
	templ_valuet &v = static_cast<templ_valuet&>(value);
	v.resize(templ.size());
	for(unsigned row = 0; row<templ.size(); row++)
		v[row] = false_exprt();
}

exprt linrank_domaint::get_not_constraints(const linrank_domaint::templ_valuet &value,
			    exprt::operandst &cond_exprs,
			    linrank_domaint::pre_post_valuest &value_exprs)
{
	cond_exprs.resize(value.size());
	value_exprs.resize(value.size());

	for(unsigned row = 0; row<templ.size(); row++)
	{
		value_exprs[row] = templ[row].expr;
		rename(value_exprs[row]);

		exprt sum_first = value[row].d;
		exprt sum_second = value[row].d;
		for(int i = 0; i < value[row].c.size(); ++i)
		{
			sum_first = plus_exprt(sum_first, mult_exprt(value[row].c[i], templ[row].expr[i].first));
			sum_second = plus_exprt(sum_second, mult_exprt(value[row].c[i], templ[row].expr[i].second));
		}

		exprt bounded = binary_relation_exprt(sum_first, ID_g, constant_exprt(0, value[row].d.type()));
		exprt decreasing = binary_relation_exprt(sum_first, ID_g, sum_second);

		cond_exprs[row] = implies_exprt(and_exprt(templ[row].pre_guard, templ[row].post_guard),
				not_exprt(and_exprt(bounded, decreasing)));
	}

	return conjunction(cond_exprs);
}

exprt linrank_domaint::get_row_symb_contraint(linrank_domaint::row_valuet &symb_values, // contains vars c and d
			       const linrank_domaint::rowt &row,
			       linrank_domaint::pre_post_valuest &values
			       )
{
	symb_values.c.resize(values.size());

	symb_values.d = symbol_exprt(SYMB_BOUND_VAR+"d", values[0].first.type());
	exprt sum_first = symb_values.d;
	exprt sum_second = symb_values.d;
	for(int i = 0; i < symb_values.c.size(); ++i)
	{
		symb_values.c[i] = symbol_exprt(SYMB_BOUND_VAR+i2string(i), values[i].first.type());
		sum_first = plus_exprt(sum_first, mult_exprt(symb_values.c[i], values[i].first));
		sum_second = plus_exprt(sum_second, mult_exprt(symb_values.c[i], values[i].second));
	}

	exprt bounded = binary_relation_exprt(sum_first, ID_g, constant_exprt(0, symb_values.d.type()));
	exprt decreasing = binary_relation_exprt(sum_first, ID_g, sum_second);

	return implies_exprt(and_exprt(templ[row].pre_guard, templ[row].post_guard),
			and_exprt(bounded, decreasing));
}

linrank_domaint::row_valuet linrank_domaint::get_row_value(const rowt &row, const templ_valuet &value)
{
	assert(row<value.size());
	assert(value.size()==templ.size());
	return value[row];
}

void linrank_domaint::set_row_value(const rowt &row, const row_valuet &row_value, templ_valuet &value)
{
	assert(row<value.size());
	assert(value.size()==templ.size());
	value[row] = row_value;
}

void linrank_domaint::output_value(std::ostream &out, const valuet &value,
  const namespacet &ns)
{
  const templ_valuet &v = static_cast<const templ_valuet &>(value);
  for(unsigned row = 0; row<templ.size(); row++)
  {
    const template_rowt &templ_row = templ[row];
    switch(templ_row.kind)
    {
    case LOOP:
      out << "(LOOP) [ " << from_expr(ns,"",templ_row.pre_guard) << " | ";
      out << from_expr(ns,"",templ_row.post_guard) << " ] ===> " << std::endl << "       ";
      break;
//    case IN: out << "(IN)   "; break;
//    case OUT: case OUTL: out << "(OUT)  "; break;
    default: assert(false);
    }
    out << "( " << from_expr(ns,"",templ_row.expr) << " <= " << from_expr(ns,"",v[row]) << " )" << std::endl;
  }
}

void linrank_domaint::output_domain(std::ostream &out, const namespacet &ns)
{
  for(unsigned row = 0; row<templ.size(); row++)
  {
    const template_rowt &templ_row = templ[row];
    switch(templ_row.kind)
    {
    case LOOP:
      out << "(LOOP) [ " << from_expr(ns,"",templ_row.pre_guard) << " | ";
      out << from_expr(ns,"",templ_row.post_guard) << " ] ===> " << std::endl << "      ";
      break;
//    case IN:
//      out << "(IN)   ";
//      out << from_expr(ns,"",templ_row.pre_guard) << " ===> " << std::endl << "      ";
//      break;
//    case OUT: case OUTL:
//      out << "(OUT)  ";
//      out << from_expr(ns,"",templ_row.post_guard) << " ===> " << std::endl << "      ";
//      break;
    default: assert(false);
    }
    out << "( " <<
        from_expr(ns,"",templ_row.expr) << " <= CONST )" << std::endl;
  }
}

void linrank_domaint::project_on_loops(const valuet &value, exprt &result)
{
	const templ_valuet &v = static_cast<const templ_valuet &>(value);
	assert(v.size()==templ.size());
	exprt::operandst c;
	c.reserve(templ.size());
	for(unsigned row = 0; row<templ.size(); row++)
	{
		project_row_on_kind(v,row,LOOP,c);
		assert(templ[row].kind == LOOP);
		c.push_back(binary_relation_exprt(templ[row].expr,ID_le,v[row]));
	}
	result = conjunction(c);
}

void linrank_domaint::project_on_inout(const valuet &value, exprt &result)
{
	result = true_exprt();
}

void linrank_domaint::project_on_vars(const valuet &value, const var_sett &vars, exprt &result)
{
	const templ_valuet &v = static_cast<const templ_valuet &>(value);
	assert(v.size()==templ.size());
	exprt::operandst c;
	for(unsigned row = 0; row<templ.size(); row++)
	{
		const template_rowt &templ_row = templ[row];

		std::set<symbol_exprt> symbols;
		find_symbols(templ_row.expr,symbols);

		bool pure = true;
		for(std::set<symbol_exprt>::iterator it = symbols.begin();
					it != symbols.end(); it++)
		{
			if(vars.find(*it)==vars.end())
			{
				pure = false;
				break;
			}
		}
		if(!pure) continue;

		c.push_back(binary_relation_exprt(templ_row.expr,ID_le,v[row]));
	}
	result = conjunction(c);
}



// /*******************************************************************\

// Function: linrank_domaint::between

//   Inputs:

//  Outputs:

//  Purpose:

// \*******************************************************************/

// linrank_domaint::row_valuet linrank_domaint::between(
//   const row_valuet &lower, const row_valuet &upper)
// {
//   if(lower.type()==upper.type() && 
//      (lower.type().id()==ID_signedbv || lower.type().id()==ID_unsignedbv))
//   {
//     mp_integer vlower, vupper;
//     to_integer(lower, vlower);
//     to_integer(upper, vupper);
//     assert(vupper>=vlower);
//     if(vlower+1==vupper) return from_integer(vlower,lower.type()); //floor
//     return from_integer((vupper+vlower)/2,lower.type());
//   }
//   if(lower.type().id()==ID_floatbv && upper.type().id()==ID_floatbv)
//   {
//     ieee_floatt vlower(to_constant_expr(lower));
//     ieee_floatt vupper(to_constant_expr(upper));
//     if(vlower.get_sign()==vupper.get_sign()) 
//     {
//       mp_integer plower = vlower.pack(); //compute "median" float number
//       mp_integer pupper = vupper.pack();
//       //assert(pupper>=plower);
//       ieee_floatt res;
//       res.unpack((plower+pupper)/2); //...by computing integer mean
//       return res.to_expr();
//     }
//     ieee_floatt res;
//     res.make_zero();
//     return res.to_expr();
//   }
//   assert(false); //types do not match or are not supported
// }

// /*******************************************************************\

// Function: linrank_domaint::leq

//   Inputs:

//  Outputs:

//  Purpose:

// \*******************************************************************/

// bool linrank_domaint::less_than(const row_valuet &v1, const row_valuet &v2)
// {
//   if(v1.type()==v2.type() && 
//      (v1.type().id()==ID_signedbv || v1.type().id()==ID_unsignedbv))
//   {
//     mp_integer vv1, vv2;
//     to_integer(v1, vv1);
//     to_integer(v2, vv2);
//     return vv1<vv2;
//   }
//   if(v1.type().id()==ID_floatbv && v2.type().id()==ID_floatbv)
//   {
//     ieee_floatt vv1(to_constant_expr(v1));
//     ieee_floatt vv2(to_constant_expr(v2));
//     return vv1<vv2;
//   }
//   assert(false); //types do not match or are not supported
// }

// /*******************************************************************\

// Function: linrank_domaint::get_row_pre_constraint

//   Inputs:

//  Outputs:

//  Purpose: pre_guard ==> row_expr <= row_value

// \*******************************************************************/

// exprt linrank_domaint::get_row_constraint(const rowt &row, 
//   const row_valuet &row_value)
// {
//   assert(row<templ.size());
//   kindt k = templ[row].kind;
//   if(k==OUT || k==OUTL) return true_exprt();
//   if(is_row_value_neginf(row_value)) return false_exprt();
//   if(is_row_value_inf(row_value)) return true_exprt();
//   return binary_relation_exprt(templ[row].expr,ID_le,row_value);
// }

// exprt linrank_domaint::get_row_pre_constraint(const rowt &row, 
//   const row_valuet &row_value)
// {
//   assert(row<templ.size());
//   const template_rowt &templ_row = templ[row];
//   kindt k = templ_row.kind;
//   if(k==OUT || k==OUTL) return true_exprt();
//   if(is_row_value_neginf(row_value)) 
//     return implies_exprt(templ_row.pre_guard, false_exprt());
//   if(is_row_value_inf(row_value)) 
//    return implies_exprt(templ_row.pre_guard, true_exprt());
//   return implies_exprt(templ_row.pre_guard, 
//     binary_relation_exprt(templ_row.expr,ID_le,row_value));
// }


// exprt linrank_domaint::get_row_pre_constraint(const rowt &row, 
//   const templ_valuet &value)
// {
//   assert(value.size()==templ.size());
//   return get_row_pre_constraint(row,value[row]);
// }

// /*******************************************************************\

// Function: linrank_domaint::get_row_post_constraint

//   Inputs:

//  Outputs:

//  Purpose: row_expr <= row_value

// \*******************************************************************/

// exprt linrank_domaint::get_row_post_constraint(const rowt &row, 
//   const row_valuet &row_value)
// {
//   assert(row<templ.size());
//   const template_rowt &templ_row = templ[row];
//   if(templ_row.kind==IN) return true_exprt();
//   if(is_row_value_neginf(row_value)) 
//     return implies_exprt(templ_row.post_guard, false_exprt());
//   if(is_row_value_inf(row_value)) 
//     return implies_exprt(templ_row.post_guard, true_exprt());
//   exprt c = implies_exprt(templ_row.post_guard, 
//     binary_relation_exprt(templ_row.expr,ID_le,row_value));
//   rename(c);
//   return c;
// }

// exprt linrank_domaint::get_row_post_constraint(const rowt &row, 
//   const templ_valuet &value)
// {
//   assert(value.size()==templ.size());
//   return get_row_post_constraint(row,value[row]);
// }

// /*******************************************************************\

// Function: linrank_domaint::to_pre_constraints

//   Inputs:

//  Outputs:

//  Purpose: /\_all_rows ( pre_guard ==> (row_expr <= row_value) )

// \*******************************************************************/

// exprt linrank_domaint::to_pre_constraints(const templ_valuet &value)
// {
//   assert(value.size()==templ.size());
//   exprt::operandst c; 
//   for(unsigned row = 0; row<templ.size(); row++)
//   {
//     c.push_back(get_row_pre_constraint(row,value[row]));
//   }
//   return conjunction(c); 
// }

// /*******************************************************************\

// Function: linrank_domaint::make_not_post_constraints

//   Inputs:

//  Outputs:

//  Purpose: for all rows !(post_guard ==> (row_expr <= row_value))
//           to be connected disjunctively

// \*******************************************************************/

// void linrank_domaint::make_not_post_constraints(const templ_valuet &value,
//   exprt::operandst &cond_exprs, 
//   exprt::operandst &value_exprs)
// {
//   assert(value.size()==templ.size());
//   cond_exprs.resize(templ.size());
//   value_exprs.resize(templ.size());

//   exprt::operandst c; 
//   for(unsigned row = 0; row<templ.size(); row++)
//   {
//     value_exprs[row] = templ[row].expr;
//     rename(value_exprs[row]);
//     cond_exprs[row] = not_exprt(get_row_post_constraint(row,value));
//   }
// }


// // void update_constraints(const templ_valuet &value,
// // 			   exprt::operandst &cond_exprs, 
// // 			   exprt::operandst &value_exprs) 
// // {
// // }

// // void get_contraint(const templ_valuet &c, 
// // 		      std::vector<linrank_domaint::row_valuet> &values)
// // {
// // }


// /*******************************************************************\

// Function: linrank_domaint::get_row_symb_value

//   Inputs:

//  Outputs:

//  Purpose: generates symbolic value symbol

// \*******************************************************************/

// exprt linrank_domaint::get_row_symb_value(const rowt &row)
// {
//   assert(row<templ.size());
//   return symbol_exprt(SYMB_BOUND_VAR+i2string(row),templ[row].expr.type());
// }

// /*******************************************************************\

// Function: linrank_domaint::get_row_symb_pre_constraint

//   Inputs:

//  Outputs:

//  Purpose: pre_guard ==> (row_expr <= symb_value)

// \*******************************************************************/

// exprt linrank_domaint::get_row_symb_pre_constraint(const rowt &row, 
//   const row_valuet &row_value)
// {
//   assert(row<templ.size());
//   const template_rowt &templ_row = templ[row];
//   if(templ_row.kind==OUT || templ_row.kind==OUTL) return true_exprt();
//   return implies_exprt(templ_row.pre_guard, 
//     binary_relation_exprt(templ_row.expr,ID_le,get_row_symb_value(row)));
// }

// /*******************************************************************\

// Function: linrank_domaint::get_row_symb_post_constraint

//   Inputs:

//  Outputs:

//  Purpose: post_guard && (row_expr >= row_symb_value)  (!!!)

// \*******************************************************************/

// exprt linrank_domaint::get_row_symb_post_constraint(const rowt &row)
// {
//   assert(row<templ.size());
//   const template_rowt &templ_row = templ[row];
//   if(templ_row.kind==IN) return true_exprt();
//   exprt c = and_exprt(templ_row.post_guard,
//     binary_relation_exprt(templ_row.expr,ID_ge,get_row_symb_value(row)));
//   rename(c);
//   return c;
// }


// /*******************************************************************\

// Function: linrank_domaint::to_symb_pre_constraints

//   Inputs:

//  Outputs:

//  Purpose: pre_guard ==> (row_expr <= symb_row_value)

// \*******************************************************************/

// exprt linrank_domaint::to_symb_pre_constraints(const templ_valuet &value)
// {
//   assert(value.size()==templ.size());
//   exprt::operandst c; 
//   for(unsigned row = 0; row<templ.size(); row++)
//   {
//     c.push_back(get_row_symb_pre_constraint(row,value[row]));
//   }
//   return conjunction(c); 
// }

// /*******************************************************************\

// Function: linrank_domaint::to_symb_pre_constraints

//   Inputs:

//  Outputs:

//  Purpose: pre_guard ==> (row_expr <= symb_row_value)

// \*******************************************************************/

// exprt linrank_domaint::to_symb_pre_constraints(const templ_valuet &value,
// 						const std::set<rowt> &symb_rows)
// {
//   assert(value.size()==templ.size());
//   exprt::operandst c; 
//   for(unsigned row = 0; row<templ.size(); row++)
//   {
//     if(symb_rows.find(row)!=symb_rows.end())
//       c.push_back(get_row_symb_pre_constraint(row,value[row]));
//     else
//       c.push_back(get_row_pre_constraint(row,value[row]));
//   }
//   return conjunction(c); 
// }

// /*******************************************************************\

// Function: linrank_domaint::to_symb_post_constraints

//   Inputs:

//  Outputs:

//  Purpose: post_guard ==> (row_expr >= symb_row_value)

// \*******************************************************************/

// exprt linrank_domaint::to_symb_post_constraints()
// {
//   exprt::operandst c; 
//   for(unsigned row = 0; row<templ.size(); row++)
//   {
//     c.push_back(get_row_symb_post_constraint(row));
//   }
//   return conjunction(c); 
// }

// /*******************************************************************\

// Function: linrank_domaint::get_row_symb_value_constraint

//   Inputs:

//  Outputs:

//  Purpose: row_value <= symb_row_value

// \*******************************************************************/

// exprt linrank_domaint::get_row_symb_value_constraint(const rowt &row, 
// 						const row_valuet &row_value)
// {
//   if(is_row_value_neginf(row_value)) return false_exprt();
//   if(is_row_value_inf(row_value)) return true_exprt();
//   exprt c = binary_relation_exprt(row_value,ID_le,get_row_symb_value(row));
//   rename(c);
//   return c;
// }


// /*******************************************************************\

// Function: linrank_domaint::get_row_value

//   Inputs:

//  Outputs:

//  Purpose:

// \*******************************************************************/

// linrank_domaint::row_valuet linrank_domaint::get_row_value(
//   const rowt &row, const templ_valuet &value)
// {
//   assert(row<value.size());
//   assert(value.size()==templ.size());
//   return value[row];
// }

// /*******************************************************************\

// Function: linrank_domaint::project_on_loops

//   Inputs:

//  Outputs:

//  Purpose:

// \*******************************************************************/

// void linrank_domaint::project_on_loops(const valuet &value, exprt &result)
// {
//   const templ_valuet &v = static_cast<const templ_valuet &>(value);
//   assert(v.size()==templ.size());
//   exprt::operandst c;
//   c.reserve(templ.size());
//   for(unsigned row = 0; row<templ.size(); row++)
//   {
//     if(templ[row].kind!=LOOP) continue;
//     const row_valuet &row_v = v[row];
//     if(is_row_value_neginf(row_v)) c.push_back(false_exprt());
//     else if(is_row_value_inf(row_v)) c.push_back(true_exprt());
//     else c.push_back(binary_relation_exprt(templ[row].expr,ID_le,row_v));
//   }
//   result = conjunction(c);
// }

// /*******************************************************************\

// Function: linrank_domaint::project_on_inout

//   Inputs:

//  Outputs:

//  Purpose:

// \*******************************************************************/

// void linrank_domaint::project_on_inout(const valuet &value, exprt &result)
// {
//   const templ_valuet &v = static_cast<const templ_valuet &>(value);
//   assert(v.size()==templ.size());
//   exprt::operandst c;
//   c.reserve(templ.size());
//   for(unsigned row = 0; row<templ.size(); row++)
//   {
//     const template_rowt &templ_row = templ[row];
//     if(templ_row.kind==LOOP || templ_row.kind==OUTL) continue;
//     const row_valuet &row_v = v[row];
//     if(is_row_value_neginf(row_v)) c.push_back(false_exprt());
//     else if(is_row_value_inf(row_v)) c.push_back(true_exprt());
//     else c.push_back(binary_relation_exprt(templ_row.expr,ID_le,row_v));
//   }
//   result = conjunction(c);
// }

// /*******************************************************************\

// Function: linrank_domaint::project_on_vars

//   Inputs:

//  Outputs:

//  Purpose:

// \*******************************************************************/

// void linrank_domaint::project_on_vars(const valuet &value, 
// 				       const var_sett &vars, exprt &result)
// {
//   const templ_valuet &v = static_cast<const templ_valuet &>(value);
//   assert(v.size()==templ.size());
//   exprt::operandst c;
//   for(unsigned row = 0; row<templ.size(); row++)
//   {
//     const template_rowt &templ_row = templ[row];

//     std::set<symbol_exprt> symbols;
//     find_symbols(templ_row.expr,symbols);

//     bool pure = true;
//     for(std::set<symbol_exprt>::iterator it = symbols.begin();
// 	it != symbols.end(); it++)
//     {
//       if(vars.find(*it)==vars.end()) 
//       {
//         pure = false;
//         break;
//       }
//     }
//     if(!pure) continue;

//     const row_valuet &row_v = v[row];
//     if(is_row_value_neginf(row_v)) c.push_back(false_exprt());
//     else if(is_row_value_inf(row_v)) c.push_back(true_exprt());
//     else c.push_back(binary_relation_exprt(templ_row.expr,ID_le,row_v));
//   }
//   result = conjunction(c);
// }

// /*******************************************************************\

// Function: linrank_domaint::set_row_value

//   Inputs:

//  Outputs:

//  Purpose:

// \*******************************************************************/

// void linrank_domaint::set_row_value(
//   const rowt &row, const linrank_domaint::row_valuet &row_value, templ_valuet &value)
// {
//   assert(row<value.size());
//   assert(value.size()==templ.size());
//   value[row] = row_value;
// }


// /*******************************************************************\

// Function: linrank_domaint::get_row_max_value

//   Inputs:

//  Outputs:

//  Purpose:

// \*******************************************************************/

// linrank_domaint::row_valuet linrank_domaint::get_max_row_value(
//   const linrank_domaint::rowt &row)
// {
//   const template_rowt &templ_row = templ[row];
//   if(templ_row.expr.type().id()==ID_signedbv)
//   {
//     return to_signedbv_type(templ_row.expr.type()).largest_expr();
//   }
//   if(templ_row.expr.type().id()==ID_unsignedbv)
//   {
//     return to_unsignedbv_type(templ_row.expr.type()).largest_expr();
//   }
//   if(templ_row.expr.type().id()==ID_floatbv) 
//   {
//     ieee_floatt max;
//     max.make_fltmax();
//     return max.to_expr();
//   }
//   assert(false); //type not supported
// }

// /*******************************************************************\

// Function: linrank_domaint::get_row_max_value

//   Inputs:

//  Outputs:

//  Purpose:

// \*******************************************************************/

// linrank_domaint::row_valuet linrank_domaint::get_min_row_value(
//   const linrank_domaint::rowt &row)
// {
//   const template_rowt &templ_row = templ[row];
//   if(templ_row.expr.type().id()==ID_signedbv)
//   {
//     return to_signedbv_type(templ_row.expr.type()).smallest_expr();
//   }
//   if(templ_row.expr.type().id()==ID_unsignedbv)
//   {
//     return to_unsignedbv_type(templ_row.expr.type()).smallest_expr();
//   }
//   if(templ_row.expr.type().id()==ID_floatbv) 
//   {
//     ieee_floatt min;
//     min.make_fltmin();
//     return min.to_expr();
//   }
//   assert(false); //type not supported
// }

// /*******************************************************************\

// Function: linrank_domaint::output_value

//   Inputs:

//  Outputs:

//  Purpose:

// \*******************************************************************/

// void linrank_domaint::output_value(std::ostream &out, const valuet &value, 
//   const namespacet &ns) const
// {
//   const templ_valuet &v = static_cast<const templ_valuet &>(value);
//   for(unsigned row = 0; row<templ.size(); row++)
//   {
//     const template_rowt &templ_row = templ[row];
//     switch(templ_row.kind)
//     {
//     case LOOP:
//       out << "(LOOP) [ " << from_expr(ns,"",templ_row.pre_guard) << " | ";
//       out << from_expr(ns,"",templ_row.post_guard) << " ] ===> " << std::endl << "       ";
//       break;
//     case IN: out << "(IN)   "; break;
//     case OUT: case OUTL: out << "(OUT)  "; break;
//     default: assert(false);
//     }
//     out << "( " << from_expr(ns,"",templ_row.expr) << " <= ";
//     if(is_row_value_neginf(v[row])) out << "-oo";
//     else if(is_row_value_inf(v[row])) out << "oo";
//     else out << from_expr(ns,"",v[row]);
//     out << " )" << std::endl;
//   }
// }

// /*******************************************************************\

// Function: linrank_domaint::output_domain

//   Inputs:

//  Outputs:

//  Purpose:

// \*******************************************************************/

// void linrank_domaint::output_domain(std::ostream &out, const namespacet &ns) const
// {
//   for(unsigned row = 0; row<templ.size(); row++)
//   {
//     const template_rowt &templ_row = templ[row];
//     switch(templ_row.kind)
//     {
//     case LOOP:
//       out << "(LOOP) [ " << from_expr(ns,"",templ_row.pre_guard) << " | ";
//       out << from_expr(ns,"",templ_row.post_guard) << " ] ===> " << std::endl << "      ";
//       break;
//     case IN: 
//       out << "(IN)   ";
//       out << from_expr(ns,"",templ_row.pre_guard) << " ===> " << std::endl << "      ";
//       break;
//     case OUT: case OUTL:
//       out << "(OUT)  "; 
//       out << from_expr(ns,"",templ_row.post_guard) << " ===> " << std::endl << "      ";
//       break;
//     default: assert(false);
//     }
//     out << "( " << 
//         from_expr(ns,"",templ_row.expr) << " <= CONST )" << std::endl;
//   }
// }

// /*******************************************************************\

// Function: linrank_domaint::template_size

//   Inputs:

//  Outputs:

//  Purpose:

// \*******************************************************************/

// unsigned linrank_domaint::template_size()
// {
//   return templ.size();
// }

// /*******************************************************************\

// Function: linrank_domaint::is_row_value_neginf

//   Inputs:

//  Outputs:

//  Purpose:

// \*******************************************************************/

// bool linrank_domaint::is_row_value_neginf(const row_valuet & row_value) const
// {
//   return row_value.get(ID_value)==ID_false;
// }

// /*******************************************************************\

// Function: linrank_domaint::is_row_value_inf

//   Inputs:

//  Outputs:

//  Purpose:

// \*******************************************************************/

// bool linrank_domaint::is_row_value_inf(const row_valuet & row_value) const
// {
//   return row_value.get(ID_value)==ID_true;
// }


// /*******************************************************************\

// Function: make_interval_template

//   Inputs:

//  Outputs:

//  Purpose:

// \*******************************************************************/

// void linrank_domaint::add_interval_template(templatet &templ, 
// 					      const var_specst &var_specs,
// 					      const namespacet &ns)
// {
//   unsigned size = 2*var_specs.size();
//   templ.reserve(templ.size()+size);
  
//   for(var_specst::const_iterator v = var_specs.begin(); 
//       v!=var_specs.end(); v++)
//   {
//     if(v->kind==IN) continue; //TODO: must be done in caller (for preconditions, e.g.)

//     // x
//     {
//       templ.push_back(template_rowt());
//       template_rowt &templ_row = templ.back();
//       templ_row.expr = v->var;
//       templ_row.pre_guard = v->pre_guard;
//       templ_row.post_guard = v->post_guard;
//       templ_row.kind = v->kind;
//     }

//     // -x
//     {
//       templ.push_back(template_rowt());
//       template_rowt &templ_row = templ.back();
//       unary_minus_exprt um_expr(v->var,v->var.type());
//       extend_expr_types(um_expr);
//       templ_row.expr = um_expr;
//       templ_row.pre_guard = v->pre_guard;
//       templ_row.post_guard = v->post_guard;
//       templ_row.kind = v->kind;
//     }
//   }
// }

// /*******************************************************************\

// Function: make_zone_template

//   Inputs:

//  Outputs:

//  Purpose:

// \*******************************************************************/

// void linrank_domaint::add_zone_template(templatet &templ, 
// 					  const var_specst &var_specs,
// 					  const namespacet &ns)
// { 
//   unsigned size = 2*var_specs.size()+var_specs.size()*(var_specs.size()-1);
//   templ.reserve(templ.size()+size);
  
//   for(var_specst::const_iterator v1 = var_specs.begin(); 
//       v1!=var_specs.end(); v1++)
//   {
//     if(v1->kind!=IN) //TODO: must be done in caller (for preconditions, e.g.)
//     {
//       // x
//       {
// 	templ.push_back(template_rowt());
// 	template_rowt &templ_row = templ.back();
// 	templ_row.expr = v1->var;
// 	templ_row.pre_guard = v1->pre_guard;
// 	templ_row.post_guard = v1->post_guard;
// 	templ_row.kind = v1->kind;
//       }

//       // -x
//       {
// 	templ.push_back(template_rowt());
// 	template_rowt &templ_row = templ.back();
// 	unary_minus_exprt um_expr(v1->var,v1->var.type());
// 	extend_expr_types(um_expr);
// 	templ_row.expr = um_expr;
// 	templ_row.pre_guard = v1->pre_guard;
// 	templ_row.post_guard = v1->post_guard;
// 	templ_row.kind = v1->kind;
//       }
//     }

//     var_specst::const_iterator v2 = v1; v2++;
//     for(; v2!=var_specs.end(); v2++)
//     {
//       kindt k = domaint::merge_kinds(v1->kind,v2->kind);
//       if(k==IN) continue; //TODO: must be done in caller (for preconditions, e.g.)

//       exprt pre_g = and_exprt(v1->pre_guard,v2->pre_guard);
//       exprt post_g = and_exprt(v1->post_guard,v2->post_guard);
//       simplify(pre_g,ns);
//       simplify(post_g,ns);

//       // x1 - x2
//       {
// 	templ.push_back(template_rowt());
// 	template_rowt &templ_row = templ.back();
//         minus_exprt m_expr(v1->var,v2->var);
//         extend_expr_types(m_expr);
// 	templ_row.expr = m_expr;
// 	templ_row.pre_guard = pre_g;
// 	templ_row.post_guard = post_g;
// 	templ_row.kind = k;
//       }

//       // x2 - x1
//       {
// 	templ.push_back(template_rowt());
// 	template_rowt &templ_row = templ.back();
//         minus_exprt m_expr(v2->var,v1->var);
//         extend_expr_types(m_expr);
// 	templ_row.expr = m_expr;
// 	templ_row.pre_guard = pre_g;
// 	templ_row.post_guard = post_g;
// 	templ_row.kind = k;
//       }
//     }
//   }
// }

// /*******************************************************************\

// Function: make_octagon_template

//   Inputs:

//  Outputs:

//  Purpose:

// \*******************************************************************/

// void linrank_domaint::add_octagon_template(templatet &templ,
// 					     const var_specst &var_specs,
// 					     const namespacet &ns)
// {
//   unsigned size = 2*var_specs.size()+2*var_specs.size()*(var_specs.size()-1);
//   templ.reserve(templ.size()+size);
  
//   for(var_specst::const_iterator v1 = var_specs.begin(); 
//       v1!=var_specs.end(); v1++)
//   {
//     if(v1->kind!=IN) //TODO: must be done in caller (for preconditions, e.g.)
//     {
//       // x
//       {
// 	templ.push_back(template_rowt());
// 	template_rowt &templ_row = templ.back();
// 	templ_row.expr = v1->var;
// 	templ_row.pre_guard = v1->pre_guard;
// 	templ_row.post_guard = v1->post_guard;
// 	templ_row.kind = v1->kind;
//       }

//       // -x
//       {
// 	templ.push_back(template_rowt());
// 	template_rowt &templ_row = templ.back();
// 	unary_minus_exprt um_expr(v1->var,v1->var.type());
// 	extend_expr_types(um_expr);
// 	templ_row.expr = um_expr;
// 	templ_row.pre_guard = v1->pre_guard;
// 	templ_row.post_guard = v1->post_guard;
// 	templ_row.kind = v1->kind;
//       }
//     }

//     var_specst::const_iterator v2 = v1; v2++;
//     for(; v2!=var_specs.end(); v2++)
//     {
//       kindt k = domaint::merge_kinds(v1->kind,v2->kind);
//       if(k==IN) continue; //TODO: must be done in caller (for preconditions, e.g.)

//       exprt pre_g = and_exprt(v1->pre_guard,v2->pre_guard);
//       exprt post_g = and_exprt(v1->post_guard,v2->post_guard);
//       simplify(pre_g,ns);
//       simplify(post_g,ns);

//       // x1 - x2
//       {
// 	templ.push_back(template_rowt());
// 	template_rowt &templ_row = templ.back();
//         minus_exprt m_expr(v1->var,v2->var);
//         extend_expr_types(m_expr);
// 	templ_row.expr = m_expr;
// 	templ_row.pre_guard = pre_g;
// 	templ_row.post_guard = post_g;
// 	templ_row.kind = k;
//       }

//       // -x1 + x2
//       {
// 	templ.push_back(template_rowt());
// 	template_rowt &templ_row = templ.back();
//         minus_exprt m_expr(v2->var,v1->var);
//         extend_expr_types(m_expr);
// 	templ_row.expr = m_expr;
// 	templ_row.pre_guard = pre_g;
// 	templ_row.post_guard = post_g;
// 	templ_row.kind = k;
//       }

//       // -x1 - x2
//       {
// 	templ.push_back(template_rowt());
// 	template_rowt &templ_row = templ.back();
//         minus_exprt p_expr(unary_minus_exprt(v1->var,v1->var.type()),v2->var);
//         extend_expr_types(p_expr);
// 	templ_row.expr = p_expr;
// 	templ_row.pre_guard = pre_g;
// 	templ_row.post_guard = post_g;
// 	templ_row.kind = k;
//       }

//       // x1 + x2
//       {
// 	templ.push_back(template_rowt());
// 	template_rowt &templ_row = templ.back();
//         plus_exprt p_expr(v1->var,v2->var);
//         extend_expr_types(p_expr);
// 	templ_row.expr = p_expr;
// 	templ_row.pre_guard = pre_g;
// 	templ_row.post_guard = post_g;
// 	templ_row.kind = k;
//       }
//     }
//   }

//}

