//
//  Interpreter.cpp
//  scripting
//
//  Created by Vassili Kaplan on 31/03/16.
//  Copyright © 2016 Vassili Kaplan. All rights reserved.
//

#include <iostream>

#include "Interpreter.h"
#include "Functions.h"
#include "Parser.h"
#include "ParserFunction.h"
#include "Translation.h"

void Interpreter::init()
{
  ParserFunction::addGlobalFunction(Constants::ALL,         new AllFunctions());
  ParserFunction::addGlobalFunction(Constants::ALLVARS,     new AllVariables());
  // Add control flow functions
  ParserFunction::addGlobalFunction(Constants::BREAK,       new BreakStatement());
  ParserFunction::addGlobalFunction(Constants::CONTINUE,    new ContinueStatement());
  ParserFunction::addGlobalFunction(Constants::EXIT,        new ExitStatement());
  ParserFunction::addGlobalFunction(Constants::FOR,         new ForStatement());
  ParserFunction::addGlobalFunction(Constants::FUNCTION,    new FunctionCreator());
  ParserFunction::addGlobalFunction(Constants::INCLUDE,     new IncludeFunction());
  ParserFunction::addGlobalFunction(Constants::IF,          new IfStatement());
  ParserFunction::addGlobalFunction(Constants::RETURN,      new ReturnStatement());
  ParserFunction::addGlobalFunction(Constants::THROW,       new ThrowFunction());
  ParserFunction::addGlobalFunction(Constants::TRY,         new TryStatement());
  ParserFunction::addGlobalFunction(Constants::WHILE,       new WhileStatement());
  
  // Add global math and auxiliary functions
  ParserFunction::addGlobalFunction(Constants::ABS,         new AbsFunction());
  ParserFunction::addGlobalFunction(Constants::ADD,         new AddFunction());
  ParserFunction::addGlobalFunction(Constants::APPENDLINE,  new AppendlineFunction());
  ParserFunction::addGlobalFunction(Constants::CEIL,        new CeilFunction());
  ParserFunction::addGlobalFunction(Constants::COS,         new CosFunction());
  ParserFunction::addGlobalFunction(Constants::CONTAINS,    new ContainsFunction());
  ParserFunction::addGlobalFunction(Constants::EXP,         new ExpFunction());
  ParserFunction::addGlobalFunction(Constants::FLOOR,       new FloorFunction());
  ParserFunction::addGlobalFunction(Constants::ISNULL,      new IsNullFunction());
  ParserFunction::addGlobalFunction(Constants::INDEX_OF,    new IndexOfFunction());
  ParserFunction::addGlobalFunction(Constants::JOIN,        new ThreadFunction(false, true));
  ParserFunction::addGlobalFunction(Constants::LOG,         new LogFunction());
  ParserFunction::addGlobalFunction(Constants::LOCK,        new LockFunction());
  ParserFunction::addGlobalFunction(Constants::MORE,        new MoreFunction());
  ParserFunction::addGlobalFunction(Constants::PI,          new PiFunction());
  ParserFunction::addGlobalFunction(Constants::POW,         new PowFunction());
  ParserFunction::addGlobalFunction(Constants::PRINT,       new PrintFunction(true));
  ParserFunction::addGlobalFunction(Constants::PRINT_BLACK, new PrintFunction(true, OS::Color::BLACK));
  ParserFunction::addGlobalFunction(Constants::PRINT_GRAY,  new PrintFunction(true, OS::Color::GRAY));
  ParserFunction::addGlobalFunction(Constants::PRINT_GREEN, new PrintFunction(true, OS::Color::GREEN));
  ParserFunction::addGlobalFunction(Constants::PRINT_RED,   new PrintFunction(true, OS::Color::RED));
  ParserFunction::addGlobalFunction(Constants::PRINT_WHITE, new PrintFunction(true, OS::Color::WHITE));
  ParserFunction::addGlobalFunction(Constants::PSTIME,      new ProcessorTimeFunction());
  ParserFunction::addGlobalFunction(Constants::READ,        new ReadFunction());
  ParserFunction::addGlobalFunction(Constants::READFILE,    new ReadfileFunction());
  ParserFunction::addGlobalFunction(Constants::READNUM,     new ReadnumFunction());
  ParserFunction::addGlobalFunction(Constants::ROUND,       new RoundFunction());
  ParserFunction::addGlobalFunction(Constants::RUN,         new RunFunction());
  ParserFunction::addGlobalFunction(Constants::SHOW,        new ShowFunction());
  ParserFunction::addGlobalFunction(Constants::SIGNAL,      new SignalWaitFunction(true));
  ParserFunction::addGlobalFunction(Constants::SIN,         new SinFunction());
  ParserFunction::addGlobalFunction(Constants::SIZE,        new SizeFunction());
  ParserFunction::addGlobalFunction(Constants::SQRT,        new SqrtFunction());
  ParserFunction::addGlobalFunction(Constants::SLEEP,       new SleepFunction());
  ParserFunction::addGlobalFunction(Constants::SUBSTR,      new SubstrFunction());
  ParserFunction::addGlobalFunction(Constants::TAIL,        new TailFunction());
  ParserFunction::addGlobalFunction(Constants::THREAD,      new ThreadFunction(true));
  ParserFunction::addGlobalFunction(Constants::THREAD_ID,   new ThreadIDFunction());
  ParserFunction::addGlobalFunction(Constants::THREAD_J,    new ThreadFunction(false));
  ParserFunction::addGlobalFunction(Constants::TRANSLATE,   new TranslateFunction());
  ParserFunction::addGlobalFunction(Constants::TOUCH,       new TouchFunction());
  ParserFunction::addGlobalFunction(Constants::TYPE,        new TypeFunction());
  ParserFunction::addGlobalFunction(Constants::WAIT,        new SignalWaitFunction(false));
  ParserFunction::addGlobalFunction(Constants::WRITE,       new PrintFunction(false));
  ParserFunction::addGlobalFunction(Constants::WRITEFILE,   new WritefileFunction());
  
  ParserFunction::addGlobalFunction(Constants::CD,          new CdFunction());
  ParserFunction::addGlobalFunction(Constants::CD__,        new CdFunction(true));
  ParserFunction::addGlobalFunction(Constants::COPY,        new CpFunction());
  ParserFunction::addGlobalFunction(Constants::ENV,         new EnvFunction());
  ParserFunction::addGlobalFunction(Constants::FIND,        new FindFunction());
  ParserFunction::addGlobalFunction(Constants::GREP,        new GrepFunction());
  ParserFunction::addGlobalFunction(Constants::LS,          new LsFunction());
  ParserFunction::addGlobalFunction(Constants::MKDIR,       new MkdirFunction());
  ParserFunction::addGlobalFunction(Constants::SETENV,      new SetEnvFunction());
  ParserFunction::addGlobalFunction(Constants::PWD,         new PwdFunction());
  ParserFunction::addGlobalFunction(Constants::MOVE,        new RenameFunction());
  ParserFunction::addGlobalFunction(Constants::RM,          new RmFunction());
  
  // Add operators
  ParserFunction::addAction(Constants::ASSIGN,              new AssignFunction());
  ParserFunction::addAction(Constants::INCREMENT,           new IncrDecrFunction());
  ParserFunction::addAction(Constants::DECREMENT,           new IncrDecrFunction());

  for (size_t i = 0; i < Constants::OPER_ACTIONS.size(); i++) {
    ParserFunction::addAction(Constants::OPER_ACTIONS[i],
                              new OperatorAssignFunction());
  }
  
  readConfig("cscs.cfg");
}

Variable Interpreter::process(const string& scriptData)
{
  unordered_map<size_t, size_t> char2Line;
  string data = Utils::convertToScript(scriptData, char2Line);
  if (data.empty()) {
    return Variable::emptyInstance;
  }
  
  ParsingScript script(data);
  script.setChar2Line(char2Line);
  script.setOriginalScript(scriptData);
  Variable result;
  
  while (script.stillValid()) {
    result = Parser::loadAndCalculate(script, Constants::END_PARSING_STR);
    Utils::goToNextStatement(script);
  }
  
  return result;
}

Variable Interpreter::processIf(ParsingScript& script)
{
  size_t startIfCondition = script.getPointer();
  
  Variable result = Parser::loadAndCalculate(script, Constants::END_ARG_STR);
  bool isTrue = result.numValue != 0;
  
  if (isTrue) {
    result = processBlock(script);
    
    if (result.type == Constants::BREAK_STATEMENT ||
        result.type == Constants::CONTINUE_STATEMENT) {
      // Got here from the middle of the if-block. Skip it.
      script.setPointer(startIfCondition);
      skipBlock(script);
    }
    skipRestBlocks(script);
    return result;
  }
  
  // We are in Else. Skip everything in the If statement.
  skipBlock(script);
  
  ParsingScript nextData(script);
  string nextToken = Utils::getNextToken(nextData);
  
  if (Constants::ELSE_IF_LIST.find(nextToken) !=
      Constants::ELSE_IF_LIST.end()) {
    script.setPointer(nextData.getPointer() + 1);
    result = processIf(script);
  }
  if (Constants::ELSE_LIST.find(nextToken) !=
      Constants::ELSE_LIST.end()) {
    script.setPointer(nextData.getPointer() + 1);
    result = processBlock(script);
  }
  
  return Variable::emptyInstance;
}

Variable Interpreter::processFor(ParsingScript& script)
{
  string forString = Utils::getBodyBetween(script, Constants::START_ARG, Constants::END_ARG);
  script.forward();
  
  if (forString.find(Constants::END_STATEMENT) != string::npos) {
    // Looks like: "for(i = 0; i < 10; i++)".
    processCanonicalFor(script, forString);
  } else {
    // Otherwise looks like: "for(item : array)"
    processArrayFor(script, forString);
  }
  
  return Variable::emptyInstance;
}

void Interpreter::processArrayFor(ParsingScript& script, const string& forString)
{
  size_t index = forString.find(Constants::FOR_ANY);
  if (index == string::npos || index == forString.size() - 1) {
    throw ParsingException("Expecting: for(item : array)");
  }
  
  string varName = forString.substr(0, index);
  
  ParsingScript forScript(forString);
  Variable arrayValue = forScript.executeFrom(index + 1);

  size_t cycles = arrayValue.totalElements();
  size_t startForCondition = script.getPointer();
  Variable result;
  
  for (size_t i = 0; i < cycles; i++) {
    script.setPointer(startForCondition);
    Variable& current = arrayValue.getValue(i);
    ParserFunction::addGlobalOrLocalVariable(varName,
                                             new GetVarFunction(current));
    
    result = processBlock(script);
    if (result.isReturn || result.type == Constants::BREAK_STATEMENT) {
      script.setPointer(startForCondition);
      break;
    }
  }
}

void Interpreter::processCanonicalFor(ParsingScript& script, const string& forString)
{
  vector<string> forTokens = Utils::tokenize(forString, string(1, Constants::END_STATEMENT));
  if (forTokens.size() != 3) {
    throw ParsingException("Expecting: for(init; condition; loopStatement)");
  }
  
  ParsingScript initScript(forTokens[0] + Constants::END_STATEMENT);
  ParsingScript condScript(forTokens[1] + Constants::END_STATEMENT);
  ParsingScript loopScript(forTokens[2] + Constants::END_STATEMENT);
  
  initScript.execute();

  size_t startForCondition = script.getPointer();
  int cycles = 0;
  bool stillValid = true;
  Variable result;
  
  while (stillValid) {
    Variable condResult = condScript.executeFrom(0);
    stillValid = condResult.numValue != 0;
    if (!stillValid) {
      break;
    }

    script.setPointer(startForCondition);
    
    // Check for an infinite loop if we are comparing same values:
    if (++cycles >= Constants::MAX_LOOPS) {
      throw ParsingException("Looks like an infinite loop after " +
                             to_string(cycles) + " cycles.");
    }
    
    result = processBlock(script);
    if (result.isReturn || result.type == Constants::BREAK_STATEMENT) {
      script.setPointer(startForCondition);
      skipBlock(script);
      break;
    }
    loopScript.executeFrom(0);
  }
}

Variable Interpreter::processWhile(ParsingScript& script)
{
  size_t startWhileCondition = script.getPointer();
  
  // A check against an infinite loop.
  int cycles = 0;
  bool stillValid = true;
  Variable result;
  
  while (stillValid) {
    script.setPointer(startWhileCondition);
    
    result = Parser::loadAndCalculate(script, Constants::END_ARG_STR);
    bool stillValid = result.numValue != 0;
    
    if (!stillValid) {
      break;
    }
    
    // Check for an infinite loop if we are comparing same values:
    if (++cycles >= Constants::MAX_LOOPS) {
      throw ParsingException("Looks like an infinite loop after " +
                             to_string(cycles) + " cycles.");
    }
    
    result = processBlock(script);
    if (result.isReturn || result.type == Constants::BREAK_STATEMENT) {
      script.setPointer(startWhileCondition);
      break;
    }
  }
  
  // The while condition is not true anymore: must skip the whole while
  // block before continuing with next statements.
  skipBlock(script);
  return Variable::emptyInstance;
}

Variable Interpreter::processTry(ParsingScript& script)
{
  size_t startTryCondition = script.getPointer() - 1;
  size_t currentStackLevel = ParserFunction::getCurrentStackLevel();
  ParsingException exception;
  
  Variable result;
  
  try {
    result = processBlock(script);
  }
  catch(ParsingException& exc) {
    exception = exc;
  }
  
  if (!exception.msg().empty() ||
      result.type == Constants::BREAK_STATEMENT) {
    // Got here from the middle of the try-block either because
    // an exception was thrown or because of a Break. Skip it.
    script.setPointer(startTryCondition);
    skipBlock(script);
  }
  
  string catchToken = Utils::getNextToken(script);
  script.forward(); // skip opening parenthesis
  
  // The next token after the try block must be a catch.
  if (Constants::CATCH_LIST.find(catchToken) ==
      Constants::CATCH_LIST.end()) {
    throw ParsingException("Expecting a '" + *Constants::CATCH_LIST.begin() +
                           "' but got [" + catchToken + "]");
  }
  
  string exceptionName = Utils::getNextToken(script);
  script.forward(); // skip closing parenthesis
  
  if (!exception.msg().empty()) {
    string excStack = ParserFunction::invalidateStacksAfterLevel(currentStackLevel);
    if (!excStack.empty()) {
      excStack = " --> " + exceptionName + excStack;
    }
    
    GetVarFunction* excFunc =
    new GetVarFunction(Variable(exception.msg() + excStack));
    ParserFunction::addGlobalOrLocalVariable(exceptionName, excFunc);
    
 	  result = processBlock(script);
    ParserFunction::popLocalVariable(exceptionName);
  } else {
    skipBlock(script);
  }
  
  skipRestBlocks(script);
  return result;
}

Variable Interpreter::processBlock(ParsingScript& script)
{
  size_t blockStart = script.getPointer();
  Variable result;
  
  while(script.stillValid()) {
    int endGroupRead = Utils::goToNextStatement(script);
    if (endGroupRead > 0) {
      return result;
    }
    if (!script.stillValid()) {
      throw ParsingException("Couldn't process block [" +
                             script.substr(blockStart) + "]");
    }
    
    result = Parser::loadAndCalculate(script, Constants::END_PARSING_STR);
    
    if (result.type == Constants::BREAK_STATEMENT ||
        result.type == Constants::CONTINUE_STATEMENT) {
      return result;
    }
  }
  return result;
}

void Interpreter::skipBlock(ParsingScript& script)
{
  size_t blockStart = script.getPointer();
  int startCount = 0;
  int endCount = 0;
  while (startCount == 0 || startCount > endCount) {
    if (!script.stillValid()) {
      throw ParsingException("Couldn't skip block [" +
                             script.substr(blockStart) + "]");
    }
    char currentChar = script.current();
    script.forward();
    switch (currentChar)
    {
      case Constants::START_GROUP:
        startCount++;
        break;
      case Constants::END_GROUP:
        endCount++;
        break;
    }
  }
  
  if (startCount != endCount) {
    throw ParsingException("Mismatched parentheses");
  }
}

void Interpreter::skipRestBlocks(ParsingScript& script)
{
  while (script.stillValid()) {
    ParsingScript nextData(script);
    string nextToken = Utils::getNextToken(nextData);
    if (Constants::ELSE_IF_LIST.find(nextToken) == Constants::ELSE_IF_LIST.end() &&
        Constants::ELSE_LIST.find(nextToken) == Constants::ELSE_LIST.end()) {
      return;
    }
    script.setPointer(nextData.getPointer());
    skipBlock(script);
  }
}

void Interpreter::readConfig(const string& configFileName)
{
  unordered_map<string, string> tr1;
  unordered_map<string, string> tr2;

  string dictPath;
  unordered_map<string, string> dict;
  string baseLanguage = Constants::ENGLISH;
  string language;
  
  vector<string> lines = Utils::getFileLines(configFileName);
  for (size_t i = 0; i < lines.size(); i++) {
    string line = Utils::trim(lines[i]);
    
    if (line.empty() || line[0] == '#') {
      continue;
    }
    size_t it = line.find("=");
    if (it == string::npos || it == 0 || it == line.size() - 1) {
      // New language data.
      if (!language.empty()) {        
        Translation::setDictionary(baseLanguage, language, tr1);
        Translation::setDictionary(language, baseLanguage, tr2);
      }
      language = Constants::language(line);
      Translation::tryLoadDictionary(dictPath, baseLanguage, language);
      tr1 = Translation::keywordsDictionary(baseLanguage, language);
      tr2 = Translation::keywordsDictionary(language, baseLanguage);
      continue;
    }
    
    string first  = Utils::trim(line.substr(0, it));
    string second = Utils::trim(line.substr(it + 1));
        
    if (first == "dictionariesPath") {
      dictPath = second;
      continue;
    }
    if (first == "errorsPath") {
      Translation::loadErrors(second);
      continue;
    }
    if (first == "language") {
      Translation::setLanguage(second);
      continue;
    }
    
    if (!first.empty() && !second.empty()) {
      Translation::add(first, second, tr1, tr2);
    }
  }
  if (!language.empty()) {
    Translation::setDictionary(baseLanguage, language, tr1);
    Translation::setDictionary(language, baseLanguage, tr2);
  }
}

