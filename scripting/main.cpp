//
//  main.cpp
//  scripting
//
//  Created by Vassili Kaplan on 29/03/16.
//  Copyright © 2016 Vassili Kaplan. All rights reserved.
//

#include <cmath>
#include <cstdio>
#include <ctime>
#include <fcntl.h>
#include <iostream>
#include <locale>
#include <map>
#include <string>
#include <vector>

#include "Interpreter.h"
#include "Parser.h"
#include "Translation.h"
#include "Utils.h"
#include "UtilsOS.h"

void processScript(const string& script);
void runLoop();

int main(int argc, char* argv[])
{
  OS::init();
  
  try {
    Interpreter::init();
  }
  catch (ParsingException& exc) {
    OS::printError(exc.what(), true);
  }
  ParserFunction::allFunctions();
  
  processScript("include(\"/Users/vk/cscs/scripts/functions.cscs\");");
  string file;
  file = "/Users/vk/Documents/github/cscscpp/bin/Debug/scripts/temp.cscs";

  if (argc > 3 ) {
    string script = Utils::getFileContents(argv[3]);
    string result = Translation::translateScript(script, argv[1], argv[2]);
    cout << result << endl;
    return 0;
  }

  string path = argv[0];
  if (argc > 1 ) {
    file = argv[1];
  }
  
  if (!file.empty()) {
    string script;
    try {
      script = Utils::getFileContents(file);
    }
    catch (exception& exc) {
      OS::printError(exc.what(), true);
      return -1;
    }
    processScript(script);
  } else {
    runLoop();
  }
  
  return 0;
}

void processScript(const string& script)
{
  Variable result;
  try {
    result = Interpreter::process(script);
    if (result.type != Constants::NONE) {
      OS::print(result.toPrint(), true);
    }
  } catch(exception& exc) {
    OS::printError(exc.what(), true);
  }
}

void splitByLast(const string& str, const string& sep, string& a, string& b)
{
  size_t it = str.find_last_of(sep);
  a = it == string::npos ? "" : str.substr(0, it + 1);
  b = it == string::npos ? str : str.substr(it + 1);
}

string completeTab(const string& script, const string& init, size_t& tabFileIndex,
                   string& start, string& base, string& startsWith)
{
  if (tabFileIndex > 0 && script.compare(init) != 0) {
    // The user has changed something in the input field
    tabFileIndex = 0;
  }
  if (tabFileIndex == 0 ||
      (!script.empty() && script.back() == OS::DIR_SEP[0])) {
    // The user pressed tab first time or pressed it on a directory
    string path;
    splitByLast(script, " ", start, path);
    splitByLast(path, "/\\", base, startsWith);
  }
  
  tabFileIndex++;
  string result = OS::getFileEntry(base, tabFileIndex, startsWith);
  result = result.empty() ? startsWith : result;
  return start + base  + result;
}

void runLoop()
{
  vector<string> commands;
  string init;
  int cmdPtr = 0;
  size_t tabFileIndex = 0;
  bool arrowMode = false;
  string start, base, startsWith;
  string script;
  bool newStatement = true;
  
  while (true) {
    OS::NEXT_CMD nextCmd = OS::NEXT_CMD::NONE;
    if (newStatement) {
      script.clear();
    }
    script += OS::getConsoleLine(nextCmd, init,
             true /*enhancedMode*/, newStatement);
    script = Utils::trim(script);
    
    if (nextCmd == OS::NEXT_CMD::TAB) {
      init = completeTab(script, init, tabFileIndex, start, base, startsWith);
      continue;
    } else if (nextCmd == OS::NEXT_CMD::PREV || nextCmd == OS::NEXT_CMD::NEXT) {
      if (arrowMode || nextCmd == OS::NEXT_CMD::NEXT) {
        cmdPtr += (int)nextCmd;
      }
      if (!commands.empty()) {
        cmdPtr = cmdPtr < 0 ? cmdPtr + (int)commands.size() :
        cmdPtr % (int)commands.size();
      }
      init = commands.empty() ? script : commands[cmdPtr];
      arrowMode = true;
      continue;
    }
    
    init.clear();
    tabFileIndex = 0;
    arrowMode = false;
    
    if (script.empty()) {
      continue;
    }
    
    if (script.back() == Constants::NEXT_LINE) {
      script.pop_back();
      newStatement = false;
      continue;
    }
    if (commands.empty() || commands.back().compare(script)) {
      commands.push_back(script);
    }
    if (script.back() != Constants::END_STATEMENT) {
      script += Constants::END_STATEMENT;
    }
    
    processScript(script);
    newStatement = true;
    cmdPtr = (int)commands.size() - 1;
  }
}
