//
//  Translation.h
//  scripting
//
//  Created by Vassili Kaplan on 14/10/16.
//  Copyright © 2016 Vassili Kaplan. All rights reserved.
//

#ifndef Translation_h
#define Translation_h

#include "Utils.h"

class Translation
{
public:
  
  static void add(const string& originalName,
                  const string& translation,
                  unordered_map<string, string>& trans1,
                  unordered_map<string, string>& trans2);
  
  template <class T>
  static bool tryAddToSet(const string& originalName,
                          const string& translation,
                          const string& listName,
                          T& items);
  
  static unordered_map<string, string>& keywordsDictionary(const string& fromLang,
                                                           const string& toLang);
  static unordered_map<string, string>& translationDictionary(const string& fromLang,
                                                              const string& toLang);
  static unordered_map<string, string>& errorDictionary(const string& lang);

  static void setDictionary(const string& fromLang,
                            const string& toLang,
                            const unordered_map<string, string>& dict,
                            bool keywords = true);
  
  static void tryLoadDictionary(const string& dirname,
                                const string& fromLang, const string& toLang);
  
  static string translateScript(const string& script, const string& toLang);
  
  static string translateScript(const string& script,
                                const string& fromLang, const string& toLang);
  
  static string& transliterate(const vector<string>& up1, const vector<string>& up2,
                              const vector<string>& lo1, const vector<string>& lo2,
                              string& str);
  static string& tryTranslit(string fromLang, string toLang,
                            string& str);
  static void printScript(const string& script);
  
  static void addNativeKeyword(const string& word);
  static void addTempKeyword(const string& word);
  
  static void addSpellError(const string& word);

  
  static void setLanguage(const string& lang) { s_language = lang; }
  
  static void loadErrors(const string& filename);
  static string getErrorString(const string& key);
  static string tryFindError(const string& item, const ParsingScript& script);

  static void throwException(const ParsingScript& script,
                             const string& excName1,
                             const string& errorToken = "",
                             const string& excName2 = "");
private:
  
  static string s_language;
  
  static unordered_map<string, string>& getDictionary(const string& fromLang,
                                                      const string& toLang,
      unordered_map<string, unordered_map<string, string>>& dictionaries);

  static unordered_map<string, string>& getDictionary(const string& key,
      unordered_map<string, unordered_map<string, string>>& dictionaries);
  
  static void loadDictionary(const string& filename, const string& fromLang, const string& toLang);

  static unordered_map<string, unordered_map<string, string>> s_keywords;
  static unordered_map<string, unordered_map<string, string>> s_dictionaries;
  static unordered_map<string, unordered_map<string, string>> s_errors;

  static unordered_map<string, string> s_spellErrors;
  static unordered_set<string> s_nativeWords;
  static unordered_set<string> s_tempWords;
};


#endif /* Translation_h */
