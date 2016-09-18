//
// ------------------------------------------------------------------------
// 		The OpenAmulet User Interface Development Environment
// ------------------------------------------------------------------------
// This code is based on the Amulet project of Carnegie Mellon University,
// and has been placed in the public domain.  If you are using this code
// or any part of OpenAmulet, please contact amulet@cs.cmu.edu to be put
// on the mailing list or have a look at:
// http://www.scrap.de/html/amulet.htm
//

// This code was written by Rodolphe Ortalo & Robert M. Muench
// Send comments to: amulet-users@cs.cmu.edu

#include <amulet/filefind.h>
#include <am_config.h> // DEFAULT_AMULET_DIR
#include <algorithm>
#include <cstdlib>
#include <cerrno>

#include <sys/stat.h>
#ifdef _WIN32
// Windows-specific headers
#else
// Unix-specific headers
#include <unistd.h>
#endif

//
// Implementation of the Am_Filename holder class
// (based on the STL std::string class)
//
// Constructors
Am_Filename::Am_Filename(const char *in_fname) : filename(in_fname) {}

Am_Filename::Am_Filename(const std::string in_fname) : filename(in_fname) {}

Am_Filename::Am_Filename(const Am_Filename dir, const Am_Filename name)
    : filename(name.Is_Relative() ? dir.filename + "/" + name.filename
                                  : name.filename) // UNIX-specific
{
}

// Conversion
const char *
Am_Filename::c_str() const
{
  return filename.c_str();
}

std::string
Am_Filename::name() const
{
  return filename;
}

// Operators
bool
Am_Filename::operator==(const Am_Filename &f) const
{
  return (filename == f.filename);
}

// Output method
void
Am_Filename::Print(std::ostream &out) const
{
  out << filename;
}

// Query methods
bool
Am_Filename::Is_Directory() const
{
#ifdef _WIN32
  WIN32_FIND_DATA *mEntryInformation = new WIN32_FIND_DATA;
  HANDLE mDirectoryIterator =
      FindFirstFile(filename.c_str(), mEntryInformation);
  if (mDirectoryIterator != INVALID_HANDLE_VALUE) {
    FindClose(mDirectoryIterator);
  }
  bool res = ((mEntryInformation->dwFileAttributes &
               FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY);
  delete mEntryInformation;
  return res;
#else
  struct stat buf;
  int res = stat(filename.c_str(), &buf); // UNIX-specific
  return ((res == 0) ? (S_ISDIR(buf.st_mode) != 0) : false);
#endif
}

bool
Am_Filename::Is_Relative() const
{
  return ((filename.size() == 0)
              ? false
              : (filename[0] != '/')); // UNIX-specific ? TODO: Check on Win32
}

bool
Am_Filename::Exists() const
{
#ifdef _WIN32
  bool res = true;
  // We need a place to store the information on the file
  WIN32_FIND_DATA *mEntryInformation = new WIN32_FIND_DATA;
  // and we get the information; handle to work with Win32 directory functions
  HANDLE mDirectoryIterator =
      FindFirstFile(filename.c_str(), mEntryInformation);
  if (mDirectoryIterator == INVALID_HANDLE_VALUE) {
    // if it was a file-not-found:
    if (GetLastError() == ERROR_FILE_NOT_FOUND) {
      res = false;
    } else {
      std::cout << "Unknown error in FindFirstFile() !" << std::endl;
      res = false;
    }
  } else {
    // Release the allocated resource
    FindClose(mDirectoryIterator);
  }
  delete mEntryInformation;
  return res;
#else
  struct stat buf;
  int res = stat(filename.c_str(), &buf); // UNIX-specific
  if (res != 0) {
    if (errno == ENOENT)
      return false;
    std::cout << "Unknown error in stat() !" << std::endl;
    return false; // Just to be conservative
  }
  return true;
#endif
}

//
// Implementation of the Am_Data_Locator search class
// for Unix.
//
#ifdef _WIN32
static const char envvar_separator = ';';
#else
static const char envvar_separator = ':';
#endif
Am_Data_Locator::Am_Data_Locator(const Am_Filename first_dir,
                                 const char *envvarname)
{
  // Initially: a directory and the content of an environement variable
  Add_Search_Directory(first_dir);
  if (envvarname != (0L)) {
    char *envvarp = getenv(envvarname); // UNIX-specific ?? TODO: Check on Win32
    if (envvarp != (0L)) {
      const std::string envvar(envvarp);
      std::string::size_type pos = 0;
      bool loop = true;
      while (loop) {
        const std::string::size_type ppos = envvar.find(envvar_separator, pos);
        if (ppos == std::string::npos) {
          Add_Search_Directory(envvar.substr(pos));
          loop = false;
        } else {
          Add_Search_Directory(envvar.substr(pos, (ppos - pos)));
          pos = ppos + 1;
        }
      }
    }
  }
}

bool
Am_Data_Locator::Find_File(const Am_Filename f, Am_Filename &found) const
{
  if (!(f.Is_Relative()) && f.Exists()) // Checks the filename itself
  {
    found = f;
    return true;
  } else // Try to complete the name with some search directory
  {
    for (dir_list_t::const_iterator it = search_list.begin();
         it != search_list.end(); ++it) {
      const Am_Filename fullname((*it).first, f);
      if (fullname.Exists()) {
        found = fullname;
        return true;
      }
    }
  }
  return false;
}

class search_pred
{
public:
  search_pred(const Am_Filename dir) : dirname(dir) { ; }
  ~search_pred() { ; }
  bool operator()(const std::pair<Am_Filename, unsigned int> &v) const
  {
    return (v.first == dirname);
  }

private:
  const Am_Filename dirname;
};

void
Am_Data_Locator::Add_Search_Directory(const Am_Filename dir)
{
  if (dir.Is_Directory()) {
    dir_list_t::iterator it =
        std::find_if(search_list.begin(), search_list.end(), search_pred(dir));
    if (it == search_list.end())
      // Put the directory in first place on search list (with a count of 1)
      search_list.push_front(std::pair<Am_Filename, unsigned int>(dir, 1));
    else
      // Increments the reference count of that directory
      ++((*it).second);
  } else
    std::cout << "Am_Data_Locator: search path " << dir
              << " is not a directory!" << std::endl;
  return;
}

void
Am_Data_Locator::Remove_Search_Directory(const Am_Filename dir)
{
  dir_list_t::iterator it =
      std::find_if(search_list.begin(), search_list.end(), search_pred(dir));
  if (it != search_list.end()) {
    // Decreases the reference count of the directory
    --((*it).second);
    if ((*it).second == 0)
      search_list.erase(it); // and even deletes it if we reach 0
  } else
    std::cout << "Am_Data_Locator: removing search path " << dir
              << " absent from search list" << std::endl;
  return;
}

class search_dir_out
{
public:
  search_dir_out(std::ostream &in_os) : os(in_os) { ; }
  ~search_dir_out() { ; }
  bool operator()(const std::pair<Am_Filename, unsigned int> &v) const
  {
    os << v.first << " [" << v.second << " ref.]" << std::endl;
    return (true);
  }

private:
  std::ostream &os;
};

void
Am_Data_Locator::Print(std::ostream &out) const
{
  std::for_each(search_list.begin(), search_list.end(), search_dir_out(out));
}

// Default object
Am_Data_Locator Am_Default_Data_Locator(DEFAULT_AMULET_DIR, "AMULET_DIR");

//
// Output functions to print the various objects
//
// stream operators
std::ostream &
operator<<(std::ostream &out, const Am_Filename &f)
{
  f.Print(out);
  return out;
}

std::ostream &
operator<<(std::ostream &out, const Am_Data_Locator &l)
{
  l.Print(out);
  return out;
}
