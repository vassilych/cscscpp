//
//  Parser.cpp
//  scripting
//
//  Created by Vassili Kaplan on 29/03/16.
//  Copyright © 2016 Vassili Kaplan. All rights reserved.
//

#include "Parser.h"
#include "Variable.h"

#include <algorithm>
#include <stdlib.h>
#include <ctype.h>


Variable Parser::loadAndCalculate(ParsingScript& script,
                                  const string& to)
{
  vector<Variable> listToMerge = split(script, to);
  
  if (listToMerge.empty()) {
    throw ParsingException("Couldn't parse [" +
                           script.rest() + "]");
  }
  
  // If there is just one resulting cell there is no need
  // to perform the second step to merge tokens.
  if (listToMerge.size() == 1) {
    return listToMerge[0];
  }
  
  Variable& baseCell = listToMerge[0];
  size_t index = 1;
  
  // Second step: merge list of cells to get the result of an expression.
  Variable result = merge(baseCell, index, listToMerge);
  return result;
}

vector<Variable> Parser::split(ParsingScript& script,
                               const string& to)
{
  vector<Variable> listToMerge;
  
  if (!script.stillValid() || Utils::contains(to, script.current())) {
    script.forward();
    listToMerge.emplace_back(Variable::emptyInstance);
    return listToMerge;
  }
  
  string parsingItem;
  int negated = 0;
  bool inQuotes = false;
  int indexDepth = 0;
  
  do
  { // Main processing cycle of the first part.
    string negateSymbol = Utils::isNotSign(script.rest());
    if (!negateSymbol.empty()) {
      negated++;
      script.forward(negateSymbol.size());
      continue;
    }
    
    char ch = script.currentAndForward();
    checkQuotesIndices(script, ch, inQuotes, indexDepth);
    
    string action = Constants::EMPTY;
    
    bool keepCollecting = inQuotes || indexDepth > 0 ||
              stillCollecting(script, parsingItem, to, action);
    if (keepCollecting)
    { // The char still belongs to the previous operand.
      parsingItem += ch;
      
      bool goForMore = script.stillValid() &&
        (inQuotes || indexDepth > 0 || !Utils::contains(to, script.current()));
      
      if (goForMore) {
        continue;
      }
    }
    
    checkConsistency(script, parsingItem, listToMerge);
    
    Utils::moveForwardIf(script, Constants::SPACE);
    
    if (action.size() > 1) {
      script.forward(action.size() - 1);
    }
    
    // We are done getting the next token. The getValue() call below may
    // recursively call loadAndCalculate(). This will happen if extracted
    // item is a function or if the next item is starting with a START_ARG '('.
    ParserFunction func(script, parsingItem, ch, action);
    Variable current = func.getValue(script);
    
    if (negated > 0 && current.getType() == Constants::NUMBER) {
      // If there has been a NOT sign, this is a boolean.
      // Use XOR (true if exactly one of the arguments is true).
      bool boolRes = !((negated % 2 == 0) ^ Utils::toBool(current.numValue));
      current = Variable(boolRes);
      negated = 0;
    }
    
    if (action.empty()) {
      action = updateAction(script, to);
    } else {
      Utils::moveForwardIf(script, action[0]);
    }
    
    char next = script.tryCurrent(); // we've already moved forward
    bool done = listToMerge.empty() && (next == Constants::END_STATEMENT ||
        (action == Constants::NULL_ACTION && current.getType() != Constants::NUMBER));
    if (done) {
      // If there is no numerical result, we are not in a math expression.
      if (!action.empty() && action != Constants::END_ARG_STR) {
        throw ParsingException("Action [" +
                               action + "] without an argument.");
      }
      listToMerge.emplace_back(current);
      return listToMerge;
    }
    
    current.action = action;
    updateIfBool(script, current);
    
    listToMerge.emplace_back(current);
    parsingItem.clear();
    
  } while (script.stillValid() &&
          (inQuotes || indexDepth > 0 || !Utils::contains(to, script.current())));
  
  // This happens when called recursively inside of the math expression:
  Utils::moveForwardIf(script, Constants::END_ARG);
  
  return listToMerge;
}

bool Parser::stillCollecting(const ParsingScript& script,
                             const string& item, const string& to, string& action)
{
  char prev = script.tryPrevPrev();
  char ch   = script.tryPrev();
  char next = script.tryCurrent();
  
  if (Utils::contains(to, ch) || ch == Constants::START_ARG ||
                                 ch == Constants::START_GROUP ||
                               next == Constants::NULL_CHAR) {
    return false;
  }
  
  // Case of a negative number, or starting with the closing bracket:
  if (item.empty() &&
     ((ch == '-' && next != '-') || ch == Constants::END_ARRAY
                                 || ch == Constants::END_ARG)) {
    return true;
  }
  
  // Case of a scientific notation 1.2e+5 or 1.2e-5 or 1e5:
  if ((prev == 'E' || prev == 'e') &&
      (ch == '-' || ch == '+' || isdigit(ch)) &&
      item.size() > 1 && isdigit(item[item.size() - 2]))
  {
    return true;
  }
  
  // Otherwise, if it's an action (+, -, *, etc.) or a space
  // we're done collecting current token.
  ParsingScript tempScript(script.getData(), script.getPointer() - 1);
  action = Utils::getValidAction(tempScript);
  
  if (action != Constants::EMPTY ||
     (item.size() > 0 && ch == Constants::SPACE)) {    
    return false;
  }
  
  return true;
}

void Parser::checkQuotesIndices(const ParsingScript& script,
                                char ch, bool& inQuotes, int& indexDepth)
{
  switch (ch)
  {
    case Constants::QUOTE:
    {
      char prev = script.tryPrevPrev();
      inQuotes = prev != '\\' ? !inQuotes : inQuotes;
      return;
    }
    case Constants::START_ARRAY:
    {
      if (!inQuotes) {
        indexDepth++;
      }
      return;
    }
    case Constants::END_ARRAY:
    {
      if (!inQuotes) {
        indexDepth--;
      }
      return;
    }
  }
}

void Parser::checkConsistency(const ParsingScript& script,
                              const string& item,
                              const vector<Variable>& listToMerge)
{
  if (listToMerge.empty()) {
    return;
  }
  
  bool isControl = Constants::CONTROL_FLOW.find(item) !=
  Constants::CONTROL_FLOW.end();
  
  if (isControl) {
    // This can happen when the end of statement ";" is forgotten.
    throw ParsingException ("Token [" +
                            item + "] can't be part of an expression. Check \";\".");
  }
}

void Parser::updateIfBool(ParsingScript& script, Variable& current)
{
  if ((current.action == "&&" && current.numValue == 0) ||
      (current.action == "||" && current.numValue != 0)) {
    // Short circuit evaluation: don't need to evaluate more.
    Utils::skipRestExpr(script);
    current.action = Constants::NULL_ACTION;
  }
}

string Parser::updateAction(ParsingScript& script, const string& to)
{
  // We search a valid action till we get to the End of Argument ')'
  // or pass the end of string.
  if (!script.stillValid() || script.current() == Constants::END_ARG ||
      Utils::contains(to, script.current())) {
    return Constants::NULL_ACTION;
  }
  
  //size_t index = script.getPointer();
  ParsingScript tempScript(script.getData(), script.getPointer());
  string action = Utils::getValidAction(tempScript);
  
  /*while (action.empty() && tempScript.stillValid() &&
         tempScript(index) == Constants::END_ARG) {
    // Look for the next character in string until a valid action is found.
    tempScript.forward();
    action = Utils::getValidAction(tempScript);
  }*/
  
  // We need to advance forward not only the action length but also all
  // the characters we skipped before getting the action.
  size_t advance = action.empty() ? 0 :
    action.size() + (tempScript.getPointer() - script.getPointer());
  script.forward(advance);
  return action.empty() ? Constants::NULL_ACTION : action;
}

Variable Parser::merge(Variable& current, size_t& index,
                       vector<Variable>& listToMerge,
                       bool mergeOneOnly)
{
  while (index < listToMerge.size())
  {
    Variable& next = listToMerge[index++];
    
    while (!current.canMergeWith(next))
    { // If we cannot merge cells yet, go to the next cell and merge
      // next cells first. E.g. if we have 1+2*3, we first merge next
      // cells, i.e. 2*3, getting 6, and then we can merge 1+6.
      merge(next, index, listToMerge, true /* mergeOneOnly */);
    }
    
    current.merge(next);
    if (mergeOneOnly) {
      break;
    }
  }
  
  return current;
}
