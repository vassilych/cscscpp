//
//  Utils.h
//  scripting
//
//  Created by Vassili Kaplan on 29/03/16.
//  Copyright © 2016 Vassili Kaplan. All rights reserved.
//

#ifndef Utils_h
#define Utils_h

#include "Constants.h"
#include "Variable.h"

#include <algorithm>
#include <memory>

class ParsingScript
{
public:
  ParsingScript(const string& d, size_t f = 0) :
    m_data(d), m_from(f) {}

  ParsingScript(const ParsingScript& other) :
    m_data(other.m_data), m_from(other.m_from),
    m_filename(other.m_filename), m_originalScript(other.m_originalScript),
    m_scriptOffset(other.m_scriptOffset), m_char2Line(other.getChar2Line()) {}
  
  inline size_t size() const       { return m_data.size(); }
  inline bool stillValid() const   { return m_from < m_data.size(); }
  inline size_t getPointer() const { return m_from; }
  inline const string& getData() const { return m_data; }
  
  inline size_t find(char ch, size_t fromDelta = 0) const
  { return m_data.find(ch, fromDelta); }
  
  inline size_t find_first_of(const string& str, size_t fromDelta = 0) const
  { return m_data.find_first_of(str, fromDelta); }
  
  inline string substr(size_t from, size_t len = string::npos) const
  { return m_data.substr(from, len); }
  
  inline char operator()(size_t i) const { return m_data[i]; }
  
  inline char current() const     { return m_data[m_from]; }
  inline char currentAndForward() { return m_data[m_from++]; }
  
  inline char tryCurrent() const { return m_from < m_data.size() ?
    m_data[m_from]   : Constants::NULL_CHAR; }
  inline char tryNext() const    { return m_from+1 < m_data.size() ?
    m_data[m_from+1] : Constants::NULL_CHAR; }
  inline char tryPrev() const    { return m_from >= 1 ?
    m_data[m_from-1] : Constants::NULL_CHAR; }
  inline char tryPrevPrev() const    { return m_from >= 2 ?
    m_data[m_from-2] : Constants::NULL_CHAR; }
  
  inline string rest(size_t maxChars = Constants::MAX_CHARS_TO_SHOW) const
  { return m_from < m_data.size() ?
    m_data.substr(m_from, maxChars) : ""; }
  
  //inline void setChar2Line(const unordered_map<size_t, size_t>& char2Line) { m_char2Line = ptr; }
  inline void setChar2Line(const unordered_map<size_t, size_t>& char2Line)
  {
    m_char2Line = char2Line;
  }
  /*inline void setChar2LinePtr(unordered_map<size_t, size_t>* char2LinePtr) const
  {
    m_char2LinePtr = char2LinePtr;
  }*/
  inline unordered_map<size_t, size_t>& getChar2Line() const
  {
    /*if (m_char2LinePtr != nullptr) {
      return *m_char2LinePtr;
    }*/
    return m_char2Line;
  }
  inline void setOffset(size_t offset) { m_scriptOffset = offset; }

  inline void setFilename(const string& filename) { m_filename = filename; }
  inline const string& getFilename() const { return m_filename; }

  inline void setOriginalScript(const string& script) { m_originalScript = script; }
  inline const string& getOriginalScript() const { return m_originalScript; }

  inline void setPointer(size_t ptr)     { m_from = ptr; }
  inline void forward(size_t delta  = 1) { m_from += delta; }
  inline void backward(size_t delta = 1) { if (m_from >= delta) m_from -= delta; }
  
  string getOriginalLine(size_t& lineNumber) const;
  size_t getOriginalLineNumber() const;
  
private:
  string m_data; // contains the whole script
  size_t m_from; // a pointer to the script

  string m_filename;      // filename containing the script
  string m_originalScript;// original raw script
  size_t m_scriptOffset = 0; // used in functiond defined in bigger scripts
  mutable unordered_map<size_t, size_t> m_char2Line;
  
  mutable unordered_map<size_t, size_t>* m_char2LinePtr = nullptr;
  //ParsingScript m_parentScript;
};

class  ParsingException : public exception
{
public:
  ParsingException(const string& err) :
  exception(), m_what(err) {}
  ParsingException() throw() {};
  const char* what()  const throw() { return m_what.c_str(); }
  const string& msg() const throw() { return m_what; }
private:
  string m_what;
};

class Utils
{
public:
  
  static string findStartingToken(const string& data, const vector<string>& items);
  static bool startsWith(const string& base, const string& item);
  
  static bool contains(const string& base, const string& item);
  static bool contains(const string& base, char ch);
  
  static string getValidAction(const ParsingScript& script);
  
  static bool moveForwardIf(ParsingScript& script, char expected,
                            char expected2 = Constants::NULL_CHAR);
  
  static bool moveBackIf(ParsingScript& script, char notExpected);
  static string isNotSign(const string& data);
  
  static bool toBool(double value);
  
  static bool isInt(double x);
  
  static void checkArgsNumber(size_t expected, size_t supplied,
                              const string& name);
  static void checkNotEmpty(const string& varName, const string& funcName);
  static void checkNotEnd(const ParsingScript& script, const string& name);
  static void checkNotNull(const string& varName, const void* func);
  static void checkNumber(const Variable& variable);
  static void checkNonNegNumber(const Variable& variable);
  static void checkInteger(const Variable& variable);
  static void checkNonNegInteger(const Variable& variable);
  
  static vector<string> getFunctionSignature(ParsingScript& script);
  
  static vector<size_t> extractArrayIndices(string& varName);
  static vector<Variable> getArrayIndices(string& varName)
  {  size_t end; return getArrayIndices(varName, end); }
  static vector<Variable> getArrayIndices(string& varName, size_t& end);
  static string extractArrayName(ParsingScript& script);
  
  static string getNextToken(ParsingScript& script);
  
  static int goToNextStatement(ParsingScript& script);
  
  static bool endsWithFunction(const string& buffer, const vector<string>& functions);
  static bool spaceNotNeeded(char next);
  static bool keepSpace(const string& script, char next);
  static bool keepSpaceOnce(const string& script, char next);

  static string convertToScript(const string& source,
                                unordered_map<size_t, size_t>& char2Line);
  
  // Checks whether there is an argument separator (e.g.  ',') before the end of the
  // function call. E.g. returns true for "a,b)" and "a(b,c),d)" and false for "b),c".
  static bool separatorExists(const ParsingScript& script);
  
  static string getFileContents(const string& filename);
  static vector<string> getFileLines(const string& filename);
    
  static Variable getItem(ParsingScript& script);
  static string getToken(ParsingScript& script,
                         const string& to = Constants::END_PARSING_STR);
  
  static vector<Variable> getArgs(ParsingScript& script,
                                  char start, char end, bool& isList);
  static string getBodyBetween(ParsingScript& script,
                               char open, char close);
  
  static void skipRestExpr(ParsingScript& script);
  
  static vector<string> tokenize(const string& data,
                                 const string& delimeters = Constants::NEW_LINE,
                                 size_t from = 0, size_t to = string::npos,
                                 bool removeEmpty = false);
  
  static string toUpper(const string& str);
  
  static string toHex(int i);
  
  static void checkSpecialChars(string const& str);
  
  static void replace(string& str, const string& from, const string& to);
  
  static string trim(string const& str);
  
  static string beautifyScript(const string& script, const string& header);
  
  template <class K, class V>
  static vector<K> getKeys(const unordered_map<K, V>& m);
  
  /*static inline string& ltrim(string &s) {
   s.erase(s.begin(), std::find_if(s.begin(), s.end(), not1(ptr_fun<int, int>(isspace))));
   return s;
   }
   
   static inline string &rtrim(string &s) {
   s.erase(find_if(s.rbegin(), s.rend(), not1(ptr_fun<int, int>(isspace))).base(), s.end());
   return s;
   }
   static inline string &trim(string &s) {
   return ltrim(rtrim(s));
   }*/
};

#endif /* Utils_hpp */
