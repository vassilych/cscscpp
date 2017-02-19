//
//  ParsingScript.c
//  scripting
//
//  Created by Vassili Kaplan on 13/02/17.
//  Copyright © 2017 Vassili Kaplan. All rights reserved.
//

#include "ParsingScript.h"

#include "Parser.h"
#include "Utils.h"
#include "UtilsOS.h"
#include "Variable.h"

template <class K, class V>
vector<K> ParsingScript::getKeys(const unordered_map<K, V>& m)
{
  vector<K> keys;
  keys.reserve(m.size());
  for(auto kv : m) {
    keys.push_back(kv.first);
  }
  std::sort(keys.begin(), keys.end());
  
  return keys;
}

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
  vector<size_t> lineStart = getKeys(char2Line);
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

Variable ParsingScript::execute(const string& to)
{
  if (m_data.empty()) {
    return Variable::emptyInstance;
  }
  if (m_data[m_data.size() - 1] != Constants::END_STATEMENT) {
    m_data += Constants::END_STATEMENT;
  }
  Variable result = Parser::loadAndCalculate(*this, to);
  return result;
}

Variable ParsingScript::executeFrom(size_t ind, const string& to)
{
  m_from = ind;
  return execute(to);
}

Variable ParsingScript::executeAll(const string& to)
{
  Variable result;
  while (stillValid()) {
    result = Parser::loadAndCalculate(*this, to);
    Utils::goToNextStatement(*this);
  }
  return result;
}
