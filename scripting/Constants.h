//
//  Constants.h
//  scripting
//
//  Created by Vassili Kaplan on 29/03/16.
//  Copyright © 2016 Vassili Kaplan. All rights reserved.
//

#ifndef Constants_h
#define Constants_h

#include <set>
#include <stack>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using namespace std;

class Constants
{
public:
  
  enum Type {
    NONE,
    NUMBER,
    STRING,
    ARRAY,
    BREAK_STATEMENT,
    CONTINUE_STATEMENT
  };
  
  static const size_t MAX_LOOPS         = 100000;
  static const size_t MAX_CHARS_TO_SHOW = 40;
  static const size_t MAX_TAIL_LINES    = 10;
  static const size_t MAX_VAR_SIZE      = 2048;
  static const size_t MAX_PROMPT_SIZE   = 32;
  static const size_t INDENT            = 2;

  static const char START_ARG     = '(';
  static const char END_ARG       = ')';
  static const char START_GROUP   = '{';
  static const char END_GROUP     = '}';
  static const char START_ARRAY   = '[';
  static const char END_ARRAY     = ']';
  static const char NEXT_ARG      = ',';
  static const char END_LINE      = '\n';
  static const char NULL_CHAR     = '\0';
  static const char NEXT_LINE     = '\\';
  static const char SPACE         = ' ';
  static const char QUOTE         = '"';
  static const char VAR_START     = '$';
  static const char FOR_ANY       = ':';
  static const char END_STATEMENT = ';';
  
  static const string ASSIGN;
  static const string DECREMENT;
  static const string INCREMENT;
  static const string NOT;
  
  static const string EMPTY;
  static const string END_ARG_STR;
  static const string END_PARSING_STR;
  static const string FUNC_SIG_SEP;
  static const string INVALID_CHAR;
  static const string NEXT_OR_END;
  static const string NEW_LINE;
  static const string NULL_ACTION;
  static const string START_ARRAY_OR_END;
  static const string TOKEN_SEPARATION;
  static const string WHITES;
  
  static const string BREAK;
  static const string CATCH;
  static const string COMMENT;
  static const string CONTINUE;
  static const string ELSE;
  static const string ELSE_IF;
  static const string EXIT;
  static const string FOR;
  static const string FUNCTION;
  static const string IF;
  static const string INCLUDE;
  static const string RETURN;
  static const string SIZE;
  static const string THROW;
  static const string TRY;
  static const string WHILE;
  
  static const string ABS;
  static const string ADD;
  static const string ALL;
  static const string ALLVARS;
  static const string APPENDLINE;
  static const string CEIL;
  static const string CONTAINS;
  static const string COS;
  static const string EXP;
  static const string FLOOR;
  static const string INDEX_OF;
  static const string ISNULL;
  static const string LOG;
  static const string MORE;
  static const string PI;
  static const string POW;
  static const string PRINT;
  static const string PRINT_BLACK;
  static const string PRINT_GRAY;
  static const string PRINT_GREEN;
  static const string PRINT_RED;
  static const string PRINT_WHITE;
	static const string PSTIME;
  static const string ROUND;
  static const string READ;
  static const string READFILE;
  static const string READNUM;
  static const string RUN;
  static const string SHOW;
  static const string SIN;
  static const string SQRT;
  static const string SUBSTR;
  static const string TAIL;
  static const string TOUCH;
  static const string TRANSLATE;
  static const string TRYGET;
  static const string WRITE;
  static const string WRITEFILE;
  
  static const string CD;
  static const string CD__;
  static const string COPY;
  static const string ENV;
  static const string EXISTS;
  static const string FIND;
  static const string GREP;
  static const string LS;
  static const string MKDIR;
  static const string MOVE;
  static const string SETENV;
  static const string PWD;
  static const string RM;
  
  static const string ENGLISH;
  static const string GERMAN;
  static const string RUSSIAN;
  static const string SPANISH;
  static const string SYNONYMS;

  static const vector<string> ACTIONS;
  static const vector<string> MATH_ACTIONS;
  static const vector<string> OPER_ACTIONS;
  
  static vector<string> FUNCT_WITH_SPACE;
  static vector<string> FUNCT_WITH_SPACE_ONCE;
  
  static set<string> ELSE_LIST;
  static set<string> ELSE_IF_LIST;
  static set<string> CATCH_LIST;
  
  static set<string> CONTROL_FLOW;
  
  static const unordered_map<string, int> PRIORITY;
  
  static string language(const string& lang);
  
};

#endif /* Constants_h */
