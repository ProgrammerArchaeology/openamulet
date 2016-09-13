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

// This code was written by Robert M. Münch
// Send comments to robert.muench@robertmuench.de

#ifndef DIRECTORY_HPP
#define DIRECTORY_HPP

#ifdef _MSC_VER
#pragma warning(disable : 4786)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <list>
#include <map>
#include <iostream>

class Am_Directory
{
public:
  // TYPEDEFS
  // forward declaration
  class mEntry;

  // map containing pointer to mEntry* class for a specific level;
  // this map makes it possible to get all entries in level X without
  // any speed problems
  typedef std::pair<const long, mEntry *> LEVEL_ENTRY;

  //
  typedef std::multimap<long, mEntry *> LEVEL_ENTRIES;

  // Win32 specific; list of file information structs for each file
  // there is one entry in the list
  typedef std::list<WIN32_FIND_DATA *> FILE_ENTRIES;

  typedef std::pair<LEVEL_ENTRIES::iterator, LEVEL_ENTRIES::iterator>
      LEVEL_RANGE_ITERATORS;

  // This class is a directory node. It contains a list of files in the
  // actual directory. The actual directory information is contained in the
  // mOsDirEntry member.
  class mEntry
  {
  public:
    mEntry() : mOsDirEntry(0), mOsParentDirEntry(0), mLevel(-1) { ; }

    explicit mEntry(long aLevel)
        : mOsDirEntry(0), mOsParentDirEntry(0), mLevel(aLevel)
    {
      ;
    }

    ~mEntry()
    {
      // only delete the WIN32_FIND_DATA object, the other
      // pointer is only a reference to an other mEntry object
      // which will be deleted later (or has been deleted already)
      delete (mOsDirEntry);
    }

    // stores the information for the directory this node represents
    WIN32_FIND_DATA *mOsDirEntry;

    // pointer to parent directory this directory might be contained in
    mEntry *mOsParentDirEntry;

    // list of files contained in this directory
    FILE_ENTRIES mFiles;

    // level this directory is in the read tree (depends on the starting
    // point within the directory hierarchy)
    long mLevel;
  };

  // CREATORS
  Am_Directory()
      : mEntryInformation(0), mCurrentDirectoryEntry(0),
        mIncludeDotEntries(false)
  {
  }
  ~Am_Directory()
  {
    delete (mCurrentDirectoryEntry);
    delete (mEntryInformation);

    // free all mEntry objects of mLevel2Directories
    LEVEL_ENTRIES::const_iterator itDirectories;
    for (itDirectories = mLevel2Directories.begin();
         itDirectories != mLevel2Directories.end(); ++itDirectories) {
      // delete all file entry objects
      for (FILE_ENTRIES::const_iterator itFiles =
               (*itDirectories).second->mFiles.begin();
           itFiles != (*itDirectories).second->mFiles.end(); ++itFiles) {
        delete (*itFiles);
      }

      // delete the directory mEntry object
      delete ((*itDirectories).second);
    }
  }

  // MANIPULATORS
  // reads the file & directory entries; reads into aLevel subdirectories
  void readEntries(long aLevel = 0);

  // ACCESSORS
  LEVEL_ENTRIES::size_type getNumberOfDirectories(bool aIncludeDots = false)
  {
    return (mLevel2Directories.size());
  }

  LEVEL_ENTRIES::size_type getNumberOfFiles()
  {
    LEVEL_ENTRIES::size_type numberOfFiles = 0;

    // free all mEntry objects of mLevel2Directories
    LEVEL_ENTRIES::const_iterator itDirectories;
    for (itDirectories = mLevel2Directories.begin();
         itDirectories != mLevel2Directories.end(); ++itDirectories) {
      numberOfFiles += (*itDirectories).second->mFiles.size();
    }

    return (numberOfFiles);
  }

  LEVEL_RANGE_ITERATORS getDirectoryIterators(long aLevel)
  {
    return (mLevel2Directories.equal_range(aLevel));
  }

  std::string getLongPath(mEntry *aEntry);

  void printDirectories(long aHowManyLevels = 1, long aStartLevel = 0);
  void printFiles(long aHowManyLevels = 1, long aStartLevel = 0);

  // DATA MEMBERS

protected:
  // CREATORS
  // MANIPULATORS
  // ACCESSORS
  // DATA MEMBERS

private:
  // CREATORS
  // MANIPULATORS
  // depending on the type (file or directory) the entry gets inserted
  // into the correct container
  void addEntry(WIN32_FIND_DATA *aEntryData, long aLevel);

  // internal function used to build a fully-qualified path name of the
  // current directory or filename, this is needed as we only get the last
  // part of the complete path but have to know the complete path
  //			void	addPathComponent(const std::string &aPath);

  // helper function which just reads all the entries in the directory
  // given by aPattern; this function doesn't perform any sub-directory diving
  void _readEntries(std::string aPattern, long aLevel = 0);

  // ACCESSORS
  // DATA MEMBERS
  // map of level number to directory entry
  // each entry is a pair of the directory level and the mEntry object pointer
  LEVEL_ENTRIES mLevel2Directories;

  // If this is true . and .. will be included when traversing the
  // directory list, otherwise they will be filtered
  bool mIncludeDotEntries;

  // used to store the result of Win32 Directory function calls
  WIN32_FIND_DATA *mEntryInformation;

  // pointer to mEntry object of the parent object
  mEntry *mCurrentDirectoryEntry;

  // handle to work with Win32 directory functions
  HANDLE mDirectoryIterator;
};

#endif
