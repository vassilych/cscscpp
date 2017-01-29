//
//  Parser.h
//  scripting
//
//  Created by Vassili Kaplan on 29/03/16.
//  Copyright © 2016 Vassili Kaplan. All rights reserved.
//

#ifndef Parser_h
#define Parser_h

#include "Constants.h"
#include "ParserFunction.h"
#include "Utils.h"
#include "Variable.h"

class Parser
{
public:
  
  static Variable loadAndCalculate(ParsingScript& script,
                                   const string& to);
  
private:
  static vector<Variable> split(ParsingScript& script,
                                const string& to);
  
  static void checkConsistency(const ParsingScript& script,
                               const string& item,
                               const vector<Variable>& listToMerge);
  
  static bool stillCollecting(const ParsingScript& script,
                              const string& item, const string& to, string& action);
  
  static void checkQuotesIndices(const ParsingScript& script,
                                 char ch, bool& inQuotes, int& indexDepth);
  
  static Variable merge(Variable& current, size_t& index,
                        vector<Variable>& listToMerge,
                        bool mergeOneOnly = false);
  
  static string updateAction(ParsingScript& script, const string& to);
  
  static void updateIfBool(ParsingScript& script, Variable& current);
  
};

#endif /* Parser_hpp */
