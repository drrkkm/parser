#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <fstream>

#include <boost/tokenizer.hpp>
#include <boost/algorithm/algorithm.hpp>

#include <boost/fusion/adapted.hpp>
#include <boost/variant.hpp>
#include <boost/dll/import.hpp>
#include <boost/functional/overloaded_function.hpp>
#include <boost/spirit/home/x3.hpp>

#include "plugin_api.hpp"

//#include "../manager.h"

namespace dll = boost::dll;
namespace x3 = boost::spirit::x3;


using Expression = boost::variant<
      double
    , boost::recursive_wrapper<struct VariableExpression>
    , boost::recursive_wrapper<struct UnaryExpression>
    , boost::recursive_wrapper<struct BinaryExpression>
    , boost::recursive_wrapper<struct FunctionCall>
    , boost::recursive_wrapper<struct IndexExpression>
    , boost::recursive_wrapper<struct VariableExpression>
    >;


struct VariableExpression {
    std::string name;
};

BOOST_FUSION_ADAPT_STRUCT(VariableExpression, name);


struct IndexExpression {
    std::string name;
    std::vector<int> args;
};

BOOST_FUSION_ADAPT_STRUCT(IndexExpression, name, args);


struct UnaryExpression {
    enum op_t { Plus, Minus, };

    op_t op;
    Expression arg;
};

BOOST_FUSION_ADAPT_STRUCT(UnaryExpression, op, arg)

struct EqualityExpression {
//     enum op_t { Equality };
//     VariableExpression name;
//     std::vector<std::pair<op_t, Expression>> ops;
    Expression first;
    Expression last;
};

BOOST_FUSION_ADAPT_STRUCT(EqualityExpression, first, last)
// BOOST_FUSION_ADAPT_STRUCT(UnaryExpression, op, arg)


struct BinaryExpression {
    enum op_t { Plus, Minus, Mul, Div, Mod, Pow, Equality};

    Expression first;
    std::vector<std::pair<op_t, Expression>> ops;
};

BOOST_FUSION_ADAPT_STRUCT(BinaryExpression, first, ops)


struct FunctionCall {
    std::string function;
    std::vector<Expression> args;
};

BOOST_FUSION_ADAPT_STRUCT(FunctionCall, function, args)

// struct equality_op_t : x3::symbols<IndexExpression::op_t> {
//     equality_op_t();
// } const equality_op;


struct unary_op_t : x3::symbols<UnaryExpression::op_t> {
    unary_op_t();
} const unary_op;


struct binary_op : x3::symbols<BinaryExpression::op_t> {
    explicit binary_op(int precedence);
};

x3::rule<class index_variable, IndexExpression> const index_variable;
x3::rule<class variable, VariableExpression> const variable;
x3::rule<class identifier, std::string> const identifier;
x3::rule<class simple_expr, Expression> const simple_expr;
x3::rule<class unary_expr, UnaryExpression> const unary_expr;
x3::rule<class function_call, FunctionCall> const function_call;
x3::rule<class binary_expr_1, BinaryExpression> const binary_expr_1;
x3::rule<class binary_expr_2, BinaryExpression> const binary_expr_2;
x3::rule<class binary_expr_3, BinaryExpression> const binary_expr_3;
x3::rule<class expr, Expression> const expr;

auto const identifier_def = x3::raw[+(x3::alpha | '_')];

auto const simple_expr_def
    = x3::double_
    | '(' >> expr >> ')'
    | unary_expr
    | function_call
    | index_variable
    | variable
    ;

auto const unary_expr_def = unary_op >> simple_expr;

//auto const equality_expr_def = simple_expr >> '=' >> simple_expr;

auto const function_call_def = identifier >> '(' >>  (expr % ',') >> ')';
auto const variable_def = x3::raw[+(x3::alpha)];
auto const index_variable_def = x3::raw[+(x3::alpha)] >> '[' >> (x3::int_ % ':') >> ']';

auto const binary_expr_1_def = binary_expr_2 >> *(binary_op(1) >> binary_expr_2);
auto const binary_expr_2_def = binary_expr_3 >> *(binary_op(2) >> binary_expr_3);
auto const binary_expr_3_def = simple_expr >> *(binary_op(3) >> simple_expr);

auto const expr_def = binary_expr_1;


BOOST_SPIRIT_DEFINE(
    identifier,
    simple_expr,
    unary_expr,
    function_call,
    binary_expr_1, binary_expr_2, binary_expr_3,
    expr, variable, index_variable);

class FormulaParser {
    
private:
    std::map<std::string, int> constants;
    std::string path = "./";
    bool presence_of_IndexExpression = false; 
    int index = 0; 
    int maxindex = 0;
    double eval_binary(BinaryExpression::op_t op, double a, double b);
    double eval(Expression e);
    double return_variable(std::string name);
    void load_constants(std::string path);
    Expression preParse(const char* input);

public:
    FormulaParser() { load_constants(path);};
    ~FormulaParser() {};
    std::vector <double> parse(std::string input);
};

