//
//  Translation.cpp
//  scripting
//
//  Created by Vassili Kaplan on 14/10/16.
//  Copyright © 2016 Vassili Kaplan. All rights reserved.
//

#include "Translation.h"
#include "Parser.h"
#include "UtilsOS.h"

unordered_map<string, unordered_map<string, string>> Translation::s_keywords;
unordered_map<string, unordered_map<string, string>> Translation::s_dictionaries;
unordered_map<string, unordered_map<string, string>> Translation::s_errors;

unordered_map<string, string> Translation::s_spellErrors;
unordered_set<string> Translation::s_nativeWords;
unordered_set<string> Translation::s_tempWords;

string Translation::s_language;

static vector<string> latUp = { "Shch", "_MZn", "_TZn", "Ts", "Ch", "Sh", "Yu", "Ya", "Ye", "Yo", "Zh", "X", "Q",   "W",  "A", "B", "V", "G", "D", "Z", "I", "J", "K", "C", "L", "M", "N", "O", "P", "R", "S", "T", "U", "F", "H", "Y", "E" };
static vector<string> latLo = { "shch", "_mzh", "_tzn", "ts", "ch", "sh", "yu", "ya", "ye", "yo", "zh", "x", "q",   "w",  "a", "b", "v", "g", "d", "z", "i", "j", "k", "c", "l", "m", "n", "o", "p", "r", "s", "t", "u", "f", "h", "y", "e" };
static vector<string> rusUp = { "Щ",    "Ъ",    "Ь",    "Ц",  "Ч",  "Ш",  "Ю",  "Я",  "Е",  "Ё",  "Ж",  "Кс", "Кю", "Уи", "А", "Б", "В", "Г", "Д", "З", "И", "Й", "К", "К", "Л", "М", "Н", "О", "П", "Р", "С", "Т", "У", "Ф", "Х", "Ы", "Э" };
static vector<string> rusLo = { "щ",    "ъ",    "ь",    "ц",  "ч",  "ш",  "ю",  "я",  "е",  "ё",  "ж",  "кс", "кю", "уи", "а", "б", "в", "г", "д", "з", "и", "й", "к", "к", "л", "м", "н", "о", "п", "р", "с", "т", "у", "ф", "х", "ы", "э" };

unordered_map<string, string>& Translation::getDictionary(const string& fromLang,
                                                          const string& toLang,
              unordered_map<string, unordered_map<string, string>>& dictionaries)
{
  string key = fromLang + "-->" + toLang;
  return getDictionary(key, dictionaries);
}

unordered_map<string, string>& Translation::getDictionary(const string& key,
        unordered_map<string, unordered_map<string, string>>& dictionaries)
{
  unordered_map<string, string> result;
  
  auto d = dictionaries.find(key);
  if (d == dictionaries.end()) {
    dictionaries[key] = result;
    return dictionaries[key];
  }
  return d->second;
}

void Translation::setDictionary(const string& fromLang,
                                const string& toLang,
                                const unordered_map<string, string>& dict,
                                bool keywords)
{
  string key = fromLang + "-->" + toLang;
  if (keywords) {
    s_keywords[key] = dict;
  } else {
    s_dictionaries[key] = dict;
  }
  unordered_map<string, string> result;

}

unordered_map<string, string>& Translation::keywordsDictionary(const string& fromLang,
                                                               const string& toLang)
{
  return Translation::getDictionary(fromLang, toLang, s_keywords);
}
unordered_map<string, string>& Translation::translationDictionary(const string& fromLang,
                                                                  const string& toLang)
{
  return Translation::getDictionary(fromLang, toLang, s_dictionaries);
}

unordered_map<string, string>& Translation::errorDictionary(const string& lang)
{
  return Translation::getDictionary(lang, s_dictionaries);
}

void Translation::addNativeKeyword(const string& word)
{
  s_tempWords.insert(word);
  addSpellError(word);
}
void Translation::addTempKeyword(const string& word)
{
  s_tempWords.insert(word);
  addSpellError(word);
}

void Translation::addSpellError(const string& word)
{
  if (word.size() > 2) {
    string key1 = word.substr(0, word.size() - 1);
    s_spellErrors[key1] = word;
    string key2 = word.substr(1);
    s_spellErrors[key2] = word;
  }
}

void Translation::tryLoadDictionary(const string& dirname,
     const string& fromLang, const string& toLang)
{
  if (dirname.empty() || !OS::pathExists(dirname)) {
    return;
  }
  string filename = dirname + fromLang + "_" + toLang + ".txt";
  if (OS::pathExists(filename)) {
    loadDictionary(filename, fromLang, toLang);
  }
  filename = dirname + toLang + "_" + fromLang + ".txt";
  if (OS::pathExists(filename)) {
    loadDictionary(filename, toLang, fromLang);
  }
}

void Translation::loadDictionary(const string& filename,
                                 const string& fromLang, const string& toLang)
{
  unordered_map<string, string>& dict1 = translationDictionary(fromLang, toLang);
  unordered_map<string, string>& dict2 = translationDictionary(toLang, fromLang);
  
  vector<string> lines = Utils::getFileLines(filename);
  for (size_t i = 0; i < lines.size(); i++) {
    vector<string> tokens = Utils::tokenize(lines[i], " ", 0, string::npos, true);
    if (tokens.size() < 2 || tokens[0].empty() || tokens[0][0] == '#') {
      continue;
    }
    string word1 = Utils::trim(tokens[0]);
    string word2 = Utils::trim(tokens[1]);
    dict1[word1] = word2;
    dict2[word2] = word1;
  }
}

void Translation::add(const string& originalName, const string& translation,
                      unordered_map<string, string>& trans1,
                      unordered_map<string, string>& trans2)
{
  if (translation.find_first_of(" \t\r\n") != string::npos) {
    throw ParsingException("Translation of [" + translation +
                           "] contains white spaces");
  }
  
  ParserFunction* originalFunction = ParserFunction::getFunction(originalName);
  if (originalFunction != 0) {
    ParserFunction::addGlobalFunction(translation, originalFunction);
    //cout << "Added [" << translation << "] to the ["
    //     << originalName << "] functions." << endl;
  }
  trans1[originalName] = translation;
  trans2[translation]  = originalName;

  addSpellError(translation);
  
  tryAddToSet(originalName, translation, "functions with space", Constants::FUNCT_WITH_SPACE);
  tryAddToSet(originalName, translation, "functions with space once", Constants::FUNCT_WITH_SPACE_ONCE);
  tryAddToSet(originalName, translation, Constants::CATCH, Constants::CATCH_LIST);
  tryAddToSet(originalName, translation, Constants::ELSE, Constants::ELSE_LIST);
  tryAddToSet(originalName, translation, Constants::ELSE_IF, Constants::ELSE_IF_LIST);
}

template <class T>
bool Translation::tryAddToSet(const string& originalName,
                              const string& translation,
                              const string& listName,
                              T& items)
{
  for (typename T::const_iterator it = items.begin(); it != items.end(); ++it) {
    if (originalName.compare(*it) == 0) {
      items.insert(items.end(), translation);
      s_nativeWords.insert(originalName);
      s_nativeWords.insert(translation);
      //cout << "Added [" << translation << "] to the ["
      //     << listName << "] list." << endl;
      return true;
    }
    
  }
  return false;
}

string& Translation::transliterate(const vector<string>& up1, const vector<string>& up2,
                                   const vector<string>& lo1, const vector<string>& lo2,
                                   string& str)
{
  for (int i = 0; i < up1.size(); i++) {
    Utils::replace(str, up1[i], up2[i]);
    Utils::replace(str, lo1[i], lo2[i]);
  }
  return str;
}

string& Translation::tryTranslit(string fromLang, string toLang,
                                 string& str)
{
  if (fromLang == "Russian") {
    transliterate(rusUp, latUp, rusLo, latLo, str);
  } else if (toLang == "Russian") {
    transliterate(latUp, rusUp, latLo, rusLo, str);
  }
  return str;
}

string Translation::translateScript(const string& script, const string& toLang)
{
  string tempScript = translateScript(script, Constants::ENGLISH, Constants::ENGLISH);
  if (toLang == Constants::ENGLISH) {
    return tempScript;
  }
  
  string result = translateScript(tempScript, Constants::ENGLISH, toLang);
  return result;
}

string Translation::translateScript(const string& script,
                                    const string& fromLang, const string& toLang)
{
  string result;
  string item;
  
  unordered_map<string, string>& keywordsDict = keywordsDictionary(fromLang, toLang);
  unordered_map<string, string>& transDict = translationDictionary(fromLang, toLang);
  bool inQuotes = false;
  
  for (int i = 0; i < script.size(); i++) {
    char ch = script[i];
    inQuotes = ch == Constants::QUOTE ? !inQuotes : inQuotes;
    
    if (inQuotes) {
      result += ch;
      continue;
    }
    if (!Utils::contains(Constants::TOKEN_SEPARATION, ch)) {
      item += ch;
      continue;
    }
    if (!item.empty()) {
      string translation;
      if (toLang == Constants::ENGLISH) {
        ParserFunction* func = ParserFunction::getFunction(item);
        if (func != nullptr) {
          translation = func->getName();
        }
      }
      if (translation.empty()) {
        auto it = keywordsDict.find(item);
        if (it == keywordsDict.end()) {
          it = transDict.find(item);
        }
        translation = it != transDict.end() ? it->second :
                      item;//tryTranslit(fromLang, toLang, item);
      }
      
      result += translation;
      item.clear();
    }
    result += ch;
  }
  
  return result;
}

void Translation::printScript(const string& script)
{
  OS::print(script);
  string item;
  string rest;
  
  bool inQuotes = false;
  
  for (size_t i = 0; i < script.size(); i++) {
    char ch = script[i];
    inQuotes = ch == Constants::QUOTE ? !inQuotes : inQuotes;
    
    if (inQuotes) {
      rest = item + ch;
      item.clear();
      continue;
    }
    if (!Utils::contains(Constants::TOKEN_SEPARATION, ch)) {
      item += ch;
      continue;
    }
    if (!item.empty()) {
      if (!rest.empty()) {
        OS::print(rest, false);
        rest.clear();
      }
      ParserFunction* func = ParserFunction::getFunction(item);
      bool isNative = s_nativeWords.count(item) > 0;
      if (func != nullptr || isNative) {
        OS::Color color;
        color = isNative || func->isNative() ? OS::Color::GREEN :
                func->isGlobal() ? OS::Color::RED :
                OS::Color::GRAY;
        OS::printColor(color, item);
      } else {
        OS::print(item, false);
      }
      item.clear();
    }
    rest += ch;
    //OS::print(string(1, ch));
  }
  OS::print(rest, false);
  OS::print(item, false);
}

void Translation::loadErrors(const string& filename)
{
  if (!OS::pathExists(filename)) {
    return;
  }
  unordered_map<string, string>* dict = &getDictionary(Constants::ENGLISH, s_errors);
  vector<string> lines = Utils::getFileLines(filename);
  for (size_t i = 0; i < lines.size(); i++) {
    string line = Utils::trim(lines[i]);
    if (line.empty() || line[0] == '#') {
      continue;
    }
    size_t it = line.find("=");
    if (it == string::npos || it == 0 || it == line.size() - 1) {
      // New language data.
      dict = &getDictionary(line, s_errors);
      continue;
    }
    
    string first  = Utils::trim(line.substr(0, it));
    string second = Utils::trim(line.substr(it + 1));
    //Utils::replace(second, "{0}", "%s");
    //Utils::replace(second, "{1}", "%s");
    
    unordered_map<string, string>& currDict = *dict;
    currDict[first] = second;
  }
}

string Translation::getErrorString(const string& key)
{
  unordered_map<string, string>& dict = getDictionary(s_language, s_errors);
  auto it = dict.find(key);
  if (it != dict.end()) {
    return it->second;
  }
  
  if (s_language != Constants::ENGLISH) {
    dict = getDictionary(Constants::ENGLISH, s_errors);
    it = dict.find(key);
    if (it != dict.end()) {
      return it->second;
    }
  }
  return key;
}

string Translation::tryFindError(const string& item, const ParsingScript& script)
{
  if (item.size() <= 1) {
    return item;
  }
  string candidate;
  size_t minSize = item.size() > 3 ? 2 : item.size() - 1;
  
  for (size_t i = item.size() - 1; i >= minSize; i--) {
    candidate = item.substr(0, i);
    if (s_nativeWords.count(candidate) > 0) {
      return candidate + " " + Constants::START_ARG;
    }
    if (s_tempWords.count(candidate) > 0) {
      return candidate;
    }
  }
  
  auto it = s_spellErrors.find(item);
  if (it != s_spellErrors.end()) {
    return it->second;
  }
  
  return "";
}

void Translation::throwException(const ParsingScript& script,
                                 const string& excName1,
                                 const string& errorToken,
                                 const string& excName2)
{
  string msg = Translation::getErrorString(excName1);
  Utils::replace(msg, "{0}", errorToken);
  
  if (!errorToken.empty()) {
    string candidate = Translation::tryFindError(errorToken, script);
    
    if (!candidate.empty() && !excName2.empty()) {
      string extra = Translation::getErrorString(excName2);
      Utils::replace(extra, "{0}", candidate);
      msg += " " + extra;
    }
  }
  
  if (!script.getFilename().empty()) {
    string fileMsg = Translation::getErrorString("errorFile");
    Utils::replace(fileMsg, "{0}", script.getFilename());
    msg += Constants::NEW_LINE + fileMsg;
  }
  
  size_t lineNumber = string::npos;
  string line = script.getOriginalLine(lineNumber);
  if (lineNumber != string::npos) {
    string lineMsg = Translation::getErrorString("errorLine");
    Utils::replace(lineMsg, "{0}", to_string(lineNumber + 1));
    Utils::replace(lineMsg, "{1}", Utils::trim(line));
    msg += script.getFilename().empty() ? Constants::NEW_LINE : " ";
    msg += lineMsg;
  }
  throw ParsingException(msg);
}

