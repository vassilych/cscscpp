//
//  UtilsOS.h
//  scripting
//
//  Created by Vassili Kaplan on 21/04/16.
//  Copyright © 2016 Vassili Kaplan. All rights reserved.
//

#ifndef UtilsOS_h
#define UtilsOS_h

#include "Utils.h"
#include <atomic>

class OS
{
public:
  
  enum class NEXT_CMD {
    NONE = 0,
    PREV = -1,
    NEXT = 1,
    TAB  = 2
  };
  enum class SPEC_CHAR {
    LEFT        = -999,
    RIGHT       = -998,
    UP          = -997,
    DOWN        = -996,
    DELETE_CH   = -995,
    BACKSPACE   = -994,
    NEWLINE     = -993,
    UNPRINTABLE = -900
  };
  enum class Color {
    NONE,
    BLACK,
    GRAY,
    GREEN,
    RED,
    WHITE
  };
  
  static void init();
  
  static string getParentDir(const string& dir = "");
  static string homeDir();
  static string pwd();
  static void chdir(const string& dir = "");
  
  static const string DIR_SEP;
  
  static string getFileEntry(string& pathname, size_t i = 0,
                             const string& startsWith = "");
  static void getFiles(string& pathname,
                       vector<string>& files,
                       const string& startsWith = "",
                       const string& endsWith   = Constants::INVALID_CHAR,
                       const string& contains   = Constants::INVALID_CHAR,
                       const string& exact      = Constants::INVALID_CHAR,
                       bool recursive = false,
                       size_t maxEntries = string::npos);
  
  static void appendLine(const string& filename, const string& msg,
                         const string& msg2 = "", const string& msg3 = "",
                         const string& msg4 = "", const string& msg5 = "");
  
  static void writeFile(const string& filename, const string& msg);
  static void tail(const string& filename, size_t maxLines = 10);
  static void touch(const string& filename);
  
  static vector<string> findFiles(const string& search, const string& dir = "");
  static vector<string> ls(const string& dir = "");
  
  static vector<string> findStringInFiles(const vector<string>& files,
                                          const string& search,
                                          bool caseSensitive = true);
  
  static bool pathExists(const string& pathname);
  static bool isDir(const string& pathname);
  static string getFileDetails(const string& dirname,
                               const string& filename);
  
  static void cp(const string& src, const string& dst);
  static void mkdir(const string& dir);
  static void rename(const string& src, const string& dst);
  static void rm(const string& path);
  
  static void clearLine();
  static string getPromptStart();
  static void getPrompt(const string& init = "",
                        const string& prompt = "");
  
  static string getConsoleLine(NEXT_CMD& nextCmd,
                               const string& init = "",
                               bool enhancedMode = true,
                               bool newStatement = true);
  static string getConsoleLine2(NEXT_CMD& nextCmd,
                               const string& init = "",
                               bool enhancedMode = true,
                               bool newStatement = true);
  
  static void redrawLine(const string& prompt, const string& line, size_t pos);
  static void setCursor(const string& prompt, const string& line, size_t pos);
  static void setCursor(size_t pos, size_t& row);
  
  static int getRowDelta();
  
  static void setTerminalCursor(size_t row, size_t column);
  static void getCursorPosition(size_t& row, size_t& column);
  static void getTerminalSize(size_t& rows, size_t& cols);
  
  static bool startsWith(const string& str, const string& pattern);
  static bool endsWith(const string& str, const string& pattern);
  static bool contains(const string& str, const string& pattern);
  static bool match(const string& str, const string& contains,
                    const string& exact, const string& startsWith,
                    const string& endsWith);
  
  // Converts a string to a wide string (utf-8)
  static wstring s2w(const string& str);
  // Converts a wide string (utf-8) to a string
  static string  w2s(const wstring& wstr);
  // Converts a string to a vector of wide chars with the
  // last 2 being explicitely '\0'. Needed for SHFILEOPSTRUCT
  static vector<wchar_t> str2widezz(const string& str);
  
  static string fileExtention(const string& pathname);
  static string toPath(const string& str);
  
  static int getNextChar(size_t timeoutms = string::npos);
  static char getChar(size_t timeoutms = string::npos);
  
  static void more(const string& filename);
  static int runCmd(const string& cmd);
  
  static string getEnv(const string& str = "");
  static void setEnv(const string& name, const string& value);
  
  static string format(const string& str, size_t n);
  static void print(const string& str, bool newLine = false);
  static void printError(const string& str, bool newLine = true);
  static void printColor(Color color, const string& str, bool newLine = false);
  static void printWithBackground(const string& str, bool newLine = false);
  
  static void showCursor(bool show);
  
  static double getCpuTime();
};

class SignalHandler;
class SignalDispatcher {
public:
  
  static void addHandler(SignalHandler* handler);
  static void removeHandler(SignalHandler* handler);
  static void signal(int signum);
  
private:
  static vector<SignalHandler*> s_handlers;
};

class SignalHandler {
public:
  SignalHandler() : m_signaled(false), m_id(++s_id) {
    SignalDispatcher::addHandler(this);
  }
  ~SignalHandler() {
    SignalDispatcher::removeHandler(this);
  }
  
  void set()   { m_signaled = true; }
  void reset() { m_signaled = false; }
  bool isSet() { return m_signaled.load(); }
  int  getId() { return m_id; }
  
private:
  atomic_bool m_signaled;
  int m_id;
  static int s_id;
};


#endif /* UtilsOS_h */
