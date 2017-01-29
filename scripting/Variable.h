//
//  Variable.h
//  scripting
//
//  Created by Vassili Kaplan on 30/03/16.
//  Copyright © 2016 Vassili Kaplan. All rights reserved.
//

#ifndef Variable_h
#define Variable_h

#include "Constants.h"

class Parser;

class Variable
{
public:
    Variable() :
        type(Constants::NONE) {}
    Variable(double val) :
        numValue(val), type(Constants::NUMBER) {}
    Variable(string str) :
        strValue(str), type(Constants::STRING) {}
    Variable(vector<Variable> vec) :
        tuple(vec),    type(Constants::ARRAY) {}
    Variable(Constants::Type tp) :
        type(tp) {}
    
    virtual ~Variable() {}
    
    static Variable duplicate(const Variable* other);
    
    void set(const string& str) { strValue = str; type = Constants::STRING; }
    void set(const double& val) { numValue = val; type = Constants::NUMBER; }
    void set(const vector<Variable>& t) { tuple = t; type = Constants::ARRAY; }

    size_t set(const string& hash, const Variable& var);
    const Variable& get(const string& hash) const;
    
    bool tryGet(const string& hash, Variable& var);
    bool exists(const string& hash) const;
    bool exists(const Variable& indexVar, bool notEmpty = false) const;
    size_t getArrayIndex(const Variable& indexVar) const;
  
    size_t totalElements() const {
      return type == Constants::ARRAY ? tuple.size() : 1;
    }
  
    Variable& getValue(size_t index);
  
    const string& getAction() const { return action; }
    Constants::Type getType() const { return type; }

    string toString() const;
    string toPrint()  const;

    bool canMergeWith(const Variable& right);
    void merge(const Variable& right);
    
    void mergeNumbers(const Variable& right);
    void mergeStrings(const Variable& right);
    
    static Variable emptyInstance;
    
    static int getPriority(const string& action);
    
    template <class T> static double mergeBool(const T& arg1, const T& arg2,
                                               const string& action);

    double numValue = 0.0;
    string strValue;
    vector<Variable> tuple;
    unordered_map<string, size_t> dictionary;
    
    string action;
    string varname;
    Constants::Type type = Constants::NONE;
    bool isReturn = false;
};

#endif /* Variable_h */
