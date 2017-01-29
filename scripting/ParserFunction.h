//
//  ParserFunction.h
//  scripting
//
//  Created by Vassili Kaplan on 29/03/16.
//  Copyright © 2016 Vassili Kaplan. All rights reserved.
//

#ifndef ParserFunction_h
#define ParserFunction_h

#include "Constants.h"
#include "Utils.h"
#include "Variable.h"

class ParserFunction;
class ActionFunction;
class StringOrNumberFunction;
class IdentityFunction;

using ParserFunctionMap = unordered_map<string, ParserFunction*>;
using ActionFunctionMap = unordered_map<string, ActionFunction*>;

class ParserFunction
{
public:
  
  struct StackLevel
  {
    StackLevel(const string& nm = Constants::EMPTY) : name(nm) {}
    
    void cleanUp(const string& name);
    void cleanUp();
    
    string name;
    ParserFunctionMap variables;
  };
  
  Variable getValue(ParsingScript& script);
  
  ParserFunction() : m_impl(this), m_newInstance(false) {}
  
  ParserFunction(ParsingScript& script,
                 const string& item, char ch, string& action);
  
  virtual ~ParserFunction();
  
  // By default return same object, but give overriding classes
  // a chance to return something else.
  virtual ParserFunction* newInstance() { return this; }
  
  const string& getName() const    { return m_name; }
  void setName(const string& name) { m_name = name; }
  
  bool isGlobal() const      { return m_isGlobal; }
  void setGlobal(bool value) { m_isGlobal = value; }
  
  bool isNative() const      { return m_isNative; }
  void setNative(bool value) { m_isNative = value; }
  
  void setNewInstance() { m_newInstance = true; }
  bool isNewInstance()  { return m_newInstance; }
  
  static ActionFunction* getRegisteredAction(const string& name, string& action);
  
  static ParserFunction* getFunction(const string& name)
  { bool isGlobal = false; return getFunction(name, isGlobal); }
  static ParserFunction* getFunction(const string& name, bool& isGlobal);
  
  static ActionFunction* getAction(const string& action);
  
  static ParserFunction* getArrayFunction(const string& name, ParsingScript& script,
                                          const string& action);
  
  static void addAction(const string& name, ActionFunction* action);
  
  static void addGlobalOrLocalVariable(const string& name,
                                       ParserFunction* function,
                                       bool onlyGlobal = false);
  static void addGlobalFunction(const string& name, ParserFunction* function,
                                bool isNative = true);
  static void addGlobalVariable(const string& name, ParserFunction* var);
  
  static void addLocalVariable(ParserFunction* local);
  static void addLocalVariables(StackLevel& locals);
  
  static string invalidateStacksAfterLevel(size_t level);
  static string popLocalVariables();
  static void popLocalVariable(const string& name);
  
  static void addStackLevel(const string& name);
  static size_t getCurrentStackLevel();
  
  template <class T, class S>
  static void add(T& container, S& value, const string& key,
                  bool isNative = true);
  
  static void allFunctions();
  static void allVariables();
  template <class T>
    static void printVars(const T& container, bool getValues = true);

  static const stack<StackLevel>& getExecutionStack()
  { return s_locals; }
  
protected:
  
  // The real implementation will be in the derived classes
  virtual Variable evaluate(ParsingScript& script)
  {
    return Variable::emptyInstance;
  }
  
  string m_name;
  bool m_isGlobal = true;
  bool m_isNative = true;
  
private:
  
  ParserFunction* m_impl;
  bool m_newInstance;
  
  static ParserFunctionMap s_functions;
  static ParserFunctionMap s_globals;
  static ActionFunctionMap s_actions;
  static stack<StackLevel> s_locals;
  
  static StringOrNumberFunction* s_strOrNumFunction;
  static IdentityFunction*       s_idFunction ;
};

class ActionFunction : public ParserFunction
{
public:
  
  virtual ~ActionFunction() {}
  virtual ActionFunction* newInstance() { return this; }
  
  void setAction(const string& action) { m_action = action; }
  
protected:
  string  m_action;
};



#endif /* ParserFunction_hpp */
