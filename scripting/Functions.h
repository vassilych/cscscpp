//
//  Functions.hpp
//  scripting
//
//  Created by Vassili Kaplan on 30/03/16.
//  Copyright © 2016 Vassili Kaplan. All rights reserved.
//

#ifndef Functions_h
#define Functions_h

#include "ParserFunction.h"
#include "Interpreter.h"
#include "UtilsOS.h"

class IfStatement;

class StringOrNumberFunction : public ParserFunction
{
public:
  virtual Variable evaluate(ParsingScript& script);
  void setItem(const string& item) { m_item = item; }
  
private:
  string m_item;
};

//-------------------------------------------
class IdentityFunction : public ParserFunction
{
public:
  virtual Variable evaluate(ParsingScript& script);
};

//-------------------------------------------
class AbsFunction : public ParserFunction
{
public:
  virtual Variable evaluate(ParsingScript& script);
};
//-------------------------------------------
class AddFunction : public ParserFunction
{
public:
  virtual Variable evaluate(ParsingScript& script);
};
//-------------------------------------------
class AllFunctions : public ParserFunction
{
public:
  virtual Variable evaluate(ParsingScript& script);
};
//-------------------------------------------
class AllVariables : public ParserFunction
{
public:
  virtual Variable evaluate(ParsingScript& script);
};
//-------------------------------------------
class CeilFunction : public ParserFunction
{
public:
  virtual Variable evaluate(ParsingScript& script);
};
//-------------------------------------------
class CosFunction : public ParserFunction
{
public:
  virtual Variable evaluate(ParsingScript& script);
};
//-------------------------------------------
class EnvFunction : public ParserFunction
{
public:
  virtual Variable evaluate(ParsingScript& script);
};
//-------------------------------------------
class SetEnvFunction : public ParserFunction
{
public:
  virtual Variable evaluate(ParsingScript& script);
};
//-------------------------------------------
class ExpFunction : public ParserFunction
{
public:
  virtual Variable evaluate(ParsingScript& script);
};
//-------------------------------------------
class FloorFunction : public ParserFunction
{
public:
  virtual Variable evaluate(ParsingScript& script);
};
//-------------------------------------------
class IndexOfFunction : public ParserFunction
{
public:
  virtual Variable evaluate(ParsingScript& script);
};
//-------------------------------------------
class LogFunction : public ParserFunction
{
public:
  virtual Variable evaluate(ParsingScript& script);
};

//-------------------------------------------
class PiFunction : public ParserFunction
{
public:
  virtual Variable evaluate(ParsingScript& script);
};
//-------------------------------------------
class PowFunction : public ParserFunction
{
public:
  virtual Variable evaluate(ParsingScript& script);
};
//-------------------------------------------
class ShowFunction : public ParserFunction
{
public:
  virtual Variable evaluate(ParsingScript& script);
};
//-------------------------------------------
class TranslateFunction : public ParserFunction
{
public:
  virtual Variable evaluate(ParsingScript& script);
};

//-------------------------------------------
class PrintFunction : public ParserFunction
{
public:
  PrintFunction(bool newLine = true, OS::Color color = OS::Color::NONE) :
                m_newLine(newLine), m_color(color) {}
  virtual Variable evaluate(ParsingScript& script);
private:
  bool m_newLine;
  OS::Color m_color;
};
//-------------------------------------------
class ProcessorTimeFunction : public ParserFunction
{
public:
  virtual Variable evaluate(ParsingScript& script);
};
//-------------------------------------------
class RoundFunction : public ParserFunction
{
public:
  virtual Variable evaluate(ParsingScript& script);
};
//-------------------------------------------
class SqrtFunction : public ParserFunction
{
public:
  virtual Variable evaluate(ParsingScript& script);
};
//-------------------------------------------
class SinFunction : public ParserFunction
{
public:
  virtual Variable evaluate(ParsingScript& script);
};
//-------------------------------------------
class SizeFunction : public ParserFunction
{
public:
  virtual Variable evaluate(ParsingScript& script);
};

//-------------------------------------------
class SubstrFunction : public ParserFunction
{
public:
  virtual Variable evaluate(ParsingScript& script);
};

//-------------------------------------------
class ThrowFunction : public ParserFunction
{
public:
  virtual Variable evaluate(ParsingScript& script);
};

//-------------------------------------------
class AppendlineFunction : public ParserFunction
{
public:
  virtual Variable evaluate(ParsingScript& script);
};
//-------------------------------------------
class BreakStatement : public ParserFunction
{
public:
  virtual Variable evaluate(ParsingScript& script);
};
//-------------------------------------------
class ContinueStatement : public ParserFunction
{
public:
  virtual Variable evaluate(ParsingScript& script);
};
//-------------------------------------------
class ExitStatement : public ParserFunction
{
public:
  virtual Variable evaluate(ParsingScript& script);
};
//-------------------------------------------
class ForStatement : public ParserFunction
{
public:
  virtual Variable evaluate(ParsingScript& script);
};
//-------------------------------------------
class IfStatement : public ParserFunction
{
public:
  virtual Variable evaluate(ParsingScript& script);
};
//-------------------------------------------
class WhileStatement : public ParserFunction
{
public:
  virtual Variable evaluate(ParsingScript& script);
};
//-------------------------------------------
class TryStatement : public ParserFunction
{
public:
  virtual Variable evaluate(ParsingScript& script);
};

//-------------------------------------------
class ReturnStatement : public ParserFunction
{
public:
  virtual Variable evaluate(ParsingScript& script);
};

//-------------------------------------------
class FunctionCreator : public ParserFunction
{
public:
  virtual Variable evaluate(ParsingScript& script);
};
//-------------------------------------------
class CustomFunction : public ParserFunction
{
public:
  CustomFunction(const string&         funcName,
                 const string&         funcBody,
                 const vector<string>& args,
                 const ParsingScript&  parentScript,
                 size_t                parentOffset = 0) :
    m_body(funcBody), m_args(args), m_parentScript(parentScript),
    m_parentOffset(parentOffset)
  { m_name = funcName;}
  
  virtual Variable evaluate(ParsingScript& script);
  
  string getBody() { return m_body; }
  string getHeader();
  
private:
  string         m_body;
  vector<string> m_args;
  ParsingScript  m_parentScript;
  size_t         m_parentOffset = 0;
};

//-------------------------------------------
class IncludeFunction : public ParserFunction
{
public:
  virtual Variable evaluate(ParsingScript& script);
private:
};
//-------------------------------------------
class IsNullFunction : public ParserFunction
{
public:
  virtual Variable evaluate(ParsingScript& script);
private:
};
//-------------------------------------------
class ContainsFunction : public ParserFunction
{
public:
  virtual Variable evaluate(ParsingScript& script);
private:
};

//-------------------------------------------
class GetVarFunction : public ParserFunction
{
public:
  GetVarFunction(const Variable& value) :
          m_value(value), m_delta(0) {}
  virtual Variable evaluate(ParsingScript& script);
  
  const Variable& getValue() const { return m_value; };
  void setIndices(const vector<Variable>& arrayIndices)
          { m_arrayIndices = arrayIndices; }
  void setDelta(size_t delta)
          { m_delta = delta; }
  
  static Variable* extractArrayElement(Variable* array,
                                       const vector<Variable>& indices);
private:
  size_t m_delta;
  Variable m_value;
  vector<Variable> m_arrayIndices;
};

//-------------------------------------------
//-------------------------------------------
class CdFunction : public ParserFunction
{
public:
  CdFunction(bool oneUp = false) : m_oneUp(oneUp) {}
  virtual Variable evaluate(ParsingScript& script);
private:
  bool m_oneUp;
};
//-------------------------------------------
class CpFunction : public ParserFunction
{
public:
  virtual Variable evaluate(ParsingScript& script);
};
//-------------------------------------------
class FindFunction : public ParserFunction
{
public:
  virtual Variable evaluate(ParsingScript& script);
};
//-------------------------------------------
class GrepFunction : public ParserFunction
{
public:
  virtual Variable evaluate(ParsingScript& script);
};
//-------------------------------------------
class LsFunction : public ParserFunction
{
public:
  virtual Variable evaluate(ParsingScript& script);
};
//-------------------------------------------
class MkdirFunction : public ParserFunction
{
public:
  virtual Variable evaluate(ParsingScript& script);
};
//-------------------------------------------
class MoreFunction : public ParserFunction
{
public:
  virtual Variable evaluate(ParsingScript& script);
};
//-------------------------------------------
class PwdFunction : public ParserFunction
{
public:
  virtual Variable evaluate(ParsingScript& script);
};
//-------------------------------------------
class ReadFunction : public ParserFunction
{
public:
  virtual Variable evaluate(ParsingScript& script);
};
//-------------------------------------------
class ReadfileFunction : public ParserFunction
{
public:
  virtual Variable evaluate(ParsingScript& script);
};
//-------------------------------------------
class ReadnumFunction : public ParserFunction
{
public:
  virtual Variable evaluate(ParsingScript& script);
};
//-------------------------------------------
class RenameFunction : public ParserFunction
{
public:
  virtual Variable evaluate(ParsingScript& script);
};
//-------------------------------------------
class RmFunction : public ParserFunction
{
public:
  virtual Variable evaluate(ParsingScript& script);
};
//-------------------------------------------
class RunFunction : public ParserFunction
{
public:
  virtual Variable evaluate(ParsingScript& script);
};
//-------------------------------------------
class TailFunction : public ParserFunction
{
public:
  virtual Variable evaluate(ParsingScript& script);
};
//-------------------------------------------
class TouchFunction : public ParserFunction
{
public:
  virtual Variable evaluate(ParsingScript& script);
};
//-------------------------------------------
class WritefileFunction : public ParserFunction
{
public:
  virtual Variable evaluate(ParsingScript& script);
};

//-------------------------------------------
//-------------------------------------------
class IncrDecrFunction : public ActionFunction
{
public:
  virtual Variable evaluate(ParsingScript& script);
  virtual ActionFunction* newInstance();
};
//-------------------------------------------
class AssignFunction : public ActionFunction
{
public:
  virtual Variable evaluate(ParsingScript& script);
  virtual ActionFunction* newInstance();
  
  static void extendArray(Variable& parent,
                          const vector<Variable>& arrayIndices,
                          size_t indexPtr,
                          const Variable& varValue);
private:
  static size_t extendArray(Variable& parent, const Variable& indexVar);
};
//-------------------------------------------
class OperatorAssignFunction : public ActionFunction
{
public:
  virtual Variable evaluate(ParsingScript& script);
  virtual ActionFunction* newInstance();
  
  static void numberOperator(Variable& left,
                             const Variable& right, const string& action);
  static void stringOperator(Variable& left,
                             const Variable& right, const string& action);
};


#endif /* Functions_h */
