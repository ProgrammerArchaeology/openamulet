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

#ifdef _MSC_VER
	#pragma warning(disable: 4786)
#endif

#include <iostream>
#include <cstring>
#include <cassert>
#include <list>

#ifndef DIRECTORY_HPP
	#include "amulet/directory.hpp"
#endif

void Am_Directory::addEntry(WIN32_FIND_DATA *aEntryData, long aLevel)
{
	// do we have a hidden entry?
	if((aEntryData->dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) == FILE_ATTRIBUTE_HIDDEN)
	{
		// than skip it (we do this as Win2000 has some strange hidden
		// directories causing a GPF under NT4)
		return;
	}

	// is the parameter a directory?
	if((aEntryData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
	{
		// check if the pattern equals . or ..
		size_t nameLength = strlen(aEntryData->cFileName);
		if( (nameLength >= 1 && strcmp( (aEntryData->cFileName)+nameLength-1, ".") == 0) || (nameLength >= 1 && strcmp( (aEntryData->cFileName)+nameLength-2, "..") == 0))
		{
			// should those entries be included?
			if(mIncludeDotEntries == false)
			{
				// no, than clean the allocated object and return
				delete(aEntryData);
				return;
			}
		}

		mEntry *newDirectoryEntry = new mEntry(aLevel);

		// insert the parent information into the new directory object
		newDirectoryEntry->mOsParentDirEntry = mCurrentDirectoryEntry;

		// insert the OS information into the new directory object
		newDirectoryEntry->mOsDirEntry = aEntryData;

		// yes, than add it to the map of directories
		mLevel2Directories.insert(LEVEL_ENTRY(aLevel,newDirectoryEntry));
	}
	else
	{
		// no, than it's a file, add it to the list of files of the current
		// directory object
		mCurrentDirectoryEntry->mFiles.push_back(aEntryData);
	}

	return;
}

/*
void Am_Directory::addPathComponent(const std::string &aPath)
{
	// we need a copy because we are going to alter the object
	std::string newEntryInformation(aPath);

	// combine the path and entry to form a complete filepath
	std::string::iterator tmp = newEntryInformation.end();
	--tmp;
	newEntryInformation.erase(tmp);
	newEntryInformation.append(mEntryInformation->cFileName);

	// now update the path in the WIN32_FIND_DATA structure
	assert( (newEntryInformation.length() <= MAX_PATH));
	strcpy(mEntryInformation->cFileName, newEntryInformation.c_str());

	return;
}
*/

std::string Am_Directory::getLongPath(mEntry *aEntry)
{
	// get the lowest part of the long path
	std::list<char*> parts;
	parts.push_front(aEntry->mOsDirEntry->cFileName);

	// now walk up till we have no longer a parent
	while(aEntry->mOsParentDirEntry->mOsDirEntry != static_cast<WIN32_FIND_DATA*>(0))
	{
		aEntry = aEntry->mOsParentDirEntry;
		parts.push_front("/");
		parts.push_front(aEntry->mOsDirEntry->cFileName);
	}

	// now build the return value
	std::string longPath;
	for(std::list<char*>::iterator it=parts.begin(); it != parts.end(); ++it)
	{
		longPath += *it;
	}

	return(longPath);
}
 
void Am_Directory::readEntries(long aLevel)
{
	// do we have a top-entry object?
	if(mCurrentDirectoryEntry != static_cast<mEntry*>(0))
	{
		// than the user should first clean out this Am_Directory instance
		return;
	}

	// allocate the top-entry object
	// ATTENTION: The OS specific data is not set correctly, this object
	// is only a container to hold the files in the current directory
	// it's therefore the anchor into the directory tree
	mCurrentDirectoryEntry = new mEntry;

	// and add this anchor to the map of directories; we use level -1
	// for the anchor object
	mLevel2Directories.insert(LEVEL_ENTRY(-1,mCurrentDirectoryEntry));

	// 1st read the current directory == level 0
	_readEntries("*");

	// should directories be read recursively?
	if(aLevel > 0)
	{
		// yes, than read as many levels as requested
		for(long x=0; x<=aLevel; ++x)
		{
			// we need to safe/copy all the found iterators, because the
			// multimap will be altered which invalidates the iterator range
			typedef std::list<LEVEL_ENTRIES::iterator> ITERATOR_LIST;
			ITERATOR_LIST directoriesToScan;

			// select all directories in the current level
			LEVEL_RANGE_ITERATORS selectedRange = mLevel2Directories.equal_range(x);

			// copy each found iterator
			for(LEVEL_ENTRIES::iterator it=selectedRange.first; it != selectedRange.second; ++it)
			{
				directoriesToScan.push_back(it);
			}

			// for each entry read the entries in this directory
			for(ITERATOR_LIST::const_iterator itl=directoriesToScan.begin(); itl != directoriesToScan.end(); ++itl)
			{
				// create the path name
				mCurrentDirectoryEntry = (*(*itl)).second;
				std::string pathToCheck(getLongPath(mCurrentDirectoryEntry));

				// check if the pathname doesn't equal . or ..
				if(pathToCheck.compare(pathToCheck.length()-1, 1, ".") != 0 && pathToCheck.compare(pathToCheck.length()-2, 2, "..") != 0)
				{
					pathToCheck.append("/*");

					_readEntries(pathToCheck, x+1);
				}
			}
		}
	}

	return;
}

void Am_Directory::_readEntries(std::string aPattern, long aLevel)
{
	// check if the pattern equals . or ..
	if( (aPattern.length() >= 1 && aPattern.compare(aPattern.length()-1, 1, ".") == 0) || (aPattern.length() >= 2 && aPattern.compare(aPattern.length()-2, 2, "..") == 0))
	{
		return;
	}

	// allocate a new WIN32_FIND_DATA object
	mEntryInformation = new WIN32_FIND_DATA;

	// we are interested in all entries
	mDirectoryIterator = FindFirstFile(aPattern.c_str(), mEntryInformation);

	addEntry(mEntryInformation, aLevel);

	// now use the HANDLE from FindFirstFile to continue search until all
	// entries are found
	bool continueLoop(true);
	while(continueLoop == true)
	{
		// allocate a new WIN32_FIND_DATA object
		mEntryInformation = new WIN32_FIND_DATA;

		// get next entry and check if we had an error
		if(FindNextFile(mDirectoryIterator, mEntryInformation) == false)
		{
			// Ups, looks like an error but wait
			if(GetLastError() == ERROR_NO_MORE_FILES)
			{
				// we are done
//				std::cout << "Finished reading directory." << std::endl;
			}
			else
			{
				// something else must be failed
				std::cout << "There happend an error while reading the directory." << std::endl;
			}

			// free the last allocated WIN32_FIND_DATA object
			delete(mEntryInformation);
			mEntryInformation = static_cast<WIN32_FIND_DATA*>(0);

			// and exit the while loop
			continueLoop = false;
		}
		else
		{
			addEntry(mEntryInformation, aLevel);
		}
	}

	// close the handle
	FindClose(mDirectoryIterator);
}

void Am_Directory::printDirectories(long aHowManyLevels, long aStartLevel)
{
	std::cout << "Listing Directories: " << getNumberOfDirectories() << std::endl;

	for(int x=-1; x<aHowManyLevels; ++x)
	{
		// get the iterators for this level
		LEVEL_RANGE_ITERATORS selectedRange = mLevel2Directories.equal_range(aStartLevel+x);
		for(LEVEL_ENTRIES::const_iterator it=selectedRange.first; it != selectedRange.second; ++it)
		{
			if((*it).second->mOsDirEntry != static_cast<WIN32_FIND_DATA*>(0))
			{
				std::cout << (*it).second->mOsDirEntry->cFileName;
			}
			else
			{
				std::cout << "Top-Level-Directory";
			}

			std::cout << "\t# of files: " << (*it).second->mFiles.size();
			std::cout << std::endl;
		}

		std::cout << std::endl;
	}

	return;
}

void Am_Directory::printFiles(long aHowManyLevels, long aStartLevel)
{
	std::cout << "Listing found Files: " << getNumberOfFiles() << std::endl;

	for(int x=-1; x<aHowManyLevels; ++x)
	{
		// get the iterators for this level
		LEVEL_RANGE_ITERATORS selectedRange = mLevel2Directories.equal_range(aStartLevel+x);
		for(LEVEL_ENTRIES::const_iterator itDirectories=selectedRange.first; itDirectories != selectedRange.second; ++itDirectories)
		{
			// now print each file stored in this object
			for(FILE_ENTRIES::const_iterator itFiles=(*itDirectories).second->mFiles.begin(); itFiles != (*itDirectories).second->mFiles.end() ; ++itFiles)
			{
				std::cout << (*itFiles)->cFileName << std::endl;
			}
		}

		std::cout << std::endl;
	}

	return;
}
