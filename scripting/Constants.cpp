//
//  Constants.cpp
//  scripting
//
//  Created by Vassili Kaplan on 29/03/16.
//  Copyright © 2016 Vassili Kaplan. All rights reserved.
//

#include "Constants.h"

const string Constants::EMPTY = "";
const string Constants::WHITES = " \t\r";
const string Constants::END_ARG_STR(1, END_ARG);
const string Constants::NULL_ACTION(1, END_ARG);
const string Constants::NEW_LINE = "\n";
const string Constants::INVALID_CHAR = "\1";

const string Constants::END_PARSING_STR    = SPACE + END_ARG_STR +
                   END_GROUP + NEW_LINE + END_STATEMENT;
const string Constants::FUNC_SIG_SEP       = NEXT_ARG + END_ARG_STR;
const string Constants::NEXT_OR_END        = END_PARSING_STR + NEXT_ARG;
const string Constants::START_ARRAY_OR_END =
                   END_PARSING_STR + START_ARRAY + END_ARRAY;
const string Constants::TOKEN_SEPARATION   = "<>=+-*/%&|^,!()[]{}\t\n; ";

const string Constants::ASSIGN      = "=";
const string Constants::DECREMENT   = "--";
const string Constants::INCREMENT   = "++";
const string Constants::NOT         = "!";

const string Constants::BREAK       = "break";
const string Constants::CATCH       = "catch";
const string Constants::COMMENT     = "//";
const string Constants::CONTINUE    = "continue";
const string Constants::ELSE        = "else";
const string Constants::ELSE_IF     = "elif";
const string Constants::EXIT        = "exit";
const string Constants::FOR         = "for";
const string Constants::FUNCTION    = "function";
const string Constants::IF          = "if";
const string Constants::INCLUDE     = "include";
const string Constants::RETURN      = "return";
const string Constants::SIZE        = "size";
const string Constants::TRY         = "try";
const string Constants::THROW       = "throw";
const string Constants::WHILE       = "while";

const string Constants::ABS         = "abs";
const string Constants::ADD         = "add";
const string Constants::ALL         = "all";
const string Constants::ALLVARS     = "allvars";
const string Constants::APPENDLINE  = "appendline";
const string Constants::CEIL        = "ceil";
const string Constants::CONTAINS    = "contains";
const string Constants::COS         = "cos";
const string Constants::EXP         = "exp";
const string Constants::FLOOR       = "floor";
const string Constants::ISNULL      = "isnull";
const string Constants::INDEX_OF    = "indexof";
const string Constants::JOIN        = "join";
const string Constants::LOCK        = "lock";
const string Constants::LOG         = "log";
const string Constants::MORE        = "more";
const string Constants::PI          = "pi";
const string Constants::POW         = "pow";
const string Constants::PRINT       = "print";
const string Constants::PRINT_BLACK = "printblack";
const string Constants::PRINT_GRAY  = "printgray";
const string Constants::PRINT_GREEN = "printgreen";
const string Constants::PRINT_RED   = "printred";
const string Constants::PRINT_WHITE = "printwhite";
const string Constants::PSTIME      = "pstime";
const string Constants::ROUND       = "round";
const string Constants::READ        = "read";
const string Constants::READFILE    = "readfile";
const string Constants::READNUM     = "readnum";
const string Constants::RUN         = "run";
const string Constants::SHOW        = "show";
const string Constants::SIGNAL      = "signal";
const string Constants::SIN         = "sin";
const string Constants::SLEEP       = "sleep";
const string Constants::SQRT        = "sqrt";
const string Constants::SUBSTR      = "substr";
const string Constants::TAIL        = "tail";
const string Constants::THREAD      = "thread";
const string Constants::THREAD_ID   = "threadid";
const string Constants::THREAD_J    = "threadj";
const string Constants::TOUCH       = "touch";
const string Constants::TRANSLATE   = "translate";
const string Constants::TYPE        = "type";
const string Constants::WAIT        = "wait";
const string Constants::WRITE       = "write";
const string Constants::WRITEFILE   = "writefile";

const string Constants::CD          = "cd";
const string Constants::CD__        = "cd..";
const string Constants::COPY        = "cp";
const string Constants::ENV         = "env";
const string Constants::EXISTS      = "exists";
const string Constants::FIND        = "find";
const string Constants::GREP        = "grep";
const string Constants::LS          = "ls";
const string Constants::MKDIR       = "mkdir";
const string Constants::MOVE        = "mv";
const string Constants::SETENV      = "setenv";
const string Constants::PWD         = "pwd";
const string Constants::RM          = "rm";

vector<string> Constants::FUNCT_WITH_SPACE = { APPENDLINE, CD, CD__, COPY, EXISTS, FIND, FUNCTION,
    GREP, INCLUDE, LS, MKDIR, MORE, MOVE, PWD, READFILE, RM, RUN, SHOW, TRANSLATE, TAIL, TOUCH, WRITEFILE  };
vector<string> Constants::FUNCT_WITH_SPACE_ONCE = { RETURN, THROW };

const vector<string> Constants::MATH_ACTIONS = { "&&", "||", "==", "!=", "<=", ">=", "++", "--",
                                                "%", "*", "/", "+", "-", "^", "<", ">", "=" };
const vector<string> Constants::OPER_ACTIONS = { "+=", "-=", "*=", "/=", "%=", "&=", "|=", "^=" };


set<string> Constants::ELSE_LIST    = {ELSE};
set<string> Constants::ELSE_IF_LIST = {ELSE_IF};
set<string> Constants::CATCH_LIST   = {CATCH};

set<string> Constants::CONTROL_FLOW = {BREAK, CONTINUE, FUNCTION, IF, INCLUDE,
                                       WHILE, RETURN, THROW, TRY};

const string Constants::ENGLISH  = "en";
const string Constants::GERMAN   = "de";
const string Constants::SPANISH  = "es";
const string Constants::RUSSIAN  = "ru";
const string Constants::SYNONYMS = "sy";

string Constants::language(const string& lang) {
  if (lang == "English")  { return Constants::ENGLISH; }
  if (lang == "German")   { return Constants::GERMAN; }
  if (lang == "Russian")  { return Constants::RUSSIAN; }
  if (lang == "Spanish")  { return Constants::SPANISH; }
  if (lang == "Synonyms") { return Constants::SYNONYMS; }
  if (lang == "English")  { return Constants::ENGLISH; }
  return lang;
}

vector<string> initActions()
{
    vector<string> allActions(Constants::OPER_ACTIONS);
  
    allActions.insert(allActions.end(), Constants::MATH_ACTIONS.begin(),
                      Constants::MATH_ACTIONS.end());
    return allActions;
}

const vector<string> Constants::ACTIONS(initActions());

unordered_map<string, int> initPriority()
{
    unordered_map<string, int> prio;
    prio["++"] = 10;
    prio["--"] = 10;
    prio["^"]  = 9;
    prio["%"]  = 8;
    prio["*"]  = 8;
    prio["/"]  = 8;
    prio["+"]  = 7;
    prio["-"]  = 7;
    prio["<"]  = 6;
    prio[">"]  = 6;
    prio["<="] = 6;
    prio[">="] = 6;
    prio["=="] = 5;
    prio["!="] = 5;
    prio["&&"] = 4;
    prio["||"] = 3;
    prio["+="] = 2;
    prio["-="] = 2;
    prio["*="] = 2;
    prio["/="] = 2;
    prio["%="] = 2;
    prio["="]  = 2;

    return prio;
}

const unordered_map<string, int> Constants::PRIORITY(initPriority());

string Constants::typeToString(Type type)
{
  switch (type) {
    case NUMBER:             return "NUMBER";
    case STRING:             return "STRING";
    case ARRAY:              return "ARRAY";
    case BREAK_STATEMENT:    return "BREAK";
    case CONTINUE_STATEMENT: return "CONTINUE";
    default:                 return "NONE";
  }
}
