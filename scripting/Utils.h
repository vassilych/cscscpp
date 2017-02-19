//
//  Utils.h
//  scripting
//
//  Created by Vassili Kaplan on 29/03/16.
//  Copyright © 2016 Vassili Kaplan. All rights reserved.
//

#ifndef Utils_h
#define Utils_h

#include "Constants.h"
#include "ParsingScript.h"
#include "Variable.h"

#include <algorithm>
#include <memory>

class  ParsingException : public exception
{
public:
  ParsingException(const string& err) :
  exception(), m_what(err) {}
  ParsingException() throw() {};
  const char* what()  const throw() { return m_what.c_str(); }
  const string& msg() const throw() { return m_what; }
private:
  string m_what;
};

class Utils
{
public:
  
  static string findStartingToken(const string& data, const vector<string>& items);
  static bool startsWith(const string& base, const string& item);
  
  static bool contains(const string& base, const string& item);
  static bool contains(const string& base, char ch);
  
  static string getValidAction(const ParsingScript& script);
  
  static bool moveForwardIf(ParsingScript& script, char expected,
                            char expected2 = Constants::NULL_CHAR);
  
  static bool moveBackIf(ParsingScript& script, char notExpected);
  static string isNotSign(const string& data);
  
  static bool toBool(double value);
  
  static bool isInt(double x);
  
  static void checkArgsNumber(size_t expected, size_t supplied,
                              const string& name);
  static void checkNotEmpty(const string& varName, const string& funcName);
  static void checkNotEnd(const ParsingScript& script, const string& name);
  static void checkNotNull(const string& varName, const void* func);
  static void checkNumber(const Variable& variable);
  static void checkNonNegNumber(const Variable& variable);
  static void checkInteger(const Variable& variable);
  static void checkNonNegInteger(const Variable& variable);
  
  static vector<string> getFunctionSignature(ParsingScript& script);
  
  static vector<size_t> extractArrayIndices(string& varName);
  static vector<Variable> getArrayIndices(string& varName)
  {  size_t end; return getArrayIndices(varName, end); }
  static vector<Variable> getArrayIndices(string& varName, size_t& end);
  static string extractArrayName(ParsingScript& script);
  
  static string getNextToken(ParsingScript& script);
  
  static int goToNextStatement(ParsingScript& script);
  
  static bool endsWithFunction(const string& buffer, const vector<string>& functions);
  static bool spaceNotNeeded(char next);
  static bool keepSpace(const string& script, char next);
  static bool keepSpaceOnce(const string& script, char next);

  static string convertToScript(const string& source,
                                unordered_map<size_t, size_t>& char2Line);
  
  // Checks whether there is an argument separator (e.g.  ',') before the end of the
  // function call. E.g. returns true for "a,b)" and "a(b,c),d)" and false for "b),c".
  static bool separatorExists(const ParsingScript& script);
  
  static string getFileContents(const string& filename);
  static vector<string> getFileLines(const string& filename);
    
  static Variable getItem(ParsingScript& script);
  static string getToken(ParsingScript& script,
                         const string& to = Constants::END_PARSING_STR);
  
  static vector<Variable> getArgs(ParsingScript& script,
                                  char start, char end, bool& isList);
  static string getBodyBetween(ParsingScript& script,
                               char open, char close);
  
  static void skipRestExpr(ParsingScript& script);
  
  static vector<string> tokenize(const string& data,
                                 const string& delimeters = Constants::NEW_LINE,
                                 size_t from = 0, size_t to = string::npos,
                                 bool removeEmpty = false);
  
  static string toUpper(const string& str);
  
  static string toHex(int i);
  
  static void checkSpecialChars(string const& str);
  
  static void replace(string& str, const string& from, const string& to);
  
  static string trim(string const& str);
  
  static string beautifyScript(const string& script, const string& header);

};

#endif /* Utils_hpp */
