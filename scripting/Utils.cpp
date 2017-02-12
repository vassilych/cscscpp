//
//  Utils.cpp
//  scripting
//
//  Created by Vassili Kaplan on 29/03/16.
//  Copyright © 2016 Vassili Kaplan. All rights reserved.
//

#include <algorithm>
#include <cctype>
#include <climits>
#include <codecvt>
#include <cstdio>
#include <ctype.h>
#include <iomanip>
#include <iterator>
#include <locale>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <streambuf>
#include <string>

#include "Parser.h"
#include "Utils.h"
#include "UtilsOS.h"
#include "Variable.h"

string ParsingScript::getOriginalLine(size_t& lineNumber) const
{
  lineNumber = getOriginalLineNumber();
  if (lineNumber == string::npos) {
    return "";
  }
  
  vector<string> lines = Utils::tokenize(m_originalScript);
  if (lineNumber < lines.size()) {
    return lines[lineNumber];
  }
  
  return "";
}

size_t ParsingScript::getOriginalLineNumber() const
{
  unordered_map<size_t, size_t>& char2Line = getChar2Line();
  if (char2Line.empty()) {
    return string::npos;
  }
  
  size_t pos = m_scriptOffset + m_from;
  vector<size_t> lineStart = Utils::getKeys(char2Line);
  size_t lower = 0;
  size_t index = lower;
  
  if (pos <= lineStart[lower]) { // First line.
    return char2Line[lineStart[lower]];
  }
  size_t upper = lineStart.size() - 1;
  if (pos >= lineStart[upper]) { // Last line.
    return char2Line[lineStart[upper]];
  }
  
  while (lower <= upper) {
    index = (lower + upper) / 2;
    size_t guessPos = lineStart[index];
    if (pos == guessPos) {
      break;
    }
    if (pos < guessPos) {
      if (index == 0 || pos > lineStart[index - 1]) {
        break;
      }
      upper = index - 1;
    } else {
      lower = index + 1;
    }
  }
  
  return m_char2Line[lineStart[index]];
}

template <class K, class V>
vector<K> Utils::getKeys(const unordered_map<K, V>& m)
{
  vector<K> keys;
  keys.reserve(m.size());
  for(auto kv : m) {
    keys.push_back(kv.first);
  }
  std::sort(keys.begin(), keys.end());
  
  return keys;
}

Variable Utils::getItem(ParsingScript& script)
{
  moveForwardIf(script, Constants::NEXT_ARG, Constants::SPACE);
  Utils::checkNotEnd(script, "Incomplete function definition");
  
  Variable value;
  
  if (script.current() == Constants::START_GROUP) {
    // We are extracting a list between curly braces.
    script.forward(); // Skip first brace.
    bool isList = true;
    value.set(getArgs(script, Constants::START_GROUP,
                      Constants::END_GROUP, isList));
    
    return value;
  } else {
    // A variable, a function, or a number.
    string parsing3 = script.rest();
    value = Parser::loadAndCalculate(script, Constants::NEXT_OR_END);
  }
  
  string parsing4 = script.rest();
  //moveForwardIf(script, Constants::END_ARG, Constants::SPACE);
  moveForwardIf(script, Constants::SPACE);
  return value;
}

string Utils::getToken(ParsingScript& script, const string& to)
{
  char curr = script.tryCurrent();
  char prev = script.tryPrev();
  
  if (to.find_first_of(Constants::SPACE) == string::npos) {
    // Skip a leading space unless we are inside of quotes
    while (curr == Constants::SPACE && prev != Constants::QUOTE) {
      script.forward();
      curr = script.tryCurrent();
      prev = script.tryPrev();
    }
  }
  
  // String in quotes
  bool inQuotes = curr == Constants::QUOTE;
  if (inQuotes) {
    size_t end = script.find(Constants::QUOTE, script.getPointer() + 1);
    if (end == string::npos) {
      throw ParsingException("Unmatched quotes in [" +
                             script.rest() + "]");
    }
    string result = script.substr(script.getPointer() + 1, end - script.getPointer() - 1);
    script.setPointer(end + 1);
    return result;
  }
  
  // Variable defined as $foobar
  bool isVariable = curr == Constants::VAR_START;
  if (isVariable) {
    script.forward(); // skip the starting '$' sign
    return getItem(script).toString();
  }
  
  // Nothing, empty.
  size_t end = script.find_first_of(to, script.getPointer());
  if (end == string::npos || script.getPointer() >= end) {
    script.forward();
    return Constants::EMPTY;
  }
  
  // Skip found characters that have a backslash before.
  while ((end > 0 && script(end - 1) == '\\') &&
         end + 1 < script.size()) {
    end = script.find_first_of(to, end + 1);
  }
  
  if (end == string::npos) {
    throw ParsingException("Couldn't extract token from [" +
                           script.rest() + "]");
  }
  
  if (script(end - 1) == Constants::QUOTE) {
    end--;
  }
  
  string var = script.substr(script.getPointer(), end - script.getPointer());
  // \"yes\" --> "yes"
  script.setPointer(end);
  
  moveForwardIf(script, Constants::QUOTE, Constants::SPACE);
  
  return var;
}

vector<Variable> Utils::getArgs(ParsingScript& script,
                                char start, char end, bool& isList)
{
  vector<Variable> args;
  
  isList = script.tryCurrent() == Constants::START_GROUP;
  
  if (!script.stillValid() || script.current() == Constants::END_STATEMENT) {
    return args;
  }
  
  ParsingScript tempScript(script.getData(), script.getPointer());
  string body = Utils::getBodyBetween(tempScript, start, end);
  string arest1 = script.rest();
  
  while (script.getPointer() < tempScript.getPointer()) {
    string rest1 = script.rest();
    Variable item = Utils::getItem(script);
    if (item.type == Constants::NONE) {
      break;
    }
    
    args.push_back(item);
    string arest2 = script.rest();
    arest2 = script.rest();
  }
  
  string rest22 = script.rest();
  moveForwardIf(script, Constants::SPACE);
  return args;
}

string Utils::getBodyBetween(ParsingScript& script, char open, char close)
{
  // We are supposed to be one char after the beginning of the string, i.e.
  // we must not have the opening char as the first one.
  string result;
  int braces = 0;
  
  for (; script.stillValid(); script.forward())
  {
    char ch = script.current();
    
    if (ch == ' ' && result.empty()) {
      continue;
    } else if (ch == open) {
      braces++;
    } else if (ch == close) {
      braces--;
    }
    
    result += ch;
    if (braces == -1)
    {
      if (ch == close) {
        result.erase(result.size() - 1, 1);
      }
      break;
    }
  }
  
  return result;
}

string Utils::findStartingToken(const string& data, const vector<string>& items)
{
  for (size_t i = 0; i < items.size(); i++)  {
    if (Utils::startsWith(data, items[i])) {
      return items[i];
    }
  }
  
  return Constants::EMPTY;
}

bool Utils::startsWith(const string& base, const string& item)
{
  size_t n = item.size();
  if (n > base.size()) {
    return false;
  }
  
  int comp = strncmp(base.c_str(), item.c_str(), n);
  
  return comp == 0;
}

string Utils::isNotSign(const string& data)
{
  return startsWith(data, Constants::NOT) ? Constants::NOT : Constants::EMPTY;
}


bool Utils::contains(const string& base, const string& item)
{
  return base.find(item) != string::npos;
}

bool Utils::contains(const string& base, char ch)
{
  return base.find(ch) != string::npos;
}

string Utils::getValidAction(const ParsingScript& script)
{
  if (!script.stillValid()) {
    return Constants::EMPTY;
  }
  
  string base = script.rest(string::npos);
  string action = findStartingToken(base, Constants::ACTIONS);
  return action;
}

bool Utils::moveForwardIf(ParsingScript& script, char expected,
                          char expected2)
{
  if (script.tryCurrent() == expected ||
      script.tryCurrent() == expected2) {
    script.forward();
    return true;
  }
  return false;
}

bool Utils::moveBackIf(ParsingScript& script, char notExpected)
{
  if (script.tryCurrent() == notExpected) {
    script.backward();
    return true;
  }
  return false;
}

bool Utils::toBool(double value)
{
  return value != 0;
}

bool Utils::isInt(double x)
{
  return x < LLONG_MAX && x > LLONG_MIN && (x == floor(x));
}

void Utils::checkArgsNumber(size_t expected, size_t supplied,
                            const string& name)
{
  if (expected != supplied) {
    throw ParsingException("Function [" + name + "] arguments mismatch: " +
                           to_string(expected) + " expected, " +
                           to_string(supplied) + " supplied");
  }
}

void Utils::checkNotEnd(const ParsingScript& script, const string& name)
{
  if (!script.stillValid()) {
    throw ParsingException("Incomplete arguments for [" + name + "]");
  }
}

void Utils::checkNotEmpty(const string& varName, const string& funcName)
{
  if (varName.empty()) {
    throw ParsingException("Incomplete arguments for [" + funcName + "]");
  }
}

void Utils::checkNotNull(const string& varName, const void* func)
{
  if (func == 0) {
    throw ParsingException("Variable [" + varName + "] doesn't exist");
  }
}

void Utils::checkInteger(const Variable& variable)
{
  if (variable.type != Constants::NUMBER ||
      variable.numValue - floor(variable.numValue) != 0.0) {
    throw ParsingException("Expecting an integer instead of [" +
                           variable.toString() + "]");
  }
}
void Utils::checkNonNegInteger(const Variable& variable)
{
  checkInteger(variable);
  if (variable.numValue < 0) {
    throw ParsingException("Expecting a non negative number instead of [" +
                           variable.toString() + "]");
  }
}

void Utils::checkNumber(const Variable& variable)
{
  if (variable.type != Constants::NUMBER) {
    throw ParsingException("Expecting a number instead of [" +
                           variable.toString() + "]");
  }
}
void Utils::checkNonNegNumber(const Variable& variable)
{
  checkNumber(variable);
  if (variable.numValue <  0) {
    throw ParsingException("Expecting a non negative number instead of [" +
                           variable.toString() + "]");
  }
}

vector<string> Utils::getFunctionSignature(ParsingScript& script)
{
  moveForwardIf(script, Constants::START_ARG, Constants::SPACE);
  
  size_t endArgs = script.find(Constants::END_ARG, script.getPointer());
  if (endArgs == string::npos) {
    throw ParsingException("Couldn't extract function signature");
  }
  
  vector<string> args = tokenize(script.getData(), Constants::FUNC_SIG_SEP,
                                 script.getPointer(), endArgs);
  
  script.setPointer(endArgs + 1);
  
  return args;
}

vector<string> Utils::tokenize(const string& data,
                               const string& delimeters,
                               size_t from, size_t to,
                               bool removeEmpty)
{
  vector<string> args;
  size_t next = from;
  
  while (next != string::npos && next < to) {
    next = data.find_first_of(delimeters, from);
    if (next == string::npos ) {
      break;
    }
    
    string arg = data.substr(from, next - from);
    from = next + 1;
    
    if (removeEmpty) {
      arg = Utils::trim(arg);
      if (arg.empty()) {
        continue;
      }
    }
    
    args.push_back(arg);
  }
  
  if (from < min(data.size(), to)) {
    string arg = data.substr(from);
    args.push_back(arg);
  }
  
  return args;
}

string Utils::getNextToken(ParsingScript& script)
{
  if (!script.stillValid()) {
    return "";
  }
  
  size_t end = script.find_first_of(Constants::TOKEN_SEPARATION,
                                    script.getPointer());
  
  if (end == string::npos) {
    return "";
  }
  
  string var = script.substr(script.getPointer(), end - script.getPointer());
  script.setPointer(end);
  return var;
}

vector<Variable> Utils::getArrayIndices(string& varName, size_t& end)
{
  vector<Variable> indices;
  size_t argStart = varName.find(Constants::START_ARRAY);
  size_t firstIndexStart = argStart;
  
  while (argStart < varName.size() &&
         varName[argStart] == Constants::START_ARRAY)  {
    size_t argEnd = varName.find(Constants::END_ARRAY, argStart + 1);
    if (argEnd == string::npos || argEnd <= argStart + 1) {
      break;
    }
    
    ParsingScript tempScript(varName, argStart);
    Utils::moveForwardIf(tempScript,
                         Constants::START_ARG, Constants::START_ARRAY);
    
    string parsing4 = tempScript.rest();
    Variable index = Parser::loadAndCalculate(tempScript,
                                              string(1, Constants::END_ARRAY));
    indices.push_back(index);
    argStart = argEnd + 1;
  }
  
  if (!indices.empty()) {
    varName = varName.substr(0, firstIndexStart);
    end = argStart - 1;
  }
  
  return indices;
}

vector<size_t> Utils::extractArrayIndices(string& varName)
{
  vector<size_t> indices;
  size_t argStart = varName.find(Constants::START_ARRAY);
  size_t firstIndexStart = argStart;
  
  while (argStart != string::npos &&
         varName[argStart] == Constants::START_ARRAY)
  {
    size_t argEnd = varName.find(Constants::END_ARRAY, argStart + 1);
    if (argEnd == string::npos || argEnd <= argStart + 1) {
      break;
    }
    
    ParsingScript tempScript(varName, argStart);
    Utils::moveForwardIf(tempScript,
                         Constants::START_ARG, Constants::START_ARRAY);
    
    Variable index = Parser::loadAndCalculate(tempScript,
                                              string(1, Constants::END_ARRAY));
    
    Utils::checkNonNegInteger(index);
    if (index.type == Constants::NUMBER &&
        index.numValue >= 0) {
      indices.push_back((size_t)index.numValue);
    }
    argStart = argEnd + 1;
  }
  
  if (!indices.empty() && firstIndexStart > 0) {
    varName = varName.substr(0, firstIndexStart);
  }
  
  return indices;
}

string Utils::extractArrayName(ParsingScript& script)
{
  size_t index = script.find_first_of(Constants::START_ARRAY_OR_END, script.getPointer());
  if (index == string::npos || index == 0 ||
      script(index) != Constants::START_ARRAY) {
    return "";
  }
  
  string arrayName = script.substr(script.getPointer(), index - script.getPointer());
  script.setPointer(index + 1);
  
  return arrayName;
}

bool Utils::separatorExists(const ParsingScript& script)
{
  if (!script.stillValid()) {
    return false;
  }
  
  int argumentList = 0;
  for (size_t i = script.getPointer(); i < script.size(); i++)
  {
    char ch = script(i);
    switch (ch)
    {
      case Constants::NEXT_ARG:
        return true;
      case Constants::START_ARG:
        argumentList++;
        break;
      case Constants::END_STATEMENT:
      case Constants::END_GROUP:
      case Constants::END_ARG:
        if (--argumentList < 0)
        {
          return false;
        }
        break;
    }
  }
  
  return false;
}

int Utils::goToNextStatement(ParsingScript& script)
{
  int endGroupRead = 0;
  while (script.stillValid()) {
    char currentChar = script.current();
    switch (currentChar)
    {
      case Constants::END_GROUP: endGroupRead++;
        script.forward();
        return endGroupRead;
      case Constants::START_GROUP:
      case Constants::QUOTE:
      case Constants::SPACE:
      case Constants::END_STATEMENT:
      case Constants::END_ARG:
        script.forward();
        break;
      default:
        return endGroupRead;
    }
  }
  return endGroupRead;
}

void Utils::skipRestExpr(ParsingScript& script)
{
  int argRead = 0;
  bool inQuotes = false;
  char previous = Constants::NULL_CHAR;
  
  while (script.stillValid()) {
    char currentChar = script.current();
    if (inQuotes && currentChar != Constants::QUOTE) {
      script.forward();
      continue;
    }
    
    switch (currentChar)
    {
      case Constants::QUOTE:
        if (previous != '\\') {
          inQuotes = !inQuotes;
        }
        break;
      case Constants::START_ARG:
        argRead++;
        break;
      case Constants::END_ARG:
        argRead--;
        if (argRead < 0) {
          return;
        }
        break;
      case Constants::END_STATEMENT:
      case Constants::NEXT_ARG:
        return;
      default:
        break;
    }
    
    script.forward();
    previous = currentChar;
  }
}

bool Utils::endsWithFunction(const string& buffer, const vector<string>& functions)
{
  size_t bufSize = buffer.size();
  for (size_t i = 0; i < functions.size(); i++) {
    const string& func = functions[i];
    size_t funcSize = func.size();
    size_t from  = bufSize - funcSize;
    
    if (bufSize >= funcSize && buffer.compare(from, funcSize, func) == 0) {
      char prev = funcSize >= bufSize ?
      Constants::END_STATEMENT :
      buffer[bufSize - funcSize - 1];
      if (Utils::contains(Constants::TOKEN_SEPARATION, prev)) {
        return true;
      }
      
    }
  }
  
  return false;
}

bool Utils::spaceNotNeeded(char next)
{
  return (next == Constants::SPACE || next == Constants::START_ARG ||
          next == Constants::START_GROUP ||
          next == Constants::NULL_CHAR);
}

bool Utils::keepSpace(const string& script, char next)
{
  if (spaceNotNeeded(next)) {
    return false;
  }
  
  return endsWithFunction(script, Constants::FUNCT_WITH_SPACE);
}
bool Utils::keepSpaceOnce(const string& script, char next)
{
  if (spaceNotNeeded(next)) {
    return false;
  }
  
  return endsWithFunction(script, Constants::FUNCT_WITH_SPACE_ONCE);
}

string Utils::toHex(int i)
{
  stringstream stream;
  stream << ((i<16)?"0":"") << ((i<256)?"0":"") << ((i<4096)?"0":"");
  stream << hex << i;
  return "0x" + toUpper(stream.str());
}

void Utils::checkSpecialChars(string const& str)
{
  wstring wstr = OS::s2w(str);
  int pos = 0;
  
  for (wchar_t u : wstr) {
    pos++;
    if (u >= 127 && u <= 187) {
      wstring part = wstr.substr(pos - 1, Constants::MAX_CHARS_TO_SHOW);
      string excerpt = OS::w2s(part);
      
      throw ParsingException("Illegal character [" + string(1, u) +
                             " (" + to_string(u) + "," + toHex(u) + ")" +
                             "] at position " + to_string(pos) +
                             ": " + excerpt +
                             "...");
    }
  }
}

string Utils::convertToScript(const string& source, unordered_map<size_t, size_t>& char2Line)
{
  string result;
  
  bool inQuotes       = false;
  bool spaceOK        = false;
  bool inComments     = false;
  bool simpleComments = false;
  char previous       = Constants::NULL_CHAR;
  
  int parentheses = 0;
  int groups = 0;
  size_t lineNumber = 0;
  size_t lastScriptLength = 0;
  
  string toCheck;
  
  for (int i = 0; i < source.size(); i++)
  {
    char ch = source[i];
    char next = i + 1 < source.size() ? source[i + 1] : Constants::NULL_CHAR;
    
    if (ch == '\n') {
      if (result.size() > lastScriptLength) {
        char2Line[result.size() - 1] = lineNumber;
        lastScriptLength = result.size();
      }
      lineNumber++;
    }

    if (inComments && ((simpleComments && ch != '\n') ||
                      (!simpleComments && ch != '*'))) {
      continue;
    }
    
    switch (ch)
    {
      case '/':
        if (inComments || next == '/' || next == '*') {
          inComments = true;
          simpleComments = simpleComments || next == '/';
          continue;
        }
        break;
      case Constants::QUOTE:
        if (!inComments && previous != '\\') {
          inQuotes = !inQuotes;
        }
        break;
      case '\r':
      case '\t':
      case ' ':
        if (inQuotes) {
          break; // the char will be added
        }
        else {
          bool keep = keepSpace(result, next);
          spaceOK = keep || (previous != Constants::NULL_CHAR &&
                             previous != Constants::NEXT_ARG && spaceOK);
          bool spaceOKonce = keepSpaceOnce(result, next);
          if (spaceOK || spaceOKonce) {
            break; // the char will be added
          }
        }
        continue;
      case '\n':
        if (simpleComments) {
          inComments = simpleComments = false;
        }
        spaceOK    = false;
        continue;
      case Constants::END_ARG:
        if (!inQuotes) {
          parentheses--;
          if (parentheses < 0) {
            //throw ParsingException("Uneven parentheses " +
            //      string(1, Constants::END_ARG) +
            //      " at " + source.substr(i, Constants::MAX_CHARS_TO_SHOW));
            
          }
          spaceOK = false;
        }
        break;
      case Constants::START_ARG:
        if (!inQuotes) {
          parentheses++;
        }
        break;
      case Constants::END_GROUP:
        if (!inQuotes) {
          groups--;
          spaceOK = false;
        }
        break;
      case Constants::START_GROUP:
        if (!inQuotes) {
          groups++;
        }
        break;
      case Constants::END_STATEMENT:
        if (!inQuotes) {
          spaceOK = false;
        }
        break;
      default: break;
    }
    
    if (!inComments) {
      result += ch;
      if (inQuotes && !toCheck.empty()) {
        checkSpecialChars(toCheck);
        toCheck.clear();
      } else if (!inQuotes) {
        // We don't check whatever is in quotes
        toCheck += ch;
      }
    }
    previous = ch;
  }
  
  /*if (parentheses != 0) {
    throw ParsingException("Uneven parentheses " + string(1, Constants::START_ARG) +
                           string(1, Constants::END_ARG));
  }
  if (groups != 0) {
    throw ParsingException("Uneven groups " + string(1, Constants::START_GROUP) +
                           string(1, Constants::END_GROUP));
  }*/
  
  if (inQuotes && !toCheck.empty()) {
    checkSpecialChars(toCheck);
  }
  return result;
}

wstring readUnicodeFile(const char* filename)
{
  wifstream wifs;
  
  ifstream wif(filename, ios::binary);
  stringstream wss;
  wss << wif.rdbuf();
  string  const &str = wss.str();
  wstring wstr;
  wstr.resize(str.size() / sizeof(wchar_t));
  memcpy(&wstr[0], str.c_str(), str.size()); // copy data into wstring
  return wstr;
}
string readFile(const string& filename)
{
  wifstream wifs;
  wstring txtline;
  wstring wresult;
  
  wifs.open(filename.c_str());
  if (!wifs.is_open())
  {
    string pwd = OS::pwd();
    throw ParsingException("Could not open " + filename);
  }
  // We are going to read an UTF-8 file
  wifs.imbue(locale(wifs.getloc(), new codecvt_utf8<wchar_t, 0x10ffff, consume_header>()));
  while (getline(wifs, txtline)) {
    wresult += txtline + L"\n";
  }
  
  wstring_convert<codecvt_utf8<wchar_t>, wchar_t> converter;
  string converted_str = converter.to_bytes(wresult);
  wstring ba = converter.from_bytes(converted_str);
  
  string res2 = OS::w2s(wresult.c_str());
  wstring wres2 = OS::s2w(res2);
  
  return converted_str;
}

string Utils::getFileContents(const string& filename)
{
#ifdef _WIN32
  string result = readFile(filename);
#else
  ifstream file(filename.c_str(), ios::in | ios::binary);
  if (!file) {
    string pwd = OS::pwd();
    throw ParsingException("Couldn't open [" + filename + "] in " + pwd);
  }
  
  string result = string((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
#endif
  return result;
}

vector<string> Utils::getFileLines(const string& filename)
{
  string contents = Utils::getFileContents(filename);
  vector<string> lines = Utils::tokenize(contents);
  return lines;
}

string Utils::beautifyScript(const string& script, const string& header)
{
  string result = header;
  static string extraSpace = "<>=&|+-*/%";
  
  int indent = Constants::INDENT;
  
  bool inQuotes = false;
  bool lineStart = true;
  
  for (int i = 0; i < script.size(); i++) {
    char ch = script[i];
    inQuotes = ch == Constants::QUOTE ? !inQuotes : inQuotes;
    
    if (inQuotes) {
      result += ch;
      continue;
    }
    
    bool needExtra = Utils::contains(extraSpace, ch) && i > 0 && i < script.size() - 1;
    if (needExtra && !Utils::contains(extraSpace, script[i - 1])) {
      result += " ";
    }
    
    switch(ch) {
      case Constants::START_GROUP:
        result += string(1, ' ') + Constants::START_GROUP + Constants::NEW_LINE;
        indent += Constants::INDENT;
        lineStart = true;
        break;
      case Constants::END_GROUP:
        indent -= Constants::INDENT;
        result += string(indent, ' ') + Constants::END_GROUP+ Constants::NEW_LINE;
        lineStart = true;
        break;
      case Constants::END_STATEMENT:
        result += ch + Constants::NEW_LINE;
        lineStart = true;
        break;
      default:
        if (lineStart) {
          result += string(indent, ' ');
          lineStart = false;
        }
        result += ch;
        break;
    }
    if (needExtra && !Utils::contains(extraSpace, script[i + 1])) {
      result += " ";
    }
  }
  
  result += Constants::END_GROUP + Constants::NEW_LINE;
  return result;
}

string Utils::toUpper(const string& str)
{
  string upper;
  transform(str.begin(), str.end(), std::back_inserter(upper), ::toupper);
  return upper;
}

void Utils::replace(string& str, const string& from, const string& to)
{
  size_t pos = 0;
  while ((pos = str.find(from, pos)) != string::npos) {
    str.replace(pos, from.length(), to);
    pos += to.length();
  }
}

string Utils::trim(string const& str)
{
  size_t first = str.find_first_not_of(Constants::WHITES);
  
  if (first == string::npos) {
    return "";
  }
  
  size_t last = str.find_last_not_of(Constants::WHITES);
  
  return str.substr(first, last - first + 1);
}
