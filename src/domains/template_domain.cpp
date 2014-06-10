#include "template_domain.h"

#include <util/arith_tools.h>
#include <util/ieee_float.h>
#include <langapi/languages.h>

template_domaint::row_valuet template_domaint::between(
  const row_valuet &lower, const row_valuet &upper)
{
  if(lower.type()==upper.type() && 
     (lower.type().id()==ID_signedbv || lower.type().id()==ID_unsignedbv))
  {
    mp_integer vlower, vupper;
    to_integer(lower, vlower);
    to_integer(upper, vupper);
    assert(vupper>=vlower);
    return from_integer((vupper-vlower)/2,lower.type());
  }
  if(lower.type().id()==ID_floatbv && upper.type().id()==ID_floatbv)
  {
    ieee_floatt vlower(to_constant_expr(lower));
    ieee_floatt vupper(to_constant_expr(upper));
    if(vlower.get_sign()==vupper.get_sign()) 
    {
      mp_integer plower = vlower.pack(); //compute "median" float number
      mp_integer pupper = vupper.pack();
      //assert(pupper>=plower);
      ieee_floatt res;
      res.unpack((plower-pupper)/2); //...by computing integer mean
      return res.to_expr();
    }
    ieee_floatt res;
    res.make_zero();
    return res.to_expr();
  }
  assert(false); //types do not match or are not supported
}

bool template_domaint::leq(const row_valuet &v1, const row_valuet &v2)
{
  if(v1.type()==v2.type() && 
     (v1.type().id()==ID_signedbv || v1.type().id()==ID_unsignedbv))
  {
    mp_integer vv1, vv2;
    to_integer(v1, vv1);
    to_integer(v2, vv2);
    return vv1<=vv2;
  }
  if(v1.type().id()==ID_floatbv && v2.type().id()==ID_floatbv)
  {
    ieee_floatt vv1(to_constant_expr(v1));
    ieee_floatt vv2(to_constant_expr(v2));
    return vv1<=vv2;
  }
  assert(false); //types do not match or are not supported
}

exprt template_domaint::get_row_constraint(const rowt &row, const row_valuet &row_value)
{
  assert(row<templ.size());
  return binary_relation_exprt(templ[row],ID_le,row_value);
}

exprt template_domaint::get_row_constraint(const rowt &row, const valuet &value)
{
  assert(row<templ.size());
  assert(value.size()==templ.size());
  return binary_relation_exprt(templ[row],ID_le,value[row]);
}

exprt template_domaint::to_constraints(const valuet &value)
{
  assert(value.size()==templ.size());
  exprt::operandst c; 
  for(unsigned row = 0; row<templ.size(); row++)
  {
    c.push_back(binary_relation_exprt(templ[row],ID_le,value[row]));
  }
  return conjunction(c); 
}

void template_domaint::make_not_constraints(const valuet &value,
  exprt::operandst &cond_exprs, 
  exprt::operandst &value_exprs)
{
  assert(value.size()==templ.size());
  cond_exprs.clear();
  cond_exprs.reserve(templ.size());
  value_exprs.clear();
  value_exprs.reserve(templ.size());

  exprt::operandst c; 
  for(unsigned row = 0; row<templ.size(); row++)
  {
    value_exprs[row] = templ[row];
    cond_exprs[row] = binary_relation_exprt(value_exprs[row],ID_gt,value[row]);
    c.push_back(cond_exprs[row]);
  }
}

inline template_domaint::row_valuet template_domaint::get_row_value(
  const rowt &row, const valuet &value)
{
  assert(row<value.size());
  assert(value.size()==templ.size());
  return value[row];
}

inline void template_domaint::set_row_value(
  const rowt &row, const template_domaint::row_valuet &row_value, valuet &value)
{
  assert(row<value.size());
  assert(value.size()==templ.size());
  value[row] = row_value;
}

template_domaint::row_valuet template_domaint::get_max_row_value(
  const template_domaint::rowt &row)
{
  if(templ[row].type().id()==ID_signedbv)
  {
    return to_signedbv_type(templ[row].type()).largest_expr();
  }
  if(templ[row].type().id()==ID_unsignedbv)
  {
    return to_unsignedbv_type(templ[row].type()).largest_expr();
  }
  if(templ[row].type().id()==ID_floatbv) 
  {
    ieee_floatt max;
    max.make_fltmax();
    return max.to_expr();
  }
  assert(false); //type not supported
}


void template_domaint::output_value(std::ostream &out, const valuet &value, 
  const namespacet &ns) const
{
  for(unsigned row = 0; row<templ.size(); row++)
  {
    out << from_expr(ns,"",templ[row]) << " <= " << from_expr(ns,"",value[row]) << std::endl;
  }
}

void template_domaint::output_template(std::ostream &out, const namespacet &ns) const
{
  for(unsigned row = 0; row<templ.size(); row++)
  {
    out << from_expr(ns,"",templ[row]) << " <= CONST" << std::endl;
  }
}

unsigned template_domaint::template_size()
{
  return templ.size();
}





void make_interval_template(template_domaint::templatet &templ, 
  const template_domaint::var_listt &vars)
{
  templ.clear();
  templ.reserve(2*vars.size());
  for(template_domaint::var_listt::const_iterator v = vars.begin(); 
      v!=vars.end(); v++)
  {
    templ.push_back(*v);
    templ.push_back(unary_minus_exprt(*v,v->type()));
  }
}

void make_zone_template(template_domaint::templatet &templ, 
  const template_domaint::var_listt &vars)
{ 
  templ.clear();
  templ.reserve(2*vars.size()+vars.size()*(vars.size()-1));
  for(template_domaint::var_listt::const_iterator v1 = vars.begin(); 
      v1!=vars.end(); v1++)
  {
    templ.push_back(*v1);
    templ.push_back(unary_minus_exprt(*v1,v1->type()));
    for(template_domaint::var_listt::const_iterator v2 = v1; 
        v2!=vars.end(); v2++)
    {
      templ.push_back(minus_exprt(*v1,*v2));
      templ.push_back(minus_exprt(*v2,*v1));
    }
  }
}

void make_octagon_template(template_domaint::templatet &templ,
  const template_domaint::var_listt &vars)
{
  templ.clear();
  templ.reserve(2*vars.size()+2*vars.size()*(vars.size()-1));
  for(template_domaint::var_listt::const_iterator v1 = vars.begin(); 
      v1!=vars.end(); v1++)
  {
    templ.push_back(*v1);
    templ.push_back(unary_minus_exprt(*v1,v1->type()));
    for(template_domaint::var_listt::const_iterator v2 = v1; 
        v2!=vars.end(); v2++)
    {
      templ.push_back(minus_exprt(*v1,*v2));
      templ.push_back(minus_exprt(*v2,*v1));
      templ.push_back(plus_exprt(*v1,*v2));
      templ.push_back(plus_exprt(*v2,*v1));
    }
  }
}
