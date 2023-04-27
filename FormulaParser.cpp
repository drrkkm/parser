#include "FormulaParser.hpp"


typedef boost::tokenizer<boost::escaped_list_separator<char>> Tokenizer;
//equality_op_t::equality_op_t(){
//    add ("=", EqualityExpression::Equality);
//}

unary_op_t::unary_op_t() {
    add("+",   UnaryExpression::Plus);
    add("-",   UnaryExpression::Minus);
}


binary_op::binary_op(int precedence) {
    switch (precedence) {
        case 1:
            add("+", BinaryExpression::Plus);
            add("-", BinaryExpression::Minus);
            return;
        case 2:
            add("*",   BinaryExpression::Mul);
            add("/",   BinaryExpression::Div);
            add("mod", BinaryExpression::Mod);
            return;
        case 3:
            add("**", BinaryExpression::Pow);
            return;
        throw std::runtime_error("Unknown precedence ");
    }
}

Expression FormulaParser::preParse(const char* input) {
    auto end = input + std::strlen(input);
    Expression result;
    auto ok = phrase_parse(input, end, expr, x3::space, result);
    if (ok && input == end) return result;
    throw std::runtime_error(std::string("Failed at: `") + input + "`");
}

std::vector<double> FormulaParser::parse(std::string input){
    input.erase(std::remove_if(input.begin(), input.end(), isspace), input.end());
    auto exp = eval(preParse(input.c_str()));
    if (presence_of_IndexExpression){
        std::vector<double> data{exp};
        for(int item = 1; item < maxindex; ++item){
            index = item;
            data.push_back(eval(preParse(input.c_str())));
        }
        return data;
    }
    else {
        return std::vector<double> {exp};
    }
    return std::vector<double>{};
}

double FormulaParser::eval_binary(BinaryExpression::op_t op, double a, double b) { 
    switch (op) {
    case BinaryExpression::Plus: return a + b;
    case BinaryExpression::Minus: return a - b;
    case BinaryExpression::Mul: return a * b;
    case BinaryExpression::Div: return a / b;
    case BinaryExpression::Mod: return (int)a % (int)b;
    case BinaryExpression::Pow: return pow(a, b);
    default: throw std::runtime_error("Unknown operator");
    }
}

double FormulaParser::eval(Expression e) {
    auto visitor = boost::make_overloaded_function(
        [](double x) { return x; },

        [&](const UnaryExpression& e) -> double {

            auto a = eval(e.arg);
            switch (e.op) {
                case UnaryExpression::Plus: return +a;
                case UnaryExpression::Minus: return -a;
            }
            throw std::runtime_error("Unknown operator");
        },
        [&](const FunctionCall& e) -> double {
            
            std::vector <double> args;
            for (int item = 0; item < e.args.size(); item++){
                args.push_back(eval(e.args.at(item)));
            }
            try{
                boost::filesystem::path lib_path(".");
                boost::shared_ptr<plugin_api> plugin = nullptr; 
                std::string plugin_name = "plugin_" + e.function;
                plugin = dll::import_symbol<plugin_api>(lib_path / plugin_name, "plugin", dll::load_mode::append_decorations);
                return plugin->calculate(args);
            }
            catch (...){
                throw std::runtime_error("Unknown function");
            }
        },
        [&](const BinaryExpression& e) -> double {   
            auto a = eval(e.first);
            for (auto&& o : e.ops) {
                auto b = eval(o.second);
                a = eval_binary(o.first, a, b);
            }
            return a;
        },
        [&](const VariableExpression &e) -> double {
            auto a = e.name;
            if (constants.find(a) != constants.end()) return constants[a];
            throw std::runtime_error("Unknown variable");
        },
        [&](const IndexExpression &e) -> double {
            std::vector <int> args1{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
            if (e.args.size() == 1) return args1[eval(e.args.at(0))]; // manager.return_variable(e.name, e.args.at(0));
            if (e.args.size() == 2) {
                if (presence_of_IndexExpression && index != 0){
                    return args1[eval(e.args.at(0)) + index];
                }
                else if (presence_of_IndexExpression && index == 0) {
                    if (eval(e.args.at(1)) - eval(e.args.at(0)) + 1 != maxindex) {index = -1; throw std::runtime_error("Different sizes");}
                    return args1[eval(e.args.at(0))];
                }
                else {
                    if (eval(e.args.at(1)) - eval(e.args.at(0)) < 0) throw std::runtime_error("Wrong indexes");
                    presence_of_IndexExpression = true;
                    maxindex = eval(e.args.at(1)) - eval(e.args.at(0)) + 1;
                    return args1[eval(e.args.at(0))];
                }
            }
            throw std::runtime_error("Too much indexes");
        });
    return boost::apply_visitor(visitor, e);
}

void FormulaParser::load_constants(std::string path){
    std::string name;
    double value = 0;
    std::fstream fin;
    try{
        fin.open(path + "constants.csv", std::ios::in);
        while(fin){
            fin >> name >> value;
            constants[name] = value;
        }
        fin.close();
    }
    catch (...){
        throw std::runtime_error("No constant's file");
    }
}
