#pragma once

#include <any>
#include <regex>
#include <vector>
#include <memory>
#include <sstream>
#include <numeric>
#include <iostream>
#include <typeinfo>
#include <algorithm>
#include <functional>
#include <unordered_map>

namespace scm {

  typedef std::vector<std::any> List;
  typedef std::shared_ptr<List> lst_ptr;
  typedef std::function<std::any(const List & args)> fun_ptr;

  typedef bool Boolean;
  typedef double Number;
  typedef std::string String;

  struct Symbol : public std::string {
    Symbol() = default;
    explicit Symbol(const String & s) : std::string(s) {}
  };

  class Env;
  typedef std::shared_ptr<Env> env_ptr;

  const Symbol _if("if");
  const Symbol _true("#t");
  const Symbol _false("#f");
  const Symbol _quote("quote");
  const Symbol _begin("begin");
  const Symbol _lambda("lambda");
  const Symbol _define("define");

  struct If {
    std::any test, conseq, alt;
  };

  struct Quote {
    std::any exp;
  };

  struct Define {
    Symbol sym;
    std::any exp;
  };

  struct Lambda {
    std::any parms, body;
  };

  struct Begin {
    lst_ptr exps;
  };

  struct Function {
    std::any parms, body;
    env_ptr env;
  };

  template <typename T>
  std::vector<T> any_cast(const List & lst)
  {
    std::vector<T> args(lst.size());
    std::transform(lst.begin(), lst.end(), args.begin(),
      [](std::any exp) { return std::any_cast<T>(exp); });
    return args;
  }

  template <typename T>
  std::vector<T> num_cast(const List& lst)
  {
    std::vector<T> args(lst.size());
    std::transform(lst.begin(), lst.end(), args.begin(),
      [](std::any exp) { 
      double num = any_cast<double>(exp);
      return static_cast<T>(num);
    });
    return args;
  }

  fun_ptr plus = [](const List & lst) {
    std::vector<Number> args = any_cast<Number>(lst);
    return std::accumulate(next(args.begin()), args.end(), args.front(), std::plus<Number>());
  };

  fun_ptr minus = [](const List & lst) {
    std::vector<Number> args = any_cast<Number>(lst);
    return std::accumulate(next(args.begin()), args.end(), args.front(), std::minus<Number>());
  };

  fun_ptr divides = [](const List & lst) {
    std::vector<Number> args = any_cast<Number>(lst);
    return std::accumulate(next(args.begin()), args.end(), args.front(), std::divides<Number>());
  };

  fun_ptr multiplies = [](const List & lst) {
    std::vector<Number> args = any_cast<Number>(lst);
    return std::accumulate(next(args.begin()), args.end(), args.front(), std::multiplies<Number>());
  };

  fun_ptr greater = [](const List & lst) {
    std::vector<Number> args = any_cast<Number>(lst);
    return Boolean(args[0] > args[1]);
  };

  fun_ptr less = [](const List & lst) {
    std::vector<Number> args = any_cast<Number>(lst);
    return Boolean(args[0] < args[1]);
  };

  fun_ptr equal = [](const List & lst) {
    std::vector<Number> args = any_cast<Number>(lst);
    return Boolean(args[0] == args[1]);
  };

  fun_ptr car = [](const List & lst) {
    auto l = std::any_cast<lst_ptr>(lst.front());
    return l->front();
  };

  fun_ptr cdr = [](const List & lst) {
    auto l = std::any_cast<lst_ptr>(lst.front());
    return std::make_shared<List>(next(l->begin()), l->end());
  };

  fun_ptr list = [](const List & lst) {
    return std::make_shared<List>(lst.begin(), lst.end());
  };

  fun_ptr length = [](const List & lst) {
    auto l = std::any_cast<lst_ptr>(lst.front());
    return static_cast<Number>(l->size());
  };

  class Env {
  public:
    Env(std::unordered_map<std::string, std::any> inner)
      : inner(inner)
    {}
    ~Env() = default;

    explicit Env(const std::any & parm, const List & args, env_ptr outer)
      : outer(std::move(outer))
    {
      if (parm.type() == typeid(lst_ptr)) {
        auto parms = std::any_cast<lst_ptr>(parm);
        for (size_t i = 0; i < parms->size(); i++) {
          auto sym = std::any_cast<Symbol>((*parms)[i]);
          this->inner[sym] = args[i];
        }
      }
      else {
        auto sym = std::any_cast<Symbol>(parm);
        this->inner[sym] = args;
      }
    }

    std::any get(Symbol sym)
    {
      if (this->inner.find(sym) != this->inner.end()) {
        return this->inner.at(sym);
      }
      if (this->outer) {
        return this->outer->get(sym);
      }
      throw std::runtime_error("undefined symbol: " + sym);
    }

    std::unordered_map<std::string, std::any> inner;
    env_ptr outer{ nullptr };
  };

  env_ptr global_env()
  {
    return std::make_shared<Env>(
      std::unordered_map<std::string, std::any>{
        { Symbol("pi"), Number(3.14159265358979323846) },
        { Symbol("+"), plus },
        { Symbol("-"), minus },
        { Symbol("/"), divides },
        { Symbol("*"), multiplies },
        { Symbol(">"), greater },
        { Symbol("<"), less },
        { Symbol("="), equal },
        { Symbol("car"), car },
        { Symbol("cdr"), cdr },
        { Symbol("list"), list },
        { Symbol("length"), length }
    });
  }

void print(std::any exp) 
{
  if (exp.type() == typeid(Number)) {
    std::cout << std::any_cast<Number>(exp);
  } 
  else if (exp.type() == typeid(Symbol)) {
    std::cout << std::any_cast<Symbol>(exp);
  }
  else if (exp.type() == typeid(String)) {
    std::cout << std::any_cast<String>(exp);
  }
  else if (exp.type() == typeid(Boolean)) {
    std::string s = std::any_cast<Boolean>(exp) ? "#t" : "#f";
    std::cout << s;
  }
  else if (exp.type() == typeid(Begin)) {
    std::cout << _begin;
  }
  else if (exp.type() == typeid(Define)) {
    std::cout << _define;
  }
  else if (exp.type() == typeid(Lambda)) {
    std::cout << _lambda;
  }
  else if (exp.type() == typeid(If)) {
    std::cout << _if;
  }
  else if (exp.type() == typeid(Quote)) {
    std::cout << _quote;
  }
  else if (exp.type() == typeid(fun_ptr)) {
    std::cout << "function";
  }
  else if (exp.type() == typeid(lst_ptr)) {
    auto & list = *std::any_cast<lst_ptr>(exp);

    std::cout << "(";
    for (auto s : list) {
      print(s);
      std::cout << " ";
    }
    std::cout << ")";
  } else {
    std::cout << "()";
  }
}

std::any eval(std::any exp, env_ptr env)
{
  while (true) {
    if (exp.type() == typeid(Number) ||
        exp.type() == typeid(String) ||
        exp.type() == typeid(Boolean)) {
       return exp;
    }
    if (exp.type() == typeid(Symbol)) {
      auto symbol = std::any_cast<Symbol>(exp);
      return env->get(symbol);
    }
    if (exp.type() == typeid(Define)) {
      auto define = std::any_cast<Define>(exp);
      return env->inner[define.sym] = eval(define.exp, env);
    }
    if (exp.type() == typeid(Lambda)) {
      auto lambda = std::any_cast<Lambda>(exp);
      return Function{ lambda.parms, lambda.body, env };
    }
    if (exp.type() == typeid(Quote)) {
      auto quote = std::any_cast<Quote>(exp);
      return quote.exp;
    }
    if (exp.type() == typeid(If)) {
      auto if_ = std::any_cast<If>(exp);
      exp = std::any_cast<Boolean>(eval(if_.test, env)) ? if_.conseq : if_.alt;
    } 
    else if (exp.type() == typeid(Begin)) {
      auto begin = std::any_cast<Begin>(exp);
      std::transform(next(begin.exps->begin()), begin.exps->end(), begin.exps->begin(),
        std::bind(eval, std::placeholders::_1, env));
      exp = begin.exps->back();
    } 
    else {
      auto list = std::any_cast<lst_ptr>(exp);
      auto call = List(list->size());

      std::transform(list->begin(), list->end(), call.begin(),
        std::bind(eval, std::placeholders::_1, env));

      auto func = call.front();
      auto args = List(next(call.begin()), call.end());

      if (func.type() == typeid(Function)) {
        auto function = std::any_cast<Function>(func);
        exp = function.body;
        env = std::make_shared<Env>(function.parms, args, function.env);
      }
      else {
        auto function = std::any_cast<fun_ptr>(func);
        return function(args);
      }
    }
  }
}

inline std::any parse(std::any exp)
{
  if (exp.type() == typeid(List)) {
    auto list = std::any_cast<List>(exp);
    std::transform(list.begin(), list.end(), list.begin(), parse);

    if (list[0].type() == typeid(Symbol)) {
      auto token = std::any_cast<Symbol>(list[0]);

      if (token == _quote) {
        if (list.size() != 2) {
          throw std::invalid_argument("wrong number of arguments to quote");
        }
        return Quote{ list[1] };
      }
      if (token == _if) {
        if (list.size() != 4) {
          throw std::invalid_argument("wrong number of arguments to if");
        }
        return If{ list[1], list[2], list[3] };
      }
      if (token == _lambda) {
        if (list.size() != 3) {
          throw std::invalid_argument("wrong Number of arguments to lambda");
        }
        return Lambda{ list[1], list[2] };
      }
      if (token == _begin) {
        if (list.size() < 2) {
          throw std::invalid_argument("wrong Number of arguments to begin");
        }
        return Begin{ std::make_shared<List>(list) };
      }
      if (token == _define) {
        if (list.size() < 3 || list.size() > 4) {
          throw std::invalid_argument("wrong number of arguments to define");
        }
        if (list[1].type() != typeid(Symbol)) {
          throw std::invalid_argument("first argument to define must be a Symbol");
        }
        if (list.size() == 3) {
          return Define{ std::any_cast<Symbol>(list[1]), list[2] };
        }
        return Define{ std::any_cast<Symbol>(list[1]), Lambda{ list[2], list[3] } };
      }
    }
    return std::make_shared<List>(list);
  }

  auto token = std::any_cast<std::string>(exp);
  if (token == _true) {
    return true;
  }
  if (token == _false) {
    return false;
  }
  if (token.front() == '"' && token.back() == '"') {
    return String(token.substr(1, token.size() - 2));
  }
  std::regex match_number(R"(^[+-]?([0-9]*[.])?[0-9]+$)");
  if (std::regex_match(token, match_number)) {
    return Number(std::stod(token));
  }
  return Symbol(token);
}

std::any ast(std::vector<std::string>::iterator& token)
{
  if (*token == "(") {
    List list;
    while (*(++token) != ")") {
      list.push_back(ast(token));
    }
    return list;
  }
  else {
    return *token;
  }
}

std::any ast(std::istream_iterator<std::string>& it)
{
  if (*it == "(") {
    List list;
    while (*(++it) != ")") {
      list.push_back(ast(it));
    }
    return list;
  }
  else {
    return *it;
  }
}

std::any read(std::string input)
{
  input = std::regex_replace(input, std::regex(R"([(])"), " ( ");
  input = std::regex_replace(input, std::regex(R"([)])"), " ) ");

  std::vector<std::string> tokens;
  std::regex re(" ");
  std::sregex_token_iterator it(input.begin(), input.end(), re, -1);
  for (it; it != std::sregex_token_iterator(); ++it) {

    std::string token = *it;
    token.erase(std::remove(token.begin(), token.end(), '\n'), token.end());

    if (token == "[[") {
      std::string str("\"");
      while (*(++it) != "]]") {
        str += *it;
        str += " ";
      }
      str += "\"";
      tokens.push_back(str);
    }
    else {
      if (!token.empty()) {
        tokens.push_back(token);
      }
    }
  }

  return parse(ast(tokens.begin()));
};
} // namespace scm