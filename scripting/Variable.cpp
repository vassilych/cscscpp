//
//  Variable.cpp
//  scripting
//
//  Created by Vassili Kaplan on 30/03/16.
//  Copyright © 2016 Vassili Kaplan. All rights reserved.
//
#include <algorithm>

#include "Utils.h"
#include "Variable.h"

Variable Variable::emptyInstance;

Variable Variable::duplicate(const Variable* other)
{
    Variable copy;
    copy.numValue   = other->numValue;
    copy.strValue   = other->strValue;
    copy.tuple      = other->tuple;
    copy.dictionary = other->dictionary;
    copy.action     = other->action;
    copy.varname    = other->varname;
    copy.type       = other->type;
    
    return copy;
}

string Variable::toString() const
{
    if (type == Constants::NONE) {
        return "";
    }
    if (type == Constants::STRING) {
        return strValue;
    }
    if (type == Constants::NUMBER) {
        return Utils::isInt(numValue) ?
            std::to_string((long long)numValue) :
            std::to_string(numValue);
    }
   
    // Otherwise this is a tuple
    string result = "{ ";
    for (size_t i = 0; i < tuple.size(); i++) {
        result += "[" + tuple[i].toString() + "] ";
    }
    result += "}";
    
    return result;
}

string Variable::toPrint() const
{
    if (type != Constants::ARRAY) {
        return toString();
    }
    
    string result = "";
    for (size_t i = 0; i < tuple.size(); i++) {
        result += tuple[i].toString() +
          (i < tuple.size() - 1 ? Constants::NEW_LINE : "");
    }
    
    return result;
}

Variable& Variable::getValue(size_t index)
{
  if (index >= totalElements()) {
    throw ParsingException("There are only [" + to_string(totalElements()) +
                           "] but " + to_string(index) + " requested.");
  }
  if (type == Constants::ARRAY) {
    return tuple[index];
  }
  return *this;
}

size_t Variable::set(const string& hash, const Variable& var)
{
  auto it = dictionary.insert({hash, tuple.size()});
  if (it.second) {
    // Inserted as a new element.
    tuple.emplace_back(var);
    return tuple.size() - 1;
  }
  
  size_t ptr = it.first->second;
  if (ptr < tuple.size() ) {
    // It already exists - reset to what it points to.
    tuple[ptr] = var;
    return ptr;
  }
  
  // Otherwise it points to a non-exiting element - recreate.
  tuple.emplace_back(var);
  it.first->second = tuple.size() - 1;
  return it.first->second;
}

const Variable& Variable::get(const string& hash) const
{
    auto it = dictionary.find(hash);
    size_t ptr = it == dictionary.end() ?
        string::npos : it->second;
    
    if (ptr == string::npos || ptr >= tuple.size()) {
        throw ParsingException("Element [" + hash +
                               "] doesn't exist");
    }
 
    return tuple[ptr];
}

bool Variable::tryGet(const string& hash, Variable& var)
{
    auto it = dictionary.find(hash);
    size_t ptr = it == dictionary.end() ?
        string::npos : it->second;

    if (ptr == string::npos || ptr >= tuple.size()) {
        return false;
    }
    
    var = tuple[ptr];
    return true;
}

size_t Variable::getArrayIndex(const Variable& indexVar) const
{
    if (indexVar.type == Constants::NUMBER) {
        Utils::checkNonNegInteger(indexVar);
        return indexVar.numValue;
    }
    string hash = indexVar.toString();
    auto it = dictionary.find(hash);

    return it == dictionary.end() ?
             string::npos : it->second;
}

bool Variable::exists(const string& hash) const
{
    auto it = dictionary.find(hash);
    return it != dictionary.end();
}

bool Variable::exists(const Variable& indexVar, bool notEmpty) const
{
    if (indexVar.type == Constants::NUMBER) {
        if (indexVar.numValue < 0 ||
            indexVar.numValue >= tuple.size() ||
            indexVar.numValue - floor(indexVar.numValue) != 0.0) {
            return false;
        }
        if (notEmpty) {
          return tuple[(int)indexVar.numValue].getType() != Constants::NONE;
        }
        return true;
    }
    
    string hash = indexVar.toString();
    auto it = dictionary.find(hash);
    return it != dictionary.end();
}

bool Variable::canMergeWith(const Variable& right)
{
    return getPriority(action) >= getPriority(right.getAction());
}

int Variable::getPriority(const string& action)
{
    unordered_map<string, int>::const_iterator it =
        Constants::PRIORITY.find(action);
    
    if (it == Constants::PRIORITY.end()) {
        return 0;
    }
    
    return it->second;
}

void Variable::merge(const Variable& right)
{
    if (type == Constants::STRING ||
        right.getType() == Constants::STRING) {
        mergeStrings(right);
    } else {
        mergeNumbers(right);
    }
    
    action = right.action;
}

void Variable::mergeNumbers(const Variable& right)
{
    double tryBool = mergeBool(numValue, right.numValue, action);
    if (tryBool >= 0) {
        set(tryBool);
        return;
    }
    
    if (action.compare("+") == 0) {
        numValue += right.numValue;
    } else if (action.compare("-") == 0) {
        numValue -= right.numValue;
    } else if (action.compare("*") == 0) {
        numValue *= right.numValue;
    } else if (action.compare("/") == 0) {
        numValue /= right.numValue;
    } else if (action.compare("^") == 0) {
        numValue = pow(numValue, right.numValue);
    } else if (action.compare("%") == 0) {
        numValue = (int)numValue % (int)right.numValue;
    } else if (action.compare("&&") == 0) {
        numValue = numValue && right.numValue;
    } else if (action.compare("||") == 0) {
        numValue = numValue || right.numValue;
    /*} else {
        throw ParsingException("Unknown action [" + action +
                               "] for numbers");*/
    }
}

void Variable::mergeStrings(const Variable& right)
{
    string arg1 = toString();
    string arg2 = right.toString();
    
    double tryBool = mergeBool(arg1, arg2, action);
    if (tryBool >= 0) {
        // A Boolean comparison succeeded
        set(tryBool);
        return;
    }
    
    if (action.compare("+") == 0) {
        set(arg1 + arg2);
        return;
    }
    
    throw ParsingException("Unknown action [" + action +
                           "] for strings");
}

template <class T>
double Variable::mergeBool(const T& arg1, const T& arg2,
                           const string& action)
{
    if (action.compare(">") == 0) {
        return arg1 > arg2;
    } else if (action.compare("<") == 0) {
        return arg1 < arg2;
    } else if (action.compare(">=") == 0) {
        return arg1 >= arg2;
    } else if (action.compare("<=") == 0) {
        return arg1 <= arg2;
    } else if (action.compare("==") == 0) {
        return arg1 == arg2;
    } else if (action.compare("!=") == 0) {
        return arg1 != arg2;
    }
    
    return -1.0;
}
