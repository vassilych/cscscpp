//
//  UtilsOS.cpp
//  scripting
//
//  Created by Vassili Kaplan on 21/04/16.
//  Copyright © 2016 Vassili Kaplan. All rights reserved.
//

#include "Constants.h"
#include "UtilsOS.h"
#include "Utils.h"

#include <chrono>
#include <codecvt>
#include <csignal>
#include <cstdio>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <thread>
#include <time.h>
#include <vector>

#ifdef _WIN32
#include <conio.h>
#include <direct.h>
#include <io.h>
#include <shlwapi.h>
#include <Userenv.h>
#include <windows.h>
#include <Winuser.h>
#include <Wincon.h>
#include <Shellapi.h>
#include <sys/utime.h>
#define GetCurrentDir _getcwd

const string OS::DIR_SEP = "\\";

#else
#include <copyfile.h>
#include <curses.h>
#include <dirent.h>
#include <grp.h>
#include <pwd.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <term.h>
#include <time.h>
#include <unistd.h>
#include <utime.h>

#define GetCurrentDir getcwd
const string ANSI_COLOR_BLACK   = "\x1b[30m";
const string ANSI_COLOR_RED     = "\x1b[31m";
const string ANSI_COLOR_GREEN   = "\x1b[32m";
const string ANSI_COLOR_YELLOW  = "\x1b[33m";
const string ANSI_COLOR_BLUE    = "\x1b[34m";
const string ANSI_COLOR_MAGENTA = "\x1b[35m";
const string ANSI_COLOR_CYAN    = "\x1b[36m";
const string ANSI_COLOR_GRAY    = "\x1b[37m";
const string ANSI_COLOR_WHITE   = "\x1b[37;1m";
const string ANSI_COLOR_BG      = "\x1b[7m";
const string ANSI_COLOR_BOLD    = "\x1b[1m";
const string ANSI_COLOR_RESET   = "\x1b[0m";

const string OS::DIR_SEP = "/";
#endif

vector<SignalHandler*> SignalDispatcher::s_handlers;
int SignalHandler::s_id = 0;

extern char **environ;


void OS::init()
{
#ifdef _WIN32
  _setmode(_fileno(stdout), _O_U8TEXT);
#else
  
#endif
  
  ::signal(SIGINT, SignalDispatcher::signal);
}

string OS::getEnv(const string& str)
{
  if (str.empty()) {
    // return all env vars
    ostringstream result;
    char* s = *environ;
    for (size_t i = 0; s; i++) {
      result << s << endl;
      s = *(environ + i);
    }
    return result.str();
  }
  
#ifdef _WIN32
  char *pValue;
  size_t len;
  _dupenv_s(&pValue, &len, str.c_str());
  string result(pValue == nullptr ? "" : pValue);
  free(pValue);
  return result;
#else
  const char* res = getenv(str.c_str());
  return res == nullptr ? "" : res;
#endif
}

void OS::setEnv(const string& name, const string& value)
{
#ifdef _WIN32
  _putenv_s(name.c_str(), value.c_str());
#else
  setenv(name.c_str(), value.c_str(), true);
#endif
}

string OS::homeDir()
{
  const char *homedir;
#ifdef _WIN32
  if ((homedir = getenv("USERPROFILE")) == NULL &&
      (homedir = getenv("HOMEPATH")) == NULL) {
    homedir = "C:";
  }
#else
  if ((homedir = getenv("HOME")) == NULL) {
    homedir = getpwuid(getuid())->pw_dir;
  }
#endif
  return homedir;
}

string OS::pwd()
{
  char cCurrentPath[FILENAME_MAX];
  
  if (!GetCurrentDir(cCurrentPath, sizeof(cCurrentPath)))
  {
    return "";
  }
  
  return cCurrentPath;
}

void OS::chdir(const string& dir)
{
  string dirCopy = dir.empty() ? homeDir() : dir;
  int code = ::chdir(dirCopy.c_str());
  if (code != 0) {
    throw ParsingException("Couldn't change to directory [" + dir + "]");
  }
}

string OS::getParentDir(const string& dir)
{
  string dirCopy = dir.empty() ? pwd() : dir;
  if (dirCopy.empty()) {
    throw ParsingException("Couldn't get current directory");
  }
  
  vector<string> splitPath = Utils::tokenize(dirCopy, "/\\");
  size_t lastSize = splitPath.back().size();
  dirCopy = dirCopy.substr(0, dirCopy.size() - lastSize - 1);
  if (dirCopy.empty()) {
#ifdef _WIN32
    dirCopy = dir;
#else
    dirCopy = "/";
#endif
  }
  
  return dirCopy;
}
wstring OS::s2w(const string& str)
{
  if (str.empty()) {
    return L"";
  }
  wstring_convert<codecvt_utf8<wchar_t>, wchar_t> converter;
  try {
    wstring dest = converter.from_bytes(str);
    return dest;
  } catch (...) {
    return L"";
  }
  
}
string OS::w2s(const wstring& wstr)
{
  if (wstr.empty()) {
    return "";
  }
  wstring_convert<codecvt_utf8<wchar_t>, wchar_t> converter;
  try {
    string dest = converter.to_bytes(wstr);
    return dest;
  } catch (...) {
    return "";
  }
}
vector<wchar_t> OS::str2widezz(const string& str)
{
  wstring wstr = s2w(str);
  
  vector<wchar_t> arr(wstr.size() + 2, L'\0');
  wchar_t* start = arr.data();
  wcsncpy(start, wstr.c_str(), wstr.size());
  
  return arr;
}

vector<string> OS::ls(const string& pathname)
{
  string startsWith;
  string dirname(pathname);
  if (!dirname.empty()) {
    if (dirname.back() == '*')
    {
      size_t it = dirname.find_last_of("/\\");
      if (it != string::npos) {
        startsWith = dirname.size() <= it + 2 ? "" :
        dirname.substr(it + 1, dirname.size() - it - 2);
        dirname = dirname.substr(0, it);
      }
      else {
        startsWith = dirname.size() <= 1 ? "" :
        dirname.substr(0, dirname.size() - 1);
        dirname = "";
      }
    }
  }
  vector<string> results;
  getFiles(dirname, results, startsWith);
  for (size_t i = 0; i < results.size(); i++) {
    results[i] = getFileDetails(dirname, results[i]);
  }
  
  return results;
}

vector<string> OS::findFiles(const string& search, const string& dir)
{
  string contains   = Constants::INVALID_CHAR;
  string exact      = Constants::INVALID_CHAR;
  string startsWith = Constants::INVALID_CHAR;
  string endsWith   = Constants::INVALID_CHAR;
  
  if (search.size() > 2 && search.back() == '*' && search.front() == '*') {
    contains = search.substr(1, search.size() - 2);
  }
  else if (search.back() == '*') {
    startsWith = search.substr(0, search.size() - 1);
  }
  else if (search.front() == '*') {
    endsWith = search.substr(1, search.size() - 1);
  }
  else {
    exact = search;
  }
  
  string path(dir);
  if (path.empty()) {
    path = OS::pwd();
  }
  
  vector<string> results;
  OS::getFiles(path, results, startsWith, endsWith, contains, exact, true /*recursive*/);
  
  return results;
}

vector<string> OS::findStringInFiles(const vector<string>& files,
                                     const string& search,
                                     bool caseSensitive)
{
  string pattern = caseSensitive ? search : Utils::toUpper(search);
  vector<string> results;
  
  for (size_t i = 0; i < files.size(); i++) {
    const string& filename = files[i];
    vector<string> linesFile = Utils::getFileLines(filename);
    
    for (size_t j = 0; j < linesFile.size(); j++) {
      string line = caseSensitive ? linesFile[j] : Utils::toUpper(linesFile[j]);
      
      if (line.find(pattern) != string::npos) {
        string res = filename + " " + to_string(j + 1) + ": " + linesFile[j];
        results.emplace_back(res);
      }
    }
    
  }
  
  return results;
}

void OS::appendLine(const string& filename, const string& msg,
                    const string& msg2, const string& msg3,
                    const string& msg4, const string& msg5)
{
  ofstream file(filename, std::ios_base::app | std::ios_base::out);
  if (!file.is_open()) {
    throw ParsingException("Couldn't write to [" + filename + "]");
  }
  file << msg << msg2 << msg3 << msg4 << msg5 << endl;
  file.close();
}

void OS::writeFile(const string& filename, const string& msg)
{
  ofstream file(filename);
  if (!file.is_open()) {
    throw ParsingException("Couldn't write to [" + filename + "]");
  }
  file << msg << endl;
  file.close();
}

void OS::setTerminalCursor(size_t row, size_t column)
{
#ifdef _WIN32
  CONSOLE_SCREEN_BUFFER_INFOEX csbi;
  csbi.dwCursorPosition.X = column;
  csbi.dwCursorPosition.Y = row;
  BOOL success = SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE),
                                          csbi.dwCursorPosition);
#else
  printf("\33[%zu;%zuH", row, column + 1);
#endif
}

int OS::getRowDelta()
{
#ifdef _WIN32
  size_t row, column;
  OS::getCursorPosition(row, column);
  return row;
#else
  return 0;
#endif
}

void OS::getCursorPosition(size_t& row, size_t& column)
{
#ifdef _WIN32
  CONSOLE_SCREEN_BUFFER_INFO csbi;
  GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
  column = csbi.dwCursorPosition.X;
  row    = csbi.dwCursorPosition.Y;
#else
  /*string strX, strY, all;
   bool collectingX = false;
   bool collectingY = false;
   write(fileno(stdin), "\33[6n", 4);
   // The output looks like: xx[30;21R ...
   for (size_t i = 0; ; i++) {
   char ch = OS::getChar(50);
   all += ch;
   //if (ch == EOF) { break; }
   if (ch == '[') {
   collectingY = true;
   continue;
   }
   if (ch == ';') {
   collectingX = true;
   collectingY = false;
   continue;
   }
   if (ch == 'R') {
   break;
   }
   
   if (collectingX) {
   strX += ch;
   } else if (collectingY) {
   strY += ch;
   }
   //printf("read %c %d %x\n", ch, ch, ch);
   }
   row = atoi(strY.c_str());
   column = atoi(strX.c_str());
   appendLine("tmp.txt", "-getCursorPosition2 row="+to_string(row), ",col="+to_string(column));
   */
  
  OS::getTerminalSize(row, column);
  column = 0; // the column is not important
#endif
}

void OS::getTerminalSize(size_t& rows, size_t& cols)
{
#ifdef _WIN32
  CONSOLE_SCREEN_BUFFER_INFO csbi;
  GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
  rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
  cols = csbi.srWindow.Right - csbi.srWindow.Left + 1;
#else
  struct winsize size;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &size);
  rows = size.ws_row;
  cols = size.ws_col;
#endif
}

void OS::more(const string& filename)
{
  size_t currLine = 0;
  size_t i = 0;
  size_t rows, cols;
  char ch;
  SignalHandler doMore;
  
  size_t lineDelta = OS::getRowDelta();
  
  vector<string> allLines = Utils::getFileLines(filename);
  while (true) {
    OS::getTerminalSize(rows, cols);
    size_t maxLines = rows - 1; // extra line for the message below
    int restLines = (int)allLines.size() - (int)currLine;
    
    if (currLine > 0) {
      size_t delta = maxLines > restLines ? (maxLines - restLines) : 0;
      currLine = delta < currLine ? currLine - delta : currLine;
      OS::setTerminalCursor(lineDelta, 0);
    }
    for (i = currLine; i < allLines.size() && i - currLine < maxLines; i++) {
      if (currLine > 0) {
        OS::clearLine();
      }
      OS::print(allLines[i], true);
    }
    
    if (i == allLines.size()) {
      return;
    }
    
    size_t percent = 100 * i / allLines.size();
    OS::printWithBackground(to_string(percent) +
                        "% *** Press <space> to continue, 'b' to go back, or any key to exit ***");
    
    ch = EOF;
    static int timeoutms = 50;
    while(ch == EOF) {
      ch = OS::getChar(timeoutms);
      if (doMore.isSet()) { // Ctrl-C pressed
        return;
      }
    }
    
    if (ch == 'b' || ch == 'B') {
      OS::setTerminalCursor(lineDelta, 0);
      currLine = currLine > maxLines ? currLine - maxLines : 0;
      continue;
    }
    if (ch != ' ') {
      OS::showCursor(true);
      return;
    }
    
    currLine = i;
  }
}

void OS::tail(const string& filename, size_t maxLines)
{
  // TODO: This is more efficient, but lacks windows wide chars
  /*ifstream file(filename.c_str(), ios::in | ios::binary);
   if (!file) {
   string pwd = OS::pwd();
   throw ParsingException("Couldn't open [" + filename + "] in " + pwd);
   }
   
   static const size_t bufSize = 256;
   char buffer[bufSize + 1];
   size_t linesRead = 0;
   
   file.seekg(-1, ios_base::end);
   size_t currPos = file.tellg();
   
   while (currPos > 0 && linesRead < maxLines) {
   currPos = currPos <= bufSize ? 0 : currPos - bufSize;
   file.seekg(currPos);
   
   file.read(buffer, bufSize);
   string chunk(buffer);
   linesRead += count(chunk.begin(), chunk.end(), '\n');
   }*/
  
  vector<string> allLines = Utils::getFileLines(filename);
  size_t from = allLines.size() > maxLines ? allLines.size() - maxLines : 0;
  for (size_t i = from; i < allLines.size(); i++) {
    print(allLines[i], true);
  }
}

void OS::touch(const string& filename)
{
  if (!pathExists(filename)) {
    ofstream file(filename, std::ios_base::app | std::ios_base::out);
    if (!file.is_open()) {
      throw ParsingException("Couldn't touch [" + filename + "]");
    }
    file << "";
    file.close();
    return;
  }
  
  bool success = !utime(filename.c_str(), 0);
  if (!success) {
    throw ParsingException("Couldn't touch [" + filename + "]");
  }
  
}

int OS::runCmd(const string& cmd)
{
  int status = system(cmd.c_str());
  return status;
}

string OS::getFileEntry(string& dir,
                        size_t i,
                        const string& startsWith)
{
  vector<string> results;
  try {
    getFiles(dir, results, startsWith,
             Constants::INVALID_CHAR,
             Constants::INVALID_CHAR,
             Constants::INVALID_CHAR,
             false, i + 1);
  }
  catch (exception&) {
    return "";
  }
  if (results.empty()) {
    return "";
  }
  i = i % results.size();
  
  string pathname = results[i];
  if (results.size() == 1) {
    pathname += isDir(dir + pathname) ? DIR_SEP : " ";
  }
  return pathname;
}

void OS::getFiles(string& pathname,
                  vector<string>& files,
                  const  string&  startsWith,
                  const  string&  endsWith,
                  const  string&  contains,
                  const  string&  exact,
                  bool   recursive,
                  size_t maxEntries)
{
  string pathCopy = pathname.empty() ? pwd() : pathname;
  if (pathCopy.empty()) {
    throw ParsingException("Couldn't get working directory");
  }
  
  if (!pathExists(pathCopy)) {
    throw ParsingException("Pathname [" + pathCopy + "] doesn't exist");
  }
  if (!isDir(pathCopy)) {
    size_t it = pathCopy.find_last_of("/\\");
    string filename;
    if (it != string::npos) {
      filename = pathname.size() <= it + 1 ? "" :
      pathname.substr(it + 1, pathname.size() - it - 1);
      pathname = pathname.substr(0, it);
    }
    else {
      filename = pathname;
      pathname = "";
    }
    files.emplace_back(filename);
    return;
  }
  
#ifdef _WIN32
  WIN32_FIND_DATA ffd;
  LARGE_INTEGER filesize;
  TCHAR szDir[MAX_PATH];
  size_t length_of_arg;
  HANDLE hFind = INVALID_HANDLE_VALUE;
  DWORD dwError = 0;
  
  if (pathCopy.back() != '*') {
    pathCopy += pathCopy.back() == '/' ||
    pathCopy.back() == '\\' ? "*" : "\\*";
  }
  
  wstring wpath = OS::s2w(pathCopy);
  hFind = FindFirstFile(wpath.c_str(), &ffd);
  
  do {
    string filename(OS::w2s(ffd.cFileName));
    if (maxEntries != string::npos &&
        (!filename.compare("..") || !filename.compare("."))) {
      continue;
    }
    if (OS::match(filename, contains, exact, startsWith, endsWith)) {
      if (recursive) {
        getFiles(filename, files, startsWith, endsWith, contains, exact, recursive, maxEntries);
      }
      else {
        files.emplace_back(filename);
      }
      if (files.size() >= maxEntries) {
        break;
      }
    }
  } while (FindNextFile(hFind, &ffd) != 0);
  
#else
  DIR* directory = opendir(pathCopy.c_str());
  if (directory == 0) {
    throw ParsingException("Couldn't open directory [" + pathCopy + "]");
  }
  
  struct dirent* ent;
  while ((ent = readdir(directory)) != 0) {
    string filename(ent->d_name);
    if (maxEntries != string::npos &&
        (!filename.compare("..") || !filename.compare("."))) {
      continue;
    }
    if (OS::match(filename, contains, exact, startsWith, endsWith)) {
      if (recursive) {
        getFiles(filename, files, startsWith, endsWith, contains, exact, recursive, maxEntries);
      }
      else {
        files.emplace_back(filename);
      }
      if (files.size() >= maxEntries) {
        break;
      }
    }
  }
  
  closedir(directory);
#endif
}

bool OS::pathExists(const string& pathname)
{
  struct stat st;
  return stat(pathname.c_str(), &st) == 0;
}

bool OS::isDir(const string& pathname)
{
  struct stat st;
  if (stat(pathname.c_str(), &st) != 0) {
    return false;
  }
  return (st.st_mode & S_IFDIR) > 0;
}

string OS::fileExtention(const string& filename)
{
  auto idx = filename.rfind('.');
  return (idx != string::npos) ? filename.substr(idx + 1) : "";
}

string OS::getFileDetails(const string& dirname,
                          const string& filename)
{
  string pathname = dirname.empty() ? filename :
  dirname + DIR_SEP + filename;
  string result;
  struct stat st;
  if (stat(pathname.c_str(), &st) == 0) {
#ifdef _WIN32
    result += st.st_mode &  S_IFDIR ? "d" :
              st.st_mode & _S_IFIFO ? "p" :
              st.st_mode &  S_IFCHR ? "c" : "-";
    
    bool write = _access(pathname.c_str(), 2) == 0 ||
                 _access(pathname.c_str(), 6) == 0;
    bool read =  _access(pathname.c_str(), 4) == 0 ||
                 _access(pathname.c_str(), 6) == 0;
    //string ext = OS::fileExtention(pathname);
    bool exec = ((st.st_mode & S_IEXEC) != 0);
    result += read  ? "r" : "-";
    result += write ? "w" : "-";
    result += exec  ? "x" : "-";
    result += " ";
    result += format(to_string(st.st_nlink), 3);
    result += " ";
    
#else
    result +=  S_ISDIR(st.st_mode) ? "d" :
              S_ISBLK(st.st_mode)  ? "b" :
              S_ISCHR(st.st_mode)  ? "c" :
              S_ISSOCK(st.st_mode) ? "s" :
              S_ISFIFO(st.st_mode) ? "p" :
               S_ISLNK(st.st_mode) ? "l" : "-";
    result += st.st_mode & S_IRUSR ? "r" : "-";
    result += st.st_mode & S_IWUSR ? "w" : "-";
    result += st.st_mode & S_IXUSR ? "x" : "-";
    result += st.st_mode & S_IRGRP ? "r" : "-";
    result += st.st_mode & S_IWGRP ? "w" : "-";
    result += st.st_mode & S_IXGRP ? "x" : "-";
    result += st.st_mode & S_IROTH ? "r" : "-";
    result += st.st_mode & S_IWOTH ? "w" : "-";
    result += st.st_mode & S_IXOTH ? "x" : "-";
    
    //result += st.st_mode & S_ISVTX ? "@" : "-";
    result += " ";
    
    result += format(to_string(st.st_nlink), 3);
    result += " ";
    
    struct passwd *pw = getpwuid(st.st_uid);
    struct group  *gr = getgrgid(st.st_gid);
    result += format(pw ? pw->pw_name : " ", 13);
    result += " ";
    result += format(gr ? gr->gr_name : " ", 12);
    
#endif
    result += " ";
    result += format(to_string(st.st_size), 11) + " ";
    
    struct tm * timeinfo = localtime(&st.st_mtime);
    string modDate = asctime(timeinfo);
    // Fri Apr 22 01:35:27 2016
    modDate = modDate.substr(4, 7) + modDate.substr(20, 4) + modDate.substr(10, 6);
    result += modDate + " ";
  }
  
  result += filename;
  return result;
}

string OS::toPath(const string& str)
{
#ifdef _WIN32
  wstring wstr = OS::s2w(str);
  
  TCHAR  buffer[Constants::MAX_VAR_SIZE] = TEXT("");
  TCHAR  buf[Constants::MAX_VAR_SIZE] = TEXT("");
  TCHAR** lppPart = { NULL };
  
  bool retval = GetFullPathName(wstr.c_str(),
                                Constants::MAX_VAR_SIZE, buffer, lppPart);
  
  if (!retval) {
    return str;
  }
  string result = OS::w2s(buffer);
  return result;
#else
  return str;
#endif
}

void OS::cp(const string& src, const string& dst)
{
  string srcPath = OS::toPath(src);
  string dstPath = OS::toPath(dst);
#ifdef _WIN32
  vector<wchar_t> srcv = OS::str2widezz(srcPath);
  vector<wchar_t> dstv = OS::str2widezz(dstPath);
  
  SHFILEOPSTRUCT s = { 0 };
  s.wFunc = FO_COPY;
  s.fFlags = FOF_NO_UI;
  s.pFrom = srcv.data();
  s.pTo = dstv.data();
  int status = SHFileOperation(&s);
#else
  int status = ::copyfile(srcPath.c_str(), dstPath.c_str(),
                          NULL, COPYFILE_ALL | COPYFILE_RECURSIVE);
#endif
  if (status != 0) {
    throw ParsingException("Couldn't copy path [" +
                           srcPath + "] --> [" + dstPath + "]");
  }
}

void OS::mkdir(const string& dir)
{
  string dirPath = OS::toPath(dir);
#ifdef _WIN32
  wstring dirw = OS::s2w(dirPath);
  int status = !CreateDirectory(dirw.c_str(), NULL) &&
  GetLastError() != ERROR_ALREADY_EXISTS;
#else
  int status = ::mkdir(dirPath.c_str(), S_IRWXU | S_IRWXG | S_IRWXO);
#endif
  if (status != 0) {
    throw ParsingException("Couldn't make directory [" + dirPath + "]");
  }
}

void OS::rename(const string& src, const string& dst)
{
  string srcPath = OS::toPath(src);
  string dstPath = OS::toPath(dst);
#ifdef _WIN32
  vector<wchar_t> srcv = OS::str2widezz(srcPath);
  vector<wchar_t> dstv = OS::str2widezz(dstPath);
  
  SHFILEOPSTRUCT s = { 0 };
  s.wFunc = FO_RENAME;
  s.fFlags = FOF_NO_UI;
  s.pFrom = srcv.data();
  s.pTo = dstv.data();
  int status = SHFileOperation(&s);
#else
  int status = ::rename(srcPath.c_str(), dstPath.c_str());
#endif
  if (status != 0) {
    throw ParsingException("Couldn't rename path [" +
                           srcPath + "] --> [" + dstPath + "]");
  }
}

void OS::rm(const string& path)
{
  string rmPath = OS::toPath(path);
#ifdef _WIN32
  vector<wchar_t> pathv = OS::str2widezz(rmPath);
  
  SHFILEOPSTRUCT s = { 0 };
  s.wFunc = FO_DELETE;
  s.fFlags = FOF_NO_UI;
  s.pFrom = pathv.data();
  int status = SHFileOperation(&s);
#else
  int status = 0;
  DIR *directory;
  struct dirent *ent;
  if ((directory = opendir(rmPath.c_str())) != NULL) {
    while ((ent = readdir(directory)) != NULL) {
      if (!strcmp(ent->d_name, "..") || !strcmp(ent->d_name, ".")) {
        continue;
      }
      string son = rmPath + "/" + ent->d_name;
      rm(son);
    }
    closedir(directory);
    status = rmdir(rmPath.c_str());
  }
  else {
    status = std::remove(rmPath.c_str());
  }
#endif
  if (status != 0) {
    throw ParsingException("Couldn't remove [" + rmPath + "]");
  }
}

void OS::print(const string& str, bool newLine)
{
  showCursor(true);
#ifdef _WIN32
  if (!str.empty()) {
    wstring wstr = s2w(str);
    wcout << wstr;
  }
  if (newLine) wcout << endl;
#else
  cout << str;
  if (newLine) cout << endl;
#endif
}

void OS::printColor(OS::Color color, const string& str, bool newLine)
{
  showCursor(true);
#ifdef _WIN32
  HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
  CONSOLE_SCREEN_BUFFER_INFO   csbi;
  GetConsoleScreenBufferInfo(console, &csbi);
  WORD currAttr = csbi.wAttributes;
  
  WORD fgcolor = FOREGROUND_BLACK;
  switch(color) {
    case OS::Color::BLACK: fgcolor = FOREGROUND_BLACK;
      break;
    case OS::Color::GRAY:  fgcolor = FOREGROUND_GRAY;
      break;
    case OS::Color::GREEN: fgcolor = FOREGROUND_GREEN;
      break;
    case OS::Color::RED:   fgcolor = FOREGROUND_RED;
      break;
    case OS::Color::WHITE: fgcolor = FOREGROUND_WHITE;
      break;
    default: break;
  }
  SetConsoleTextAttribute(console, fgcolor);
  wstring wstr = s2w(str);
  wcout << wstr;
  SetConsoleTextAttribute(console, currAttr);
  if (newLine) wcout << endl;
#else
  string fgcolor;
  switch(color) {
    case OS::Color::BLACK: fgcolor = ANSI_COLOR_BLACK;
      break;
    case OS::Color::GRAY:  fgcolor = ANSI_COLOR_GRAY;
      break;
    case OS::Color::GREEN: fgcolor = ANSI_COLOR_GREEN;
      break;
    case OS::Color::RED:   fgcolor = ANSI_COLOR_RED;
      break;
    case OS::Color::WHITE: fgcolor = ANSI_COLOR_WHITE;
      break;
    default: break;
  }
  cout << fgcolor << str << ANSI_COLOR_RESET;
  if (newLine) cout << endl;
#endif
  
}

void OS::printError(const string& str, bool newLine)
{
  OS::printColor(OS::Color::RED, str, newLine);
}

void OS::printWithBackground(const string& str, bool newLine)
{
#ifdef _WIN32
  HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
  CONSOLE_SCREEN_BUFFER_INFO   csbi;
  GetConsoleScreenBufferInfo(console, &csbi);
  WORD currAttr = csbi.wAttributes;
  
  SetConsoleTextAttribute(console, FOREGROUND_RED | BACKGROUND_GREEN);
  wstring wstr = s2w(str);
  wcout << wstr;
  SetConsoleTextAttribute(console, currAttr);
  if (newLine) wcout << endl;
#else
  cout << ANSI_COLOR_BG << ANSI_COLOR_GREEN << str << ANSI_COLOR_RESET;
  if (newLine) cout << endl;
#endif
  showCursor(newLine);
}

void OS::showCursor(bool show)
{
#ifdef _WIN32
  HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
  CONSOLE_CURSOR_INFO     cursorInfo;
  
  GetConsoleCursorInfo(console, &cursorInfo);
  cursorInfo.bVisible = show;
  SetConsoleCursorInfo(console, &cursorInfo);
#else
  string cmd = show ? "\33[?25h" : "\33[?25l";
  cout << cmd << flush;
#endif
}

#ifdef _WIN32
#else
bool kbhit(int msec)
{
  struct timeval tv;
  fd_set fds;
  tv.tv_sec = 0;
  tv.tv_usec = msec * 1000;
  FD_ZERO(&fds);
  FD_SET(0, &fds); //STDIN_FILENO is 0
  select(1, &fds, NULL, NULL, &tv);
  
  bool done = FD_ISSET(0, &fds);
  return done;
}
#endif

char OS::getChar(size_t timeoutms)
{
  char ch = EOF;
  chrono::steady_clock::time_point begin = chrono::steady_clock::now();

#ifdef _WIN32
  static size_t waitms = 10;
  size_t elapsed = 0;

  HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
  DWORD mode = 0;
  GetConsoleMode(hStdin, &mode);
  SetConsoleMode(hStdin, mode & (~ENABLE_ECHO_INPUT));
  
  while ((ch = getch()) == EOF &&
         (timeoutms == string::npos ||
          (elapsed += waitms) < timeoutms)) {
           this_thread::sleep_for(chrono::milliseconds(waitms));
         }
  
  SetConsoleMode(hStdin, mode);
#else
  struct termios oldt, newt;
  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  newt.c_cc[VMIN] = 1;
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);

  while(true) {
    usleep(1);
    if (kbhit(0))
    {
      ch = fgetc(stdin);
      if (ch != EOF)
        break;
    }
    if (timeoutms > 0) {
      chrono::steady_clock::time_point end= chrono::steady_clock::now();
      auto diff = chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
      if (diff >= timeoutms) {
        break;
      }
    }
  }
 
  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
#endif
  return ch;
}

int OS::getNextChar(size_t timeoutms)
{
  static size_t waitms = 10;
  size_t elapsed = 0;
  char ch;
#ifdef _WIN32
  HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
  DWORD mode = 0;
  GetConsoleMode(hStdin, &mode);
  SetConsoleMode(hStdin, mode & (~ENABLE_ECHO_INPUT));
  
  while ((ch = getch()) == EOF &&
         (timeoutms == string::npos ||
          (elapsed += waitms) < timeoutms)) {
           this_thread::sleep_for(chrono::milliseconds(waitms));
  }
  
  SetConsoleMode(hStdin, mode);
  if (ch == -32) { // escape before left, right, delete...
    ch = getch();
    if (ch == 72) { // arrow up
      return (int)SPEC_CHAR::UP;
    }
    if (ch == 80) { // arrow down
      return (int)SPEC_CHAR::DOWN;
    }
    if (ch == 77) { // arrow right
      return (int)SPEC_CHAR::RIGHT;
    }
    if (ch == 75) { // arrow left
      return (int)SPEC_CHAR::LEFT;
    }
    if (ch == 83) { // forward (normal) delete
      return (int)SPEC_CHAR::DELETE_CH;
    }
  }
  if (ch == 8) { // backspace delete
    return (int)SPEC_CHAR::BACKSPACE;
  }
  if (ch == '\r') { // enter
    return (int)SPEC_CHAR::NEWLINE;
  }
#else
  struct termios oldt, newt;
  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  
  while ((ch = getchar()) == EOF &&
         (timeoutms == string::npos ||
          (elapsed += waitms) < timeoutms)) {
           this_thread::sleep_for(chrono::milliseconds(waitms));
  }
  
  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  if (ch == 27 || ch == -17) { // escape before left, right, delete...
    ch = getchar();
    ch = getchar();
    //printf(" Char: %c %d %x\n", ch, ch, ch);
    if (ch == 65 || ch == -128) { // arrow up
      return (int)SPEC_CHAR::UP;
    }
    if (ch == 66 || ch == -127) { // arrow down
      return (int)SPEC_CHAR::DOWN;
    }
    if (ch == 67 || ch == -125) { // arrow right
      return (int)SPEC_CHAR::RIGHT;
    }
    if (ch == 68 || ch == -126) { // arrow left
      return (int)SPEC_CHAR::LEFT;
    }
    if (ch == 51 || ch == -88) { // forward delete
      return (int)SPEC_CHAR::DELETE_CH;
    }
  }
  if (ch == 127) { // backspace delete
    return (int)SPEC_CHAR::BACKSPACE;
  }
  if (ch == '\n') { // enter
    return (int)SPEC_CHAR::NEWLINE;
  }
#endif

  if (ch == 126) { // tilde
    return (int)SPEC_CHAR::UNPRINTABLE;
  }
  return ch;
}

string OS::getPromptStart()
{
  string pwd = OS::pwd();
  if (pwd.size() > Constants::MAX_PROMPT_SIZE + 1) {
    pwd = "..." + pwd.substr(pwd.size() - Constants::MAX_PROMPT_SIZE);
  }
  
  return pwd + ">>";
}

void OS::clearLine()
{
#ifdef _WIN32
  size_t rows, cols;
  getTerminalSize(rows, cols);
  wcout << L"\r" << wstring(cols - 1, ' ') << L"\r";
#else
  cout << "\33[2K\r";
#endif
}

void OS::getPrompt(const string& init, const string& prompt)
{
  clearLine();
#ifdef _WIN32
  wcout << OS::s2w(prompt) << OS::s2w(init) << flush;
#else
  cout << prompt << init << flush;
#endif
}

void OS::redrawLine(const string& prompt, const string& line, size_t pos)
{
  size_t row = string::npos;
  setCursor(prompt.size(), row);
  size_t rows, cols;
  getTerminalSize(rows, cols);
  size_t toClean = max<int>(0, (int)cols - (int)prompt.size() - 1);
#ifdef _WIN32
  wcout << wstring(toClean, ' ');
  setCursor(prompt.size(), row);
  wcout << OS::s2w(line);
#else
  cout << string(toClean, ' ');
  setCursor(prompt.size(), row);
  cout << line;
  
#endif
  setCursor(prompt.size() + pos, row);
}

void OS::setCursor(const string& prompt, const string& line, size_t pos)
{
  clearLine();
#ifdef _WIN32
  wcout << OS::s2w(prompt) << OS::s2w(line);
  wcout << L"\r" << OS::s2w(prompt) << OS::s2w(line.substr(0, pos));
#else
  cout << prompt << line;
  cout << "\r" << prompt << line.substr(0, pos);
#endif
}

void OS::setCursor(size_t pos, size_t& row)
{
  size_t column = 0;
  if (row == string::npos) {
    OS::getCursorPosition(row, column/*unused*/);
  }
  if (row != string::npos) {
    OS::setTerminalCursor(row, pos);
    //appendLine("tmp.txt", "-setCursor row=,", to_string(row), ", pos: ", to_string(pos));
  }
}

string OS::getConsoleLine(NEXT_CMD& nextCmd, const string& init,
                          bool enhancedMode, bool newStatement)
{
  string line = init;
  nextCmd = NEXT_CMD::NONE;
  
  string prompt = newStatement ? getPromptStart() : "";
  if (!line.empty()) {
    redrawLine(prompt, line, line.size());
  } else {
    getPrompt(line, prompt);
  }
  
  if (!enhancedMode) {
    getline(cin, line);
    return line;
  }
  int delta = (int)init.size() - 1;
  
  while (true) {
    int ch = OS::getNextChar();
    if (ch == (int)SPEC_CHAR::NEWLINE) {
      break;
    }
    
    if (ch == (int)SPEC_CHAR::UP) {
      nextCmd = NEXT_CMD::PREV;
      return line;
    } else if (ch == (int)SPEC_CHAR::DOWN) {
      nextCmd = NEXT_CMD::NEXT;
      return line;
    } else if (ch == (int)SPEC_CHAR::RIGHT) {
      delta = max(-1, min(delta + 1, (int)line.size() - 1));
      redrawLine(prompt, line, delta + 1);
    } else if (ch == (int)SPEC_CHAR::LEFT) {
      delta = max(-1, min(delta - 1, (int)line.size() - 1));
      redrawLine(prompt, line, delta + 1);
    } else if (ch == (int)SPEC_CHAR::DELETE_CH) {
      if (!line.empty()) {
        line.erase(delta + 1, 1);
        redrawLine(prompt, line, max(0, delta + 1));
      }
    } else if (ch == (int)SPEC_CHAR::BACKSPACE) {
      if (!line.empty()) {
        delta = max(-1, min(delta - 1, (int)line.size() - 2));
        line.erase(delta + 1, 1);
        redrawLine(prompt, line, max(0, delta + 1));
      }
    }

    if (ch <= (int)SPEC_CHAR::UNPRINTABLE) {
      continue;
    }

    if (ch == '\t') { // tab
      nextCmd = NEXT_CMD::TAB;
      return line;
    }
    
    ++delta;
    if (delta < line.size()) {
      delta = max(0, min(delta, (int)line.size() - 1));
      line.insert(delta, 1, ch);
    }
    else {
      line += ch;
    }
    redrawLine(prompt, line, delta + 1);
  }
  
  OS::print("", true /* new line */);
  return line;
}

string OS::getConsoleLine2(NEXT_CMD& nextCmd, const string& init,
                          bool enhancedMode, bool newStatement)
{
  string line = init;
  nextCmd = NEXT_CMD::NONE;
  
  string prompt = newStatement ? getPromptStart() : "";
  if (!line.empty()) {
    redrawLine(prompt, line, line.size());
  } else {
    getPrompt(line, prompt);
  }
  
  if (!enhancedMode) {
    getline(cin, line);
    return line;
  }
  int delta = (int)init.size() - 1;
  
#ifdef _WIN32
  while (true) {
    char ch = OS::getChar();
    if (ch == '\r') {
      break;
    }
    
    if (ch == -32) { // escape before left, right, delete...
      ch = getch();
      if (ch == 72) { // arrow up
        nextCmd = NEXT_CMD::PREV;
        return line;
      }
      if (ch == 80) { // arrow down
        nextCmd = NEXT_CMD::NEXT;
        return line;
      }
      if (ch == 77) { // arrow right
        delta = max(-1, min(delta + 1, (int)line.size() - 1));
        redrawLine(prompt, line, delta + 1);
      }
      if (ch == 75) { // arrow left
        delta = max(-1, min(delta - 1, (int)line.size() - 1));
        redrawLine(prompt, line, delta + 1);
      }
      if (ch == 83) { // forward (normal) delete
        if (!line.empty()) {
          line.erase(delta + 1, 1);
          redrawLine(prompt, line, max(0, delta + 1));
        }
      }
      continue;
    }
    if (ch == 8) { // backspace delete
      if (!line.empty()) {
        delta = max(-1, min(delta - 1, (int)line.size() - 2));
        line.erase(delta + 1, 1);
        redrawLine(prompt, line, max(0, delta + 1));
      }
      continue;
    }
    if (ch == 126) { // tilde
      continue;
    }
    if (ch == '\t') { // tab
      nextCmd = NEXT_CMD::TAB;
      return line;
    }
    
    ++delta;
    if (delta < line.size()) {
      delta = max(0, min(delta, (int)line.size() - 1));
      line.insert(delta, 1, ch);
    }
    else {
      line += ch;
    }
    redrawLine(prompt, line, delta + 1);
  }
  
  wcout << endl;
#else
  while (true) {
    char ch = OS::getChar();
    if (ch == '\n') {
      break;
    }
    
    if (ch == 27 || ch == -17) { // escape before left, right, delete...
      ch = getchar();
      ch = getchar();
      //printf(" Char: %c %d %x\n", ch, ch, ch);
      if (ch == 65 || ch == -128) { // arrow up
        nextCmd = NEXT_CMD::PREV;
        return line;
      }
      if (ch == 66 || ch == -127) { // arrow down
        nextCmd = NEXT_CMD::NEXT;
        return line;
      }
      if (ch == 67 || ch == -125) { // arrow right
        delta = max(-1, min(++delta, (int)line.size() - 1));
        redrawLine(prompt, line, delta + 1);
      }
      if (ch == 68 || ch == -126) { // arrow left
        delta = max(-1, min(--delta, (int)line.size() - 1));
        redrawLine(prompt, line, delta + 1);
      }
      if (ch == 51 || ch == -88) { // forward (normal) delete
        if (!line.empty()) {
          line.erase(delta + 1, 1);
          redrawLine(prompt, line, max(0, delta + 1));
        }
      }
      continue;
    }
    if (ch == 127) { // backspace delete
      if (!line.empty()) {
        delta = max(-1, min(--delta, (int)line.size() - 2));
        line.erase(delta + 1, 1);
        redrawLine(prompt, line, max(0, delta + 1));
      }
      continue;
    }
    if (ch == 126) { // tilde
      continue;
    }
    if (ch == '\t') { // tab
      nextCmd = NEXT_CMD::TAB;
      return line;
    }
    
    ++delta;
    if (delta < line.size()) {
      delta = max(0, min(delta, (int)line.size() - 1));
      line.insert(delta, 1, ch);
    }
    else {
      line += ch;
    }
    redrawLine(prompt, line, delta + 1);
  }
  
  cout << endl;
#endif
  return line;
}

double OS::getCpuTime()
{
  double result = 0;
#ifdef _WIN32
  FILETIME a,b,c,d;
  if (GetProcessTimes(GetCurrentProcess(), &a, &b, &c, &d) != 0) {
    result =
    (double)(d.dwLowDateTime |
             ((unsigned long long)d.dwHighDateTime << 32)) * 0.0000001;
  }
#else
  result = (double)clock() / (double)CLOCKS_PER_SEC;
#endif
  return result;
}

bool OS::match(const string& str, const string& contains,
               const string& exact, const string& startsWith,
               const string& endsWith)
{
  bool result = OS::startsWith(str, startsWith) ||
  OS::endsWith(str, endsWith) ||
  OS::contains(str, contains) ||
  str.compare(exact) == 0;
  
  return result;
}

bool OS::contains(const string& str, const string& pattern)
{
  return pattern.empty() || str.find(pattern) != string::npos;
}

bool OS::startsWith(const string& str, const string& pattern)
{
  return pattern.empty() ||
  strncmp(str.c_str(), pattern.c_str(), pattern.size()) == 0;
}

bool OS::endsWith(const string& str, const string& pattern)
{
  if (pattern.empty()) {
    return true;
  }
  if (pattern.size() > str.size()) {
    return false;
  }
  string matching = str.substr(str.size() - pattern.size(), pattern.size());
  return strncmp(matching.c_str(), pattern.c_str(), pattern.size()) == 0;
}

string OS::format(const string& str, size_t n)
{
  vector<char> buf(n + 1);
  std::snprintf(&buf[0], buf.size(), "%*s", (int)n, str.c_str());
  
  string result(buf.begin(), buf.end());
  return result;
}

void SignalDispatcher::addHandler(SignalHandler* handler)
{
  s_handlers.push_back(handler);
}

void SignalDispatcher::removeHandler(SignalHandler* handler)
{
  for (auto it = s_handlers.begin(); it != s_handlers.end(); ++it) {
    if ((*it)->getId() == handler->getId()) {
      s_handlers.erase(it);
      //cout << "Removed " << handler->getId() << endl;
      return;
    }
  }
}

void SignalDispatcher::signal(int signum)
{
  for (int i = 0; i < s_handlers.size(); i++) {
    s_handlers[i]->set();
  }
}
