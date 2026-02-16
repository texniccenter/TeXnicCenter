#include <windows.h>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>

/** TeXnicCenter File Change Watcher for Fast Previews.
* 
*   This utility program watches the current directory
*   for changes to a file called "content2.tex".
*   If changes are detected, then this program quits.
* 
*   Windows' function ReadDirectoryChangesW() is used
*   to wait for changes to this directory, which allows
*   us to use as little CPU as possible while waiting,
*   and respond to a change as quickly as possible.
* 
*   Since content2.tex may have changed since our last run,
*   and therefore ReadDirectoryChangesW() cannot capture that change,
*   we are recording and checking the last modification time
*   of that file in WaitForFileChange.txt.
* 
*   Use this with the following two lines in LaTeX:
*       \immediate\write18{WaitForFileChange}
*       \input{content2}
*   The first line waits until content2.tex has changed and then returns,
*   thereby continuing the compilation. At this point, the compilation
*   should have taken care of all packages and is therefore quick to finish.
* 
*   The preview templates just contain "\input{content}".
*   In fast preview mode, TeXnicCenter writes the above two lines into content.tex
*   and the actual preview code into content2.tex.
*   In regular preview mode, TeXnicCenter writes the preview code directly into content.tex.
* 
*   This way, we can switch between regular and fast preview mode from within the UI
*   and without changing the preview templates.
*/


///Whether a file exists.
bool FileExists(LPCWSTR lpFilename)
{
    DWORD attributes = GetFileAttributesW(lpFilename);
    return (attributes != INVALID_FILE_ATTRIBUTES);
}


/// Retrieves the last modification time of a file.
bool GetLastModificationTime(LPCWSTR lpFilename, FILETIME& ftLastWrite)
{
    //Get a handle on it to check its modification time.
    HANDLE hFile = CreateFileW(
        lpFilename,
        GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        nullptr);

    //The rest of the code should have handled the existence of this file,
    //but we cannot open it.
    if (hFile == INVALID_HANDLE_VALUE) return false;

    if (!GetFileTime(hFile, nullptr, nullptr, &ftLastWrite))
    {
        //std::wcerr << L"GetFileTime failed. Error: " << GetLastError() << std::endl;
        CloseHandle(hFile);
        return false;
    }

    CloseHandle(hFile);
    return true;
}


/// Writes a FILETIME to a text file.
bool WriteFileTimeToFile(LPCWSTR lpFilename, const FILETIME& ft)
{
    std::wofstream file(lpFilename);
    if (!file) return false;
    
    // Write as hex for easy reading
    file << std::hex << ft.dwHighDateTime << L" " << ft.dwLowDateTime;
    return file.good();
}


/// Reads a FILETIME from a text file.
bool ReadFileTimeFromFile(LPCWSTR lpFilename, FILETIME& ft)
{
    std::wifstream file(lpFilename);
    if (!file) return false;
    
    file >> std::hex >> ft.dwHighDateTime >> ft.dwLowDateTime;
    return true;
}


int wmain()
{
    const std::wstring targetFile = L"content2.tex";
    const std::wstring recordFile = L"WaitForFileChange.txt";

    //If the target file does not exist, then we have nothing to watch.
    if (!FileExists(targetFile.c_str())) return 1;

    //Check if we recorded a change time for our target file before.
    bool bFileChanged = true;
    FILETIME ftRecorded{};
    FILETIME ftCurrent{};
    if (ReadFileTimeFromFile(recordFile.c_str(), ftRecorded))
    {
        if (GetLastModificationTime(targetFile.c_str(), ftCurrent))
		{
            //If they are identical, then we saw this version of the file before.
            if (CompareFileTime(&ftCurrent, &ftRecorded) == 0) bFileChanged = false;
        }
    }

    //Exit the program if the file appears changed, or our detection system is not fully setup yet.
    if (bFileChanged)
	{
        // Record current time for next run
        if (GetLastModificationTime(targetFile.c_str(), ftCurrent))
	    {
            WriteFileTimeToFile(recordFile.c_str(), ftCurrent);
        }
		return 0;
    }

    //Go into an infinite watch mode, where Windows will call us if changes to the current directory are made.
    //If a change pertains to our target file, we will exit. Otherwise, we continue watching.

    //Open handle to current directory
    HANDLE hDir = CreateFileW(
        L".",                                  // Watch current directory
        FILE_LIST_DIRECTORY,                   // Directory access
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        nullptr,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS,            // Required for directories
        nullptr
    );

    if (hDir == INVALID_HANDLE_VALUE)
    {
        std::wcerr << L"\nFailed to open directory. Error: "
                   << GetLastError() << std::endl;
        return 1;
    }

    constexpr DWORD bufferSize = 4096;
    BYTE buffer[bufferSize];
    DWORD bytesReturned = 0;

    std::wcout << L"\nWaiting for TeXnicCenter to request a preview by changing "
               << targetFile << L" ..." << std::endl;

    while (true)
    {
        BOOL success = ReadDirectoryChangesW(
            hDir,
            &buffer,
            bufferSize,
            FALSE, // Do not watch subdirectories
            FILE_NOTIFY_CHANGE_LAST_WRITE |
            FILE_NOTIFY_CHANGE_SIZE |
            FILE_NOTIFY_CHANGE_FILE_NAME,
            &bytesReturned,
            nullptr,
            nullptr
        );

        if (!success)
        {
            std::wcerr << L"ReadDirectoryChangesW failed. Error: " << GetLastError() << std::endl;
            CloseHandle(hDir);
            return 1;
        }

        //Check what changed
        FILE_NOTIFY_INFORMATION* pNotify = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(buffer);
        do
        {
            std::wstring fileName(pNotify->FileName, pNotify->FileNameLength / sizeof(WCHAR));

            //Our file got changed?
            if (fileName == targetFile)
            {
                std::wcout << L"Detected change in " << targetFile << L". Generating preview." << std::endl;
                CloseHandle(hDir);

                // Record current time for next run
                //Sleep(50); //Possibly need to sleep, since atomic writes could throw us off.
                if (GetLastModificationTime(targetFile.c_str(), ftCurrent))
	            {
                    WriteFileTimeToFile(recordFile.c_str(), ftCurrent);
                }

                return 0;
            }

            //Any more notifications?
            if (pNotify->NextEntryOffset == 0) break;

            //Go to next notification.
            pNotify = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(reinterpret_cast<BYTE*>(pNotify) + pNotify->NextEntryOffset);

        } while (true);
    }
}
