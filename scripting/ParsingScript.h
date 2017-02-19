//
//  ParsingScript.h
//  scripting
//
//  Created by Vassili Kaplan on 13/02/17.
//  Copyright © 2017 Vassili Kaplan. All rights reserved.
//

#ifndef ParsingScript_h
#define ParsingScript_h

#include "Constants.h"
#include "Variable.h"

class ParsingScript
{
public:
  ParsingScript(const string& d, size_t f = 0) :
    m_data(d), m_from(f) {}
  
  ParsingScript(const ParsingScript& other) :
  m_data(other.m_data), m_from(other.m_from),
  m_filename(other.m_filename), m_originalScript(other.m_originalScript),
  m_scriptOffset(other.m_scriptOffset), m_char2Line(other.getChar2Line()) {}
  
  inline size_t size() const           { return m_data.size(); }
  inline bool stillValid() const       { return m_from < m_data.size(); }
  inline size_t getPointer() const     { return m_from; }
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

  inline unordered_map<size_t, size_t>& getChar2Line() const
  {
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
  
  Variable execute(const string& to = Constants::END_PARSING_STR);
  Variable executeFrom(size_t ind, const string& to = Constants::END_PARSING_STR);
  Variable executeAll(const string& to = Constants::END_PARSING_STR);
  
  string getOriginalLine(size_t& lineNumber) const;
  size_t getOriginalLineNumber() const;
 
  template <class K, class V>
  static vector<K> getKeys(const unordered_map<K, V>& m);

private:
  string m_data; // contains the whole script
  size_t m_from; // a pointer to the script
  
  string m_filename;      // filename containing the script
  string m_originalScript;// original raw script
  size_t m_scriptOffset = 0; // used in functiond defined in bigger scripts
  mutable unordered_map<size_t, size_t> m_char2Line;
  
  mutable unordered_map<size_t, size_t>* m_char2LinePtr = nullptr;
};


#endif /* ParsingScript_h */
