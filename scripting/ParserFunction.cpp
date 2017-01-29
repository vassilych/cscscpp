//
//  ParserFunction.cpp
//  scripting
//
//  Created by Vassili Kaplan on 29/03/16.
//  Copyright © 2016 Vassili Kaplan. All rights reserved.
//

#include "Functions.h"
#include "ParserFunction.h"
#include "Translation.h"
#include "UtilsOS.h"


#include <iostream>

ParserFunctionMap ParserFunction::s_functions;
ParserFunctionMap ParserFunction::s_globals;
ActionFunctionMap ParserFunction::s_actions;
stack<ParserFunction::StackLevel> ParserFunction::s_locals;

StringOrNumberFunction* ParserFunction::s_strOrNumFunction =
new StringOrNumberFunction();
IdentityFunction*       ParserFunction::s_idFunction =
new IdentityFunction();

// A "virtual" Constructor
ParserFunction::ParserFunction(ParsingScript& script,
                               const string& item, char ch, string& action) :
                               m_newInstance(false)
{
  if (item.empty() && (ch == Constants::START_ARG || !script.stillValid())) {
    // There is no function, just an expression in parentheses
    m_impl = s_idFunction;
    return;
  }
  
  m_impl = getRegisteredAction(item, action);
  if (m_impl != 0) {
    return;
  }
  
  // Is this an array element?
  m_impl = getArrayFunction(item, script, action);
  if (m_impl != 0) {
    return;
  }
  
  m_impl = getFunction(item);
  if (m_impl != 0) {
    return;
  }
  
  if (m_impl == s_strOrNumFunction && item.empty())  {
    string problem = !action.empty() ? action : string(1, ch);
    string restData = string(1, ch) + script.rest();
    throw ParsingException("Couldn't parse [" + problem + "] in " + restData + "...");
  }

  // Function not found, will try to parse this as a string in quotes or a number.
  m_impl = s_strOrNumFunction;
  s_strOrNumFunction->setItem(item);
}

ParserFunction::~ParserFunction()
{
  if (m_impl != 0 && m_impl != this && m_impl->isNewInstance()) {
    delete m_impl;
  }
}

Variable ParserFunction::getValue(ParsingScript& script)
{
  Variable result = m_impl->evaluate(script);
  return result;
}

ParserFunction* ParserFunction::getArrayFunction(const string& name,
                                                 ParsingScript& script, const string& action)
{
  string arrayName(name);
  size_t arrayStart = arrayName.find(Constants::START_ARRAY);
  if (arrayStart == string::npos) {
    return 0;
  }
  
  size_t delta = 0;
  string parsing = script.rest();
  vector<Variable> arrayIndices = Utils::getArrayIndices(arrayName, delta);
  
  if (arrayIndices.empty()) {
    return 0;
  }
  ParserFunction* pf = ParserFunction::getFunction(arrayName);
  GetVarFunction* varFunc = dynamic_cast<GetVarFunction*>(pf);
  if (varFunc == 0) {
    return 0;
  }
  
  script.backward(name.size() - arrayStart - 1);
  script.backward(action.size());
  delta -= arrayName.size();
  
  varFunc->setIndices(arrayIndices);
  varFunc->setDelta(delta);
  return varFunc;
}

ParserFunction* ParserFunction::getFunction(const string& name, bool& isGlobal)
{
  // First search among local variables.
  if (!s_locals.empty()) {
    isGlobal = false;;
    const StackLevel& level = s_locals.top();
    const ParserFunctionMap& locals = level.variables;
    
    auto it = locals.find(name);
    if (it != locals.end()) {
      return it->second;
    }
  }
  
  isGlobal = true;
  
  // Check if a global variable exists
  auto it = s_globals.find(name);
  if (it != s_globals.end()) {
    return it->second;
  }
  
  // Check if a global function exists and is registered (e.g. pi, exp)
  it = s_functions.find(name);
  if (it != s_functions.end()) {
    return it->second;
  }
  
  return 0;
}

ActionFunction* ParserFunction::getRegisteredAction(const string& name,
                                                    string& action)
{
  ActionFunction* actionFunction = getAction(action);
  if (actionFunction == 0) {
    return 0;
  }
  
  // If passed action exists and is registered we are done.
  ActionFunction* theAction = actionFunction->newInstance();
  theAction->setName(name);
  theAction->setAction(action);
  
  action = Constants::EMPTY;
  return theAction;
}

ActionFunction* ParserFunction::getAction(const string& action)
{
  if (action.empty()) {
    return 0;
  }
  
  auto it = s_actions.find(action);
  if (it == s_actions.end()) {
    return 0;
  }
  
  return it->second;
}

void ParserFunction::addAction(const string& name, ActionFunction* action)
{
  s_actions[name] = action;
}

void ParserFunction::addGlobalFunction(const string& name, ParserFunction* function,
                                       bool isNative)
{
  add(s_functions, function, name, isNative);
}

void ParserFunction::addGlobalOrLocalVariable(const string& name,
                                              ParserFunction* var,
                                              bool onlyGlobal)
{
  
  if (!onlyGlobal && !s_locals.empty()) {
    var->setName(name);
    addLocalVariable(var);
  } else {
    addGlobalVariable(name, var);
  }
}

void ParserFunction::addGlobalVariable(const string& name, ParserFunction* var)
{
  add(s_globals, var, name, false);
}

void ParserFunction::addLocalVariable(ParserFunction* local)
{
  if (s_locals.empty()) {
    s_locals.emplace();
  }
  local->setGlobal(false);
  
  ParserFunctionMap& topStack = s_locals.top().variables;
  add(topStack, local, local->getName(), false);
}

template <class T, class S>
void ParserFunction::add(T& container, S& value, const string& key,
                         bool isNative)
{
  if (value->getName().empty()) {
    value->setName(key);
  }
  value->setNative(isNative);
  
  if (!isNative) {
    Translation::addTempKeyword(key);
  } else {
    Translation::addNativeKeyword(key);
  }

  auto tryInsert = container.insert({key, value});
  if (!tryInsert.second) {
    // The variable or function already exists.
    if (isNative) {
      throw ParsingException("Global name [" + key + "] already registered");
    }
    // Delete it and replace with the new one.
    delete tryInsert.first->second;
    tryInsert.first->second = value;
  }
}

string ParserFunction::invalidateStacksAfterLevel(size_t level)
{
  string stackDescr;
  while (s_locals.size() > level) {
    string stackName = popLocalVariables();
    if (!stackName.empty()) {
      stackDescr += Constants::NEW_LINE + "  " + stackName + "()";
    }
  }
  
  return stackDescr;
}

string ParserFunction::popLocalVariables()
{
  if (s_locals.empty()) {
    return "";
  }
  
  StackLevel& stLevel = s_locals.top();
  string stackName = stLevel.name;
  
  stLevel.cleanUp();
  s_locals.pop();
  
  return stackName;
}

void ParserFunction::addLocalVariables(StackLevel& locals)
{
  s_locals.emplace(locals);
}

void ParserFunction::popLocalVariable(const string& name)
{
  if (s_locals.empty()) {
    return;
  }
  
  s_locals.top().cleanUp(name);
}

void ParserFunction::addStackLevel(const string& name)
{
  s_locals.emplace(name);
}

size_t ParserFunction::getCurrentStackLevel()
{
  return s_locals.size();
}

void ParserFunction::allFunctions()
{
  OS::print("*** All available functions ***", true);

  printVars(s_functions, false);

  OS::print(string(40, '*'), true);
}

// We need the hack below in order to access the Stack container.
// And we need its container to iterate over all its elements.
template <class ADAPTER>
typename ADAPTER::container_type & getContainer (ADAPTER &a)
{
  struct hack : ADAPTER {
    static typename ADAPTER::container_type & get (ADAPTER &a) {
      return a.*&hack::c;
    }
  };
  return hack::get(a);
}

void ParserFunction::allVariables()
{
  OS::print("*** All global variables ***", true);
  printVars(s_globals);
  
  deque<StackLevel>& container = getContainer(s_locals);
  for (auto it = container.begin(); it != container.end(); ++it) {
    StackLevel& st = *it;
    OS::print("*** Local variables of " + st.name + " ***", true);
    printVars(st.variables);
  }
  
  OS::print(string(40, '*'), true);
}

template <class T>
void ParserFunction::printVars(const T& container, bool getValues)
{
  vector<string> results;
  results.reserve(container.size());
  
  for (auto it = container.begin(); it != container.end(); ++it) {
    const string& name  = it->first;
    
    if (getValues) {
      GetVarFunction* impl = dynamic_cast<GetVarFunction*>(it->second->m_impl);
      string value = impl != nullptr ? impl->getValue().toPrint() : "";
      results.push_back(name + " (" + value + ")");
    } else {
      const string& translation = it->second->getName();
      string toAdd = name == translation ? name :
                     name + " (" + translation + ")";
      results.push_back(toAdd);
    }
  }
  
  std::sort(results.begin(), results.end());
  for (auto it = results.begin(); it != results.end(); ++it) {
    OS::print(*it, true);
  }
}

void CustomFunction::StackLevel::cleanUp()
{
  for (auto it = variables.begin(); it != variables.end(); ++it) {
    ParserFunction* pf = it->second;
    delete pf;
    it->second = 0;
  }
  variables.clear();
}

void CustomFunction::StackLevel::cleanUp(const string& name)
{
  auto it = variables.find(name);
  if (it != variables.end()) {
    delete it->second;
    variables.erase(it);
  }
}

