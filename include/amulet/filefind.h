//
// ------------------------------------------------------------------------
// 		The OpenAmulet User Interface Development Environment
// ------------------------------------------------------------------------
// This code is based on the Amulet project of Carnegie Mellon University,
// and has been placed in the public domain.  If you are using this code
// or any part of OpenAmulet, please contact amulet@cs.cmu.edu to be put
// on the mailing list or have a look at:
// http://www.openip.org
//

// This code was written by Rodolphe Ortalo & Robert M. Muench
// Send comments to: amulet-users@cs.cmu.edu

#ifndef OA_FILEFIND_H
#define OA_FILEFIND_H

#include <iostream>
#include <string>
#include <list>

#ifdef _WIN32
#ifdef _MSC_VER
#pragma warning(disable : 4786)
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

class Am_Filename
{
public:
  // CREATORS
  Am_Filename() : filename() {}
  Am_Filename(const char *in_fname);
  Am_Filename(const std::string &in_fname);
  Am_Filename(const Am_Filename &head, const Am_Filename &tail);
  // OPERATORS
  const char *c_str() const;
  const std::string &name() const;
  bool operator==(const Am_Filename &f) const;
  // ACCESSORS
  bool Is_Directory() const;
  bool Is_Relative() const;
  bool Exists() const;
  void Print(std::ostream &out) const;

private:
  std::string filename;
};

class Am_Data_Locator
{
private:
  typedef std::list<std::pair<Am_Filename, unsigned int>> dir_list_t;

public:
  // CREATORS
  Am_Data_Locator(const Am_Filename &first_dir, const char *envvarname);
  // OPERATORS
  // ACCESSORS
  bool Find_File(const Am_Filename &f, Am_Filename &found) const;
  // MANIPULATORS
  void Add_Search_Directory(const Am_Filename &dir);
  void Remove_Search_Directory(const Am_Filename &dir);
  void Print(std::ostream &out) const;

private:
  dir_list_t search_list;
};

// Default object used by the library
extern Am_Data_Locator Am_Default_Data_Locator;

// Output functions
std::ostream &operator<<(std::ostream &out, const Am_Filename &f);
std::ostream &operator<<(std::ostream &out, const Am_Data_Locator &l);

#endif // OA_FILEFIND_H
