#include "FormulaParser.hpp"
#include "operands.hpp"

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

std::vector<double> FormulaParser::parse(std::string input_t){
    input_t.erase(std::remove_if(input_t.begin(), input_t.end(), isspace), input_t.end());
    const char * input = input_t.c_str();
    std::cout << input_t;
    auto end = input + std::strlen(input);
    Expression result;
    auto ok = phrase_parse(input, end, expr, x3::space, result);
    std::cout << "FormulaParser::preParse(ok) : " << ok << "\n";
//     //std::cout << result << "\n";
     if (ok && input == end) return eval(result);
     throw std::runtime_error(std::string("Failed at: `") + input + "`");
    std::cout << "FormulaParser::parse(exp) : " << exp << "\n";
    return std::vector<double>{};
}

// Expression FormulaParser::preParse(const char* input) {
//     auto end = input + std::strlen(input);
//     Expression result;
//     auto ok = phrase_parse(input, end, expr, x3::space, result);
//     std::cout << "FormulaParser::preParse(ok) : " << ok << "\n";
//     //std::cout << result << "\n";
//     if (ok && input == end) return result;
//     throw std::runtime_error(std::string("Failed at: `") + input + "`");
// }

// std::vector<double> FormulaParser::parse(std::string input){
//     input.erase(std::remove_if(input.begin(), input.end(), isspace), input.end());
//     auto exp = eval(preParse(input.c_str()));
//     std::cout << "FormulaParser::parse(exp) : " << exp << "\n";
//     return exp;
// }

std::vector<double> FormulaParser::eval_binary(BinaryExpression::op_t op, std::vector<double> a, std::vector<double> b) { 
    //if (a.size() > 1 && b.size() > 1) throw std::runtime_error("Impossible to ");

    switch (op) {
    case BinaryExpression::Plus: return a + b;
    case BinaryExpression::Minus: return a - b;
    case BinaryExpression::Mul: return a * b;
     case BinaryExpression::Div: return a / b;
    // case BinaryExpression::Mod: return (int)a % (int)b;
    //case BinaryExpression::Pow: return pow(a, b);
    default: throw std::runtime_error("Unknown operator");
    }
}

std::vector<double> FormulaParser::eval(Expression e) {
    std::vector <double> data{};
    auto visitor = boost::make_overloaded_function(
        [](double x) -> std::vector<double> { return std::vector<double>{x}; },

        [&](const UnaryExpression& e) -> std::vector<double> {
            auto a = eval(e.arg)[0];
            switch (e.op) {
                case UnaryExpression::Plus: return std::vector<double>{+a};
                case UnaryExpression::Minus: return std::vector<double>{-a};
            }
            throw std::runtime_error("Unknown operator");
        },
        [&](const FunctionCall& e) -> std::vector<double> {
            std::vector <double> args = eval(e.args[0]);
            for (int item = 1; item < e.args.size(); item++){
                auto c = eval(e.args[item]);
                args.insert(args.end(), c.begin(), c.end());
            }
            try{
                boost::filesystem::path lib_path(".");
                boost::shared_ptr<plugin_api> plugin = nullptr; 
                std::string plugin_name = "plugin_" + e.function;
                plugin = dll::import_symbol<plugin_api>(lib_path / plugin_name, "plugin", dll::load_mode::append_decorations);
                return std::vector<double>{plugin->calculate(args)};
            }
            catch (...){
                throw std::runtime_error("Unknown function");
            }
        },
        [&](const VariableExpression &e) -> std::vector<double> {
            auto a = e.name;
            if (constants.find(a) != constants.end()) return std::vector<double>{constants[a]};
            throw std::runtime_error("Unknown variable");
        },
        [&](const IndexExpression &e) -> std::vector<double> {
            std::vector <double> args1{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
            if (e.args.size() == 1) return std::vector<double>{args1[e.args[0]]}; // manager.return_variable(e.name, e.args.at(0));
            if (e.args.size() == 2) {
                int g = eval(e.args.at(1))[0];
                int f = eval(e.args.at(0))[0];
                if (g - f < 0) throw std::runtime_error("Wrong indexes");
                for (int item = f; item <= g; item++) {
                        data.push_back(args1[item]);
                    }
                    return data;
            }
            throw std::runtime_error("Too much indexes");
        },
        [&](const BinaryExpression& e) -> std::vector<double> {   
            auto a = eval(e.first);
            //std::cout << "a " << a << "\n";
            //std::cout << "FormulaParser::eval(BinaryExpression case) : ";
            for (auto&& o : e.ops) {
                auto b = eval(o.second);
                a = eval_binary(o.first, a, b);
            }
            return a;
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
