//
//  Functions.cpp
//  scripting
//
//  Created by Vassili Kaplan on 30/03/16.
//  Copyright © 2016 Vassili Kaplan. All rights reserved.
//

#include <algorithm>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <stdio.h>
#include <thread> 

#include "Functions.h"
#include "Interpreter.h"
#include "Parser.h"
#include "Translation.h"
#include "Utils.h"
#include "UtilsOS.h"

bool SignalWaitFunction::g_signaled = false;
mutex SignalWaitFunction::g_mutex;
mutex LockFunction::g_mutex;
condition_variable SignalWaitFunction::g_cv;
unordered_map<string, thread*> ThreadFunction::g_threads;

//-------------------------------------------
Variable StringOrNumberFunction::evaluate(ParsingScript& script)
{
  // First check if the passed expression is a string between quotes.
  if (m_item.size() > 1 &&
      m_item[0] == Constants::QUOTE &&
      m_item[m_item.size() - 1] == Constants::QUOTE) {
    
    string result = m_item.substr(1, m_item.size() - 2);
    return Variable(result);
  }
  
  // Otherwise this should be a number.
  char* x;
  double num = ::strtod(m_item.c_str(), &x);
  if (::strlen(x) > 0) {
    Translation::throwException(script, "parseToken", m_item, "parseTokenExtra");

    /*string msg = Translation::getErrorString("parseToken");
    Utils::replace(msg, "{0}", m_item);

    string candidate = Translation::tryFindError(m_item, script);
    if (!candidate.empty()) {
      //msg += " Did you mean using keyword [" + candidate + "] ?";
      string extra = Translation::getErrorString("parseTokenExtra");
      Utils::replace(extra, "{0}", candidate);
      msg += Constants::SPACE + extra;
    }
    
    throw ParsingException("Couldn't parse token [" + m_item + "]");*/
  }
  
  return Variable(num);
}

//-------------------------------------------
Variable IdentityFunction::evaluate(ParsingScript& script)
{
  return Parser::loadAndCalculate(script, Constants::END_ARG_STR);
}

//-------------------------------------------
Variable AllFunctions::evaluate(ParsingScript& script)
{
  ParserFunction::allFunctions();
  return Variable::emptyInstance;
}

//-------------------------------------------
Variable AllVariables::evaluate(ParsingScript& script)
{
  ParserFunction::allVariables();
  return Variable::emptyInstance;
}

//-------------------------------------------
Variable AbsFunction::evaluate(ParsingScript& script)
{
  Variable arg = Parser::loadAndCalculate(script, Constants::END_ARG_STR);
  Utils::checkNumber(arg);
  return std::abs(arg.numValue);
}
//-------------------------------------------
Variable SinFunction::evaluate(ParsingScript& script)
{
  Variable arg = Parser::loadAndCalculate(script, Constants::END_ARG_STR);
  Utils::checkNumber(arg);
  return ::sin(arg.numValue);
}
//-------------------------------------------
Variable CeilFunction::evaluate(ParsingScript& script)
{
  Variable arg = Parser::loadAndCalculate(script, Constants::END_ARG_STR);
  Utils::checkNumber(arg);
  return ::ceil(arg.numValue);
}
//-------------------------------------------
Variable CosFunction::evaluate(ParsingScript& script)
{
  Variable arg = Parser::loadAndCalculate(script, Constants::END_ARG_STR);
  Utils::checkNumber(arg);
  return ::cos(arg.numValue);
}
//-------------------------------------------
Variable ExpFunction::evaluate(ParsingScript& script)
{
  Variable arg = Parser::loadAndCalculate(script, Constants::END_ARG_STR);
  Utils::checkNumber(arg);
  return ::exp(arg.numValue);
}
//-------------------------------------------
Variable FloorFunction::evaluate(ParsingScript& script)
{
  Variable arg = Parser::loadAndCalculate(script, Constants::END_ARG_STR);
  Utils::checkNumber(arg);
  return ::floor(arg.numValue);
}
//-------------------------------------------
Variable IndexOfFunction::evaluate(ParsingScript& script)
{
  // 1. Get the name of the variable.
  string varName = Utils::getToken(script, Constants::NEXT_OR_END);
  Utils::checkNotEnd(script, Constants::INDEX_OF);
  
  // 2. Get the current value of the variable.
  ParserFunction* func = ParserFunction::getFunction(varName);
  Utils::checkNotNull(varName, func);
  Variable currentValue = func->getValue(script);
  
  // 3. Get the value to be looked for.
  Variable searchValue = Utils::getItem(script);
  
  // 4. Take either the string part if it is defined,
  // or the numerical part converted to a string otherwise.
  string basePart = currentValue.toString();
  string search = searchValue.toString();
  
  size_t index = basePart.find(search);
  int result = index == string::npos ? -1 : (int)index;
  return Variable(result);
}
//-------------------------------------------
Variable LogFunction::evaluate(ParsingScript& script)
{
  Variable arg = Parser::loadAndCalculate(script, Constants::END_ARG_STR);
  Utils::checkNumber(arg);
  return ::log(arg.numValue);
}

//-------------------------------------------
Variable PiFunction::evaluate(ParsingScript& script)
{
  return 3.141592653589793;
}

//-------------------------------------------
Variable PowFunction::evaluate(ParsingScript& script)
{
  Variable arg1 = Parser::loadAndCalculate(script, Constants::FUNC_SIG_SEP);
  Utils::checkNumber(arg1);
  script.forward(); // eat separation
  Variable arg2 = Parser::loadAndCalculate(script, Constants::FUNC_SIG_SEP);
  Utils::checkNumber(arg2);
  
  arg1.numValue = ::pow(arg1.numValue, arg2.numValue);
  return arg1;
}
//-------------------------------------------
Variable RoundFunction::evaluate(ParsingScript& script)
{
  Variable arg = Parser::loadAndCalculate(script, Constants::END_ARG_STR);
  //Utils::checkNumber(arg);
  return ::floor(arg.numValue + 0.5);
}
//-------------------------------------------
Variable SqrtFunction::evaluate(ParsingScript& script)
{
  Variable arg = Parser::loadAndCalculate(script, Constants::END_ARG_STR);
  Utils::checkNumber(arg);
  return ::sqrt(arg.numValue);
}
//-------------------------------------------
Variable SubstrFunction::evaluate(ParsingScript& script)
{
  string substring;
  
  // 1. Get the name of the variable.
  string varName = Utils::getToken(script, Constants::NEXT_OR_END);
  Utils::checkNotEnd(script, Constants::SUBSTR);
  
  // 2. Get the current value of the variable.
  ParserFunction* func = ParserFunction::getFunction(varName);
  Utils::checkNotNull(varName, func);
  Variable currentValue = func->getValue(script);
  
  // 3. Take either the string part if it is defined,
  // or the numerical part converted to a string otherwise.
  string arg = currentValue.toString();
  
  // 4. Get the initial index of the substring.
  Variable init = Utils::getItem(script);
  Utils::checkNonNegInteger(init);
  
  // 5. Get the length of the substring if available.
  bool lengthAvailable = Utils::separatorExists(script);
  if (lengthAvailable) {
    Variable length = Utils::getItem(script);
    Utils::checkNonNegInteger(length);
    if (init.numValue + length.numValue > arg.size()) {
      throw ParsingException("The substring length is larger than [" +
                             arg + "]");
    }
    substring = arg.substr((size_t)init.numValue, (size_t)length.numValue);
  }
  else {
    substring = arg.substr((size_t)init.numValue);
  }
  
  return Variable(substring);
}

//-------------------------------------------
Variable AddFunction::evaluate(ParsingScript& script)
{
  // 1. Get the name of the variable.
  string varName = Utils::getToken(script, Constants::NEXT_OR_END);
  Utils::checkNotEnd(script, Constants::ADD);
  
  // 2. Get the current value of the variable.
  bool isGlobal = true;
  ParserFunction* func = ParserFunction::getFunction(varName, isGlobal);
  Utils::checkNotNull(varName, func);
  Variable currentValue = func->getValue(script);
  
  // 3. Get the variable to add.
  Variable item = Utils::getItem(script);
  
  // 4. Add it to the tuple.
  currentValue.type = Constants::ARRAY;
  currentValue.tuple.emplace_back(item);
  
  ParserFunction::addGlobalOrLocalVariable(varName,
                                           new GetVarFunction(currentValue), isGlobal);
  
  return currentValue;
}
//-------------------------------------------
Variable SizeFunction::evaluate(ParsingScript& script)
{
  // 1. Get the name of the variable.
  string varName = Utils::getToken(script, Constants::END_ARG_STR);
  Utils::checkNotEnd(script, Constants::SIZE);
  
  vector<Variable> arrayIndices = Utils::getArrayIndices(varName);
  
  // 2. Get the current value of the variable.
  ParserFunction* func = ParserFunction::getFunction(varName);
  Utils::checkNotNull(varName, func);
  Variable currentValue = func->getValue(script);
  Variable element = currentValue;
  
  // 2b. Special case for an array.
  if (!arrayIndices.empty()) {// array element
    element = *GetVarFunction::extractArrayElement(&currentValue, arrayIndices);
    Utils::moveForwardIf(script, Constants::END_ARRAY);
  }
  
  // 3. Take either the length of the underlying tuple or
  // string part if it is defined,
  // or the numerical part converted to a string otherwise.
  size_t size = element.type == Constants::ARRAY ?
                     element.tuple.size() :
                     element.toString().size();
  
  Utils::moveForwardIf(script, Constants::END_ARG, Constants::SPACE);
  
  return Variable(size);
}

//-------------------------------------------
Variable PrintFunction::evaluate(ParsingScript& script)
{
  bool isList = false;
  vector<Variable> args = Utils::getArgs(script,
                    Constants::START_ARG, Constants::END_ARG, isList);
  
  for (size_t i = 0; i < args.size(); i++) {
    if (m_color == OS::Color::NONE) {
      OS::print(args[i].toString());
    } else {
      OS::printColor(m_color, args[i].toString());
    }
  }
  
  if (m_newLine) {
    OS::print("", true);
  }
  return Variable::emptyInstance;
}

//-------------------------------------------
Variable ProcessorTimeFunction::evaluate(ParsingScript& script)
{
  double time = 1000.0 * OS::getCpuTime();
  return Variable(time);
}

//-------------------------------------------
Variable ThrowFunction::evaluate(ParsingScript& script)
{
  // 1. Extract what to throw.
  Variable arg = Utils::getItem(script);
  
  // 2. Convert it to string.
  string result = arg.toString();
  
  // 3. Throw it!
  throw ParsingException(result);
}

//-------------------------------------------
Variable ReturnStatement::evaluate(ParsingScript& script)
{
  Utils::moveForwardIf(script, Constants::SPACE);
  
  Variable result = Utils::getItem(script);
  
  // If we are in Return, we are done:
  script.setPointer(script.size());
  result.isReturn = true;
  
  return result;
}

//-------------------------------------------
Variable ContinueStatement::evaluate(ParsingScript& script)
{
  return Variable(Constants::CONTINUE_STATEMENT);
}
//-------------------------------------------
Variable BreakStatement::evaluate(ParsingScript& script)
{
  return Variable(Constants::BREAK_STATEMENT);
}
//-------------------------------------------
Variable ExitStatement::evaluate(ParsingScript& script)
{
  ::exit(0);
  return Variable::emptyInstance;
}

//-------------------------------------------
Variable IfStatement::evaluate(ParsingScript& script)
{
  return Interpreter::processIf(script);
}
//-------------------------------------------
Variable TryStatement::evaluate(ParsingScript& script)
{
  return Interpreter::processTry(script);
}
//-------------------------------------------
Variable ForStatement::evaluate(ParsingScript& script)
{
  return Interpreter::processFor(script);
}

//-------------------------------------------
Variable WhileStatement::evaluate(ParsingScript& script)
{
  return Interpreter::processWhile(script);
}

//-------------------------------------------
Variable GetVarFunction::evaluate(ParsingScript& script)
{
  // First check if this element is part of an array:
  bool start1Before = script.tryPrev() == Constants::START_ARRAY;
  if (start1Before) {
    if (m_arrayIndices.empty()) {
      string startName = script.substr(script.getPointer() - 1);
      m_arrayIndices = Utils::getArrayIndices(startName, m_delta);
    }
    script.forward(m_delta);
    
    Variable* result = extractArrayElement(&m_value, m_arrayIndices);
    return *result;
  }
  
  // Otherwise just return the stored value.
  return m_value;
}

//-------------------------------------------
Variable* GetVarFunction::extractArrayElement(Variable* array,
                                              const vector<Variable>& indices)
{
  Variable* currLevel = array;
  
  for (size_t i = 0; i < indices.size(); i++) {
    const Variable& index = indices[i];
    size_t arrayIndex = currLevel->getArrayIndex(index);
    
    if (arrayIndex >= currLevel->tuple.size()) {
      throw ParsingException("Unknown index [" + index.toString() +
                             "] for tuple of size " +
                             to_string(currLevel->tuple.size()));
    }
    currLevel = &(currLevel->tuple[arrayIndex]);
  }
  return currLevel;
}

//-------------------------------------------
Variable FunctionCreator::evaluate(ParsingScript& script)
{
  string funcName = Utils::getToken(script, Constants::TOKEN_SEPARATION);
  //cout << "Registering function [" + funcName + "] ..." << endl;
  
  vector<string> args = Utils::getFunctionSignature(script);
  
  Utils::moveForwardIf(script, Constants::START_GROUP, Constants::SPACE);
  size_t parentOffset = script.getPointer();
  
  string body = Utils::getBodyBetween(script,
                                      Constants::START_GROUP, Constants::END_GROUP);
  
  CustomFunction* customFunc = new CustomFunction(funcName, body, args,                                                 script, parentOffset);
  ParserFunction::addGlobalFunction(funcName, customFunc, false);
  
  return Variable(funcName);
}

//-------------------------------------------
string CustomFunction::getHeader()
{
  string result = Constants::FUNCTION + " " + getName() + " " + Constants::START_ARG;
  
  for (size_t i = 0; i < m_args.size(); i++) {
    result += m_args[i];
    if (i != m_args.size() - 1) {
      result += string(1, ' ') + Constants::NEXT_ARG;
    }
  }
  
  result += string(1, Constants::END_ARG) + " " + Constants::START_GROUP + Constants::NEW_LINE;

  return result;
}

//-------------------------------------------
Variable CustomFunction::evaluate(ParsingScript& script)
{
  bool isList(false);
  vector<Variable> args = Utils::getArgs(script,
                                         Constants::START_ARG, Constants::END_ARG, isList);
  
  Utils::moveBackIf(script, Constants::START_GROUP);
  
  Utils::checkArgsNumber(m_args.size(), args.size(), m_name);
  
  // 1. Add passed arguments as local variables to the Parser.
  StackLevel stackLevel(m_name);
  
  for (size_t i = 0; i < m_args.size(); i++) {
    stackLevel.variables[m_args[i]] = new GetVarFunction(args[i]);
  }
  
  ParserFunction::addLocalVariables(stackLevel);
  
  // 2. Execute the body of the function.
  Variable result;
  ParsingScript funcScript(m_body);
  funcScript.setOffset(m_parentOffset);
  funcScript.setChar2Line(m_parentScript.getChar2Line());
  funcScript.setFilename(m_parentScript.getFilename());
  funcScript.setOriginalScript(m_parentScript.getOriginalScript());

  while (funcScript.getPointer() < funcScript.size() - 1 && !result.isReturn) {
    result = Parser::loadAndCalculate(funcScript, Constants::END_PARSING_STR);
    Utils::goToNextStatement(funcScript);
  }
  
  // 3. Return last result of the execution.
  ParserFunction::popLocalVariables();
  result.isReturn = false;
  return result;
}

//-------------------------------------------
Variable ShowFunction::evaluate(ParsingScript& script)
{
  string name = Utils::getToken(script);
  Utils::checkNotEmpty(name, Constants::SHOW);
  
  ParserFunction* func = ParserFunction::getFunction(name);
  CustomFunction* custFunc = dynamic_cast<CustomFunction*>(func);
  Utils::checkNotNull(name, custFunc);
  
  string body = Utils::beautifyScript(custFunc->getBody(), custFunc->getHeader());
  Translation::printScript(body);
  
  return Variable::emptyInstance;
}

//-------------------------------------------
Variable TranslateFunction::evaluate(ParsingScript& script)
{
  string lang = Utils::getToken(script);
  Utils::checkNotEmpty(lang, Constants::TRANSLATE);
  
  string name = Utils::getToken(script);
  Utils::checkNotEmpty(name, Constants::TRANSLATE);
 
  ParserFunction* func = ParserFunction::getFunction(name);
  CustomFunction* custFunc = dynamic_cast<CustomFunction*>(func);
  Utils::checkNotNull(name, custFunc);

  string body = Utils::beautifyScript(custFunc->getBody(), custFunc->getHeader());
  string translated = Translation::translateScript(body, lang);
  Translation::printScript(translated);
  
  return Variable::emptyInstance;
}

//-------------------------------------------
Variable IncludeFunction::evaluate(ParsingScript& script)
{
  Variable arg = Utils::getItem(script);
  string filename = arg.toString();
  
  string includeFile = Utils::getFileContents(filename);
  unordered_map<size_t, size_t> char2Line;
  string includeScript = Utils::convertToScript(includeFile, char2Line);
  
  ParsingScript tempScript(includeScript);
  tempScript.setChar2Line(char2Line);
  tempScript.setOriginalScript(includeFile);
  tempScript.setFilename(filename);

  while (tempScript.stillValid()) {
    Parser::loadAndCalculate(tempScript,
                             Constants::END_PARSING_STR);
    Utils::goToNextStatement(tempScript);
  }
  
  return Variable::emptyInstance;
}

//-------------------------------------------
Variable IsNullFunction::evaluate(ParsingScript& script)
{
  Variable varValue = Utils::getItem(script);
  bool isNull = varValue.type == Constants::NONE;
  return Variable(isNull);
}

//-------------------------------------------
Variable ContainsFunction::evaluate(ParsingScript& script)
{
  // 1. Get the name of the variable.
  string varName = Utils::getToken(script, Constants::NEXT_OR_END);
  Utils::checkNotEnd(script, Constants::CONTAINS);
  
  // 2. Get the current value of the variable.
  vector<Variable> arrayIndices = Utils::getArrayIndices(varName);
  
  ParserFunction* func = ParserFunction::getFunction(varName);
  Utils::checkNotNull(varName, func);
  Variable currentValue = func->getValue(script);
  
  // 2b. Special dealings with arrays:
  Variable* query = !arrayIndices.empty() ?
      GetVarFunction::extractArrayElement(&currentValue, arrayIndices) :
      &currentValue;

  /*if (!arrayIndices.empty()) {
    Variable* res = GetVarFunction::extractArrayElement(&currentValue, arrayIndices);
    currentValue = Variable::duplicate(res);
  }*/
  
  // 3. Get the value to be looked for.
  Variable searchValue = Utils::getItem(script);
  Utils::checkNotEnd(script, Constants::CONTAINS);
  
  // 4. Check if the value to search for exists.
  //bool exists = currentValue.exists(searchValue.toString());
  bool exists = query->exists(searchValue, true /* notEmpty */);
  
  Utils::moveBackIf(script, Constants::START_GROUP);
  return Variable(exists);
}
//-------------------------------------------
Variable AssignFunction::evaluate(ParsingScript& script)
{
  Variable varValue = Utils::getItem(script);
  
  // Special case for adding a string (or a number) to a string.
  while (varValue.type == Constants::STRING &&
         script.tryPrev() == '+') {
    Variable addition = Utils::getItem(script);
    varValue.strValue += addition.toString();
  }
  
  // Check if the variable to be set has the form of x[0],
  // meaning that this is an array element.
  vector<Variable> arrayIndices = Utils::getArrayIndices(m_name);
  
  if (arrayIndices.empty()) {
    ParserFunction::addGlobalOrLocalVariable(m_name,
                                             new GetVarFunction(varValue));
    return varValue;
  }
  
  Variable array;
  // Check if this array already exists.
  bool isGlobal = true;
  ParserFunction* func = ParserFunction::getFunction(m_name, isGlobal);
  if (func != 0) {
    array = func->getValue(script);
  }
  
  extendArray(array, arrayIndices, 0, varValue);
  
  ParserFunction::addGlobalOrLocalVariable(m_name,
                                           new GetVarFunction(array), isGlobal);
  
  return array;
}

void AssignFunction::extendArray(Variable& parent,
                                 const vector<Variable>& arrayIndices,
                                 size_t indexPtr,
                                 const Variable& varValue)
{
  if (arrayIndices.size() <= indexPtr) {
    return;
  }
  
  const Variable& index = arrayIndices[indexPtr];
  size_t currIndex = extendArray(parent, index);
  
  if (arrayIndices.size() - 1 == indexPtr) {
    parent.tuple[currIndex] = varValue;
    return;
  }
  
  Variable& son = parent.tuple[currIndex];
  extendArray(son, arrayIndices, indexPtr + 1, varValue);
}

size_t AssignFunction::extendArray(Variable& parent, const Variable& indexVar)
{
  parent.type = Constants::ARRAY;
  
  size_t arrayIndex = parent.getArrayIndex(indexVar);
  if (arrayIndex == string::npos) {
    // This not a "normal index" but a new string for the dictionary
    string hash = indexVar.toString();
    arrayIndex  = parent.set(hash, Variable::emptyInstance);
    return arrayIndex;
  }
  
  if (parent.tuple.size() <= arrayIndex) {
    for (size_t i = parent.tuple.size(); i <= arrayIndex; i++) {
      parent.tuple.emplace_back(Variable::emptyInstance);
    }
  }
  return arrayIndex;
}

ActionFunction* AssignFunction::newInstance()
{
  ActionFunction* newInstance = new AssignFunction();
  newInstance->setNewInstance();
  return newInstance;
}

//-------------------------------------------
Variable AppendlineFunction::evaluate(ParsingScript& script)
{
  string filename = Utils::getToken(script, Constants::NEXT_OR_END);
  Utils::moveForwardIf(script, Constants::NEXT_ARG);
  Utils::checkNotEnd(script, Constants::WRITEFILE);
  
  string msg = Utils::getToken(script, Constants::END_PARSING_STR);
  OS::appendLine(filename, msg);
  
  return Variable::emptyInstance;
}
//-------------------------------------------
Variable CdFunction::evaluate(ParsingScript& script)
{
  string dirname;
  if (script.tryCurrent() != Constants::END_STATEMENT) {
    dirname = Utils::getToken(script, Constants::END_PARSING_STR);
  }
  if (!dirname.compare("..") || m_oneUp) {
    dirname = OS::getParentDir();
  }
  OS::chdir(dirname);
  return Variable(dirname);
}
//-------------------------------------------
Variable CpFunction::evaluate(ParsingScript& script)
{
  string src = Utils::getToken(script, Constants::NEXT_OR_END);
  Utils::moveForwardIf(script, Constants::NEXT_ARG);
  Utils::checkNotEnd(script, Constants::COPY);
  
  string dst = Utils::getToken(script, Constants::END_PARSING_STR);
  
  OS::cp(src, dst);
  return Variable::emptyInstance;
}
//-------------------------------------------
Variable FindFunction::evaluate(ParsingScript& script)
{
  Utils::checkNotEnd(script, Constants::FIND);
  string search = Utils::getToken(script, Constants::NEXT_OR_END);
  Utils::moveForwardIf(script, Constants::NEXT_ARG);
  
  vector<string> results = OS::findFiles(search);
  
  Variable result(Constants::ARRAY);
  for (size_t i = 0; i < results.size(); i++) {
    result.tuple.emplace_back(results[i]);
  }
  
  return result;
}
//-------------------------------------------
Variable EnvFunction::evaluate(ParsingScript& script)
{
  string envVar = Utils::getToken(script, Constants::NEXT_OR_END);
  return OS::getEnv(envVar);
}

//-------------------------------------------
Variable SetEnvFunction::evaluate(ParsingScript& script)
{
  string envVar = Utils::getToken(script, Constants::NEXT_OR_END);
  Utils::moveForwardIf(script, Constants::NEXT_ARG);
  Utils::checkNotEnd(envVar, Constants::SETENV);
  
  string value = Utils::getToken(script, Constants::END_PARSING_STR);
  OS::setEnv(envVar, value);
  
  return Variable::emptyInstance;
}

//-------------------------------------------
Variable GrepFunction::evaluate(ParsingScript& script)
{
  string strPattern = Utils::getToken(script, Constants::NEXT_OR_END);
  bool ignoreCase = strPattern.compare("/i") == 0 || strPattern.compare("/i") == 0;
  if (ignoreCase) {
    strPattern = Utils::getToken(script, Constants::NEXT_OR_END);
  }
  Utils::checkNotEnd(script, Constants::GREP);
  
  Utils::moveForwardIf(script, Constants::NEXT_ARG, Constants::SPACE);
  string filesPattern = Utils::getToken(script, Constants::NEXT_OR_END);
  
  vector<string> files = OS::findFiles(filesPattern);
  vector<string> results = OS::findStringInFiles(files,
                                                 strPattern,
                                                 !ignoreCase);
  
  Variable result(Constants::ARRAY);
  for (size_t i = 0; i < results.size(); i++) {
    result.tuple.emplace_back(results[i]);
  }
  
  return result;
}

//-------------------------------------------
Variable LsFunction::evaluate(ParsingScript& script)
{
  string dirname;
  if (script.tryCurrent() != Constants::END_STATEMENT) {
    dirname = Utils::getToken(script, Constants::NEXT_OR_END);
  }
  
  Variable result(Constants::ARRAY);
  vector<string> results = OS::ls(dirname);
  
  for (size_t i = 0; i < results.size(); i++) {
    result.tuple.emplace_back(results[i]);
  }
  
  return result;
}
//-------------------------------------------
Variable MkdirFunction::evaluate(ParsingScript& script)
{
  string dirname = Utils::getToken(script, Constants::END_PARSING_STR);
  Utils::checkNotEmpty(dirname, Constants::MKDIR);

  OS::mkdir(dirname);
  return Variable::emptyInstance;
}
//-------------------------------------------
Variable MoreFunction::evaluate(ParsingScript& script)
{
  string filename = Utils::getToken(script, Constants::NEXT_OR_END);
  Utils::checkNotEmpty(filename, Constants::MORE);
  
  OS::more(filename);
  
  return Variable::emptyInstance;
}

//-------------------------------------------
Variable PwdFunction::evaluate(ParsingScript& script)
{
  return Variable(OS::pwd());
}
//-------------------------------------------
Variable RenameFunction::evaluate(ParsingScript& script)
{
  string src = Utils::getToken(script, Constants::NEXT_OR_END);
  Utils::moveForwardIf(script, Constants::NEXT_ARG);
  Utils::checkNotEmpty(src, Constants::MOVE);
  
  string dst = Utils::getToken(script, Constants::END_PARSING_STR);
  Utils::checkNotEmpty(dst, Constants::MOVE);
  
  OS::rename(src, dst);
  return Variable::emptyInstance;
}
//-------------------------------------------
Variable RmFunction::evaluate(ParsingScript& script)
{
  string dirname = Utils::getToken(script, Constants::END_PARSING_STR);
  Utils::checkNotEmpty(dirname, Constants::RM);

  OS::rm(dirname);
  return Variable::emptyInstance;
}

//-------------------------------------------
Variable ReadFunction::evaluate(ParsingScript& script)
{
  string input;
  getline(cin, input);
  return Variable(input);
}
//-------------------------------------------
Variable ReadfileFunction::evaluate(ParsingScript& script)
{
  string filename = Utils::getToken(script, Constants::END_PARSING_STR);
  Utils::checkNotEmpty(filename, Constants::READFILE);
  
  vector<string> linesFile = Utils::getFileLines(filename);
  
  Variable result(Constants::ARRAY);
  for (size_t i = 0; i < linesFile.size(); i++) {
    result.tuple.emplace_back(linesFile[i]);
  }

  return result;
}
//-------------------------------------------
Variable ReadnumFunction::evaluate(ParsingScript& script)
{
  string input;
  getline(cin, input);

  char* x;
  double num = ::strtod(input.c_str(), &x);
  if (::strlen(x) > 0) {
    throw ParsingException("Couldn't read number [" + input + "]");
  }

  return Variable(num);
}

//-------------------------------------------
Variable RunFunction::evaluate(ParsingScript& script)
{
  string cmd = Utils::getBodyBetween(script, Constants::NULL_CHAR,
                                     Constants::END_STATEMENT);
  Utils::checkNotEmpty(cmd, Constants::RUN);
  
  int status = OS::runCmd(cmd);
  
  return Variable(status);
}

//-------------------------------------------
Variable TailFunction::evaluate(ParsingScript& script)
{
  size_t maxLines = Constants::MAX_TAIL_LINES;
  
  string filename = Utils::getToken(script, Constants::NEXT_OR_END);
  Utils::checkNotEmpty(filename, Constants::TAIL);
  
  if (filename.size() > 1 && filename[0] == '-') {
    maxLines = ::atoi(filename.substr(1).c_str());
    if (maxLines > 0) {
      filename = Utils::getToken(script, Constants::END_PARSING_STR);
      Utils::checkNotEmpty(filename, Constants::TAIL);
    } else {
      maxLines = Constants::MAX_TAIL_LINES;
    }
  }
  
  OS::tail(filename, maxLines);
  
  return Variable::emptyInstance;
}

//-------------------------------------------
Variable TouchFunction::evaluate(ParsingScript& script)
{
  string filename = Utils::getToken(script, Constants::NEXT_OR_END);
  Utils::checkNotEmpty(filename, Constants::TOUCH);
  
  OS::touch(filename);
  
  return Variable::emptyInstance;
}

//-------------------------------------------
Variable WritefileFunction::evaluate(ParsingScript& script)
{
  string filename = Utils::getToken(script, Constants::NEXT_OR_END);
  Utils::moveForwardIf(script, Constants::NEXT_ARG);
  Utils::checkNotEmpty(filename, Constants::WRITEFILE);
  
  string msg = Utils::getToken(script, Constants::END_PARSING_STR);
  Utils::checkNotEmpty(msg, Constants::WRITEFILE);
  OS::writeFile(filename, msg);
  
  return Variable::emptyInstance;
}

static string threadIdToStr(thread::id threadId)
{
  ostringstream ss;
  ss << threadId;
  return ss.str();
}

//-------------------------------------------
void ThreadFunction::threadWork(const string& body)
{
  ParsingScript script(body);
  script.executeAll();
}
//-------------------------------------------
Variable ThreadFunction::evaluate(ParsingScript& script)
{
  if (m_join) {
    Variable threadId = Utils::getItem(script);
    string threadIdStr = threadId.strValue;
    Utils::checkNotEmpty(threadIdStr, "threadId");
    
    auto it = g_threads.find(threadIdStr);
    if (it != g_threads.end()) {
      throw ParsingException("Couldn't find thread [" +
                             threadIdStr + "]");

    }
    it->second->join();
    delete it->second;
    g_threads.erase(it);
    return Variable(threadId);
  }
  
  string body = Utils::getBodyBetween(script,
                                      Constants::START_ARG,
                                      Constants::END_ARG);
  
  std::thread* work = new thread(threadWork, body);
  string threadId = threadIdToStr(work->get_id());
  
  //g_threads[threadId] = work;
  if (m_detach) {
    work->detach();
    //work.join();
  }
  
  return Variable(threadId);
}

//-------------------------------------------
Variable ThreadIDFunction::evaluate(ParsingScript& script)
{
  string threadId =  threadIdToStr(this_thread::get_id());
  return Variable(threadId);
}

//-------------------------------------------
Variable SleepFunction::evaluate(ParsingScript& script)
{
  Variable sleepVar = Utils::getItem(script);
  Utils::checkNonNegInteger(sleepVar);

  long long sleepMs = sleepVar.numValue;
  this_thread::sleep_for(chrono::milliseconds(sleepMs));
  return Variable::emptyInstance;
}

//-------------------------------------------
Variable SignalWaitFunction::evaluate(ParsingScript& script)
{
  unique_lock<std::mutex> lock(g_mutex);

  if (m_signal) {
    g_signaled = true;
    g_cv.notify_all();
  } else {
    while (!g_signaled) {
      g_cv.wait(lock);
    }
    g_signaled = false; // reset it for the next time
  }
  
  return Variable::emptyInstance;
}

//-------------------------------------------
Variable LockFunction::evaluate(ParsingScript& script)
{
  unique_lock<std::mutex> lock(g_mutex);
  
  string body = Utils::getBodyBetween(script,
                                      Constants::START_ARG,
                                      Constants::END_ARG);

  ParsingScript lockScript(body);
  lockScript.executeAll();

  return Variable::emptyInstance;
}

//-------------------------------------------
Variable TypeFunction::evaluate(ParsingScript& script)
{
  // 1. Get the name of the variable.
  string varName = Utils::getToken(script, Constants::END_ARG_STR);
  Utils::checkNotEnd(script, Constants::SIZE);
  
  vector<Variable> arrayIndices = Utils::getArrayIndices(varName);
  
  // 2. Get the current value of the variable.
  ParserFunction* func = ParserFunction::getFunction(varName);
  Utils::checkNotNull(varName, func);
  Variable currentValue = func->getValue(script);
  Variable element = currentValue;
  
  // 2b. Special case for an array.
  if (!arrayIndices.empty()) {// array element
    element = *GetVarFunction::extractArrayElement(&currentValue, arrayIndices);
    Utils::moveForwardIf(script, Constants::END_ARRAY);
  }
  
  // 3. Take either the length of the underlying tuple or
  // string part if it is defined,
  // or the numerical part converted to a string otherwise.
  string type = Constants::typeToString(element.type);
  
  Utils::moveForwardIf(script, Constants::END_ARG, Constants::SPACE);
  return Variable(type);
}

//-------------------------------------------
Variable IncrDecrFunction::evaluate(ParsingScript& script)
{
  bool prefix = m_name.empty();
  if (prefix) {// If it is a prefix we do not have variable name yet.
    m_name = Utils::getToken(script, Constants::TOKEN_SEPARATION);
  }
  
  // Value to be added to the variable:
  int valueDelta = m_action == Constants::INCREMENT ? 1 : -1;
  int returnDelta = prefix ? valueDelta : 0;
  
  // Check if the variable to be set has the form of x(0),
  // meaning that this is an array element.
  double newValue = 0;
  vector<Variable> arrayIndices = Utils::getArrayIndices(m_name);
  
  bool isGlobal = true;
  ParserFunction* func = ParserFunction::getFunction(m_name, isGlobal);
  Utils::checkNotNull(m_name, func);
  
  Variable currentValue = func->getValue(script);
  
  if (!arrayIndices.empty() || script.tryCurrent() == Constants::START_ARRAY) {// array element
    if (prefix) {
      string tmpName = m_name + script.rest();
      size_t delta = 0;
      arrayIndices = Utils::getArrayIndices(tmpName, delta);
      script.forward(max(0, (int)delta - (int)tmpName.size()));
    }
    
    Variable* element = GetVarFunction::extractArrayElement(&currentValue, arrayIndices);
    Utils::moveForwardIf(script, Constants::END_ARRAY);
    
    newValue = element->numValue + returnDelta;
    element->numValue += valueDelta;
  }
  else { // A normal variable.
    newValue = currentValue.numValue + returnDelta;
    currentValue.numValue += valueDelta;
  }
  
  ParserFunction::addGlobalOrLocalVariable(m_name,
                                           new GetVarFunction(currentValue), isGlobal);
  return newValue;
}

ActionFunction* IncrDecrFunction::newInstance()
{
  ActionFunction* newInstance = new IncrDecrFunction();
  newInstance->setNewInstance();
  return newInstance;
}

//-------------------------------------------
Variable OperatorAssignFunction::evaluate(ParsingScript& script)
{
  Variable right = Utils::getItem(script);
  
  // Check if the variable to be set has the form of x(0),
  // meaning that this is an array element.
  vector<Variable> arrayIndices = Utils::getArrayIndices(m_name);
  
  bool isGlobal = true;
  ParserFunction* func = ParserFunction::getFunction(m_name, isGlobal);
  Utils::checkNotNull(m_name, func);
  Variable currentValue = func->getValue(script);
  Variable left = currentValue;
  
  if (!arrayIndices.empty()) {// array element
    left = *GetVarFunction::extractArrayElement(&currentValue, arrayIndices);
    Utils::moveForwardIf(script, Constants::END_ARRAY);
  }
  
  if (left.type == Constants::NUMBER) {
    numberOperator(left, right, m_action);
  } else {
    stringOperator(left, right, m_action);
  }
  
  if (!arrayIndices.empty()) {// array element
    AssignFunction::extendArray(currentValue, arrayIndices, 0, left);
    ParserFunction::addGlobalOrLocalVariable(m_name,
                                             new GetVarFunction(currentValue), isGlobal);
  } else {
    ParserFunction::addGlobalOrLocalVariable(m_name,
                                             new GetVarFunction(left), isGlobal);
  }
  return left;
}

void OperatorAssignFunction::numberOperator(Variable& left,
                                            const Variable& right, const string& action)
{
  if (action.compare("+=") == 0) {
    left.numValue += right.numValue;
  } else if (action.compare("-=") == 0) {
    left.numValue -= right.numValue;
  } else if (action.compare("*=") == 0) {
    left.numValue *= right.numValue;
  } else if (action.compare("/=") == 0) {
    left.numValue /= right.numValue;
  } else if (action.compare("^=") == 0) {
    left.numValue = pow(left.numValue, right.numValue);
  } else if (action.compare("%=") == 0) {
    left.numValue = (int)left.numValue % (int)right.numValue;
  } else if (action.compare("&=") == 0) {
    left.numValue = (int)left.numValue & (int)right.numValue;
  } else if (action.compare("|=") == 0) {
    left.numValue = (int)left.numValue | (int)right.numValue;
  }
}

void OperatorAssignFunction::stringOperator(Variable& left,
                                            const Variable& right, const string& action)
{
  if (action.compare("+=") == 0) {
    left.strValue += right.toString();
  }
}

ActionFunction* OperatorAssignFunction::newInstance()
{
  ActionFunction* newInstance = new OperatorAssignFunction();
  newInstance->setNewInstance();
  return newInstance;
}



