/**
 * @file file_browser.cpp
 * @brief SD Card File Browser Implementation with Serial Debugging
 */

#include "file_browser.h"

FileBrowser::FileBrowser() {
    fileCount = 0;
    scrollOffset = 0;
    selectedIndex = -1;
    currentPath = "/";
}

bool FileBrowser::begin(uint8_t csPin) {
    SerialUSB.print(F("FileBrowser: Attempting SD.begin with CS pin "));
    SerialUSB.println(csPin);
    
    if (!SD.begin(csPin)) {
        SerialUSB.println(F("FileBrowser: SD.begin() returned FALSE"));
        return false;
    }
    
    SerialUSB.println(F("FileBrowser: SD.begin() SUCCESS"));
    SerialUSB.println(F("FileBrowser: Opening root directory..."));
    
    openDirectory("/");
    
    SerialUSB.print(F("FileBrowser: Found "));
    SerialUSB.print(fileCount);
    SerialUSB.println(F(" files/folders"));
    
    return true;
}

void FileBrowser::openDirectory(const char* path) {
    SerialUSB.print(F("FileBrowser: Opening directory: "));
    SerialUSB.println(path);
    
    fileCount = 0;
    scrollOffset = 0;
    selectedIndex = -1;
    currentPath = String(path);
    
    File dir = SD.open(path);
    if (!dir) {
        SerialUSB.println(F("FileBrowser: ERROR - Failed to open directory!"));
        return;
    }
    
    if (!dir.isDirectory()) {
        SerialUSB.println(F("FileBrowser: ERROR - Path is not a directory!"));
        dir.close();
        return;
    }
    
    SerialUSB.println(F("FileBrowser: Directory opened, reading entries..."));
    
    // Read all files
    while (fileCount < MAX_FILES_DISPLAY) {
        File entry = dir.openNextFile();
        if (!entry) {
            SerialUSB.println(F("FileBrowser: No more entries"));
            break;
        }
        
        const char* entryName = entry.name();
        SerialUSB.print(F("  Entry "));
        SerialUSB.print(fileCount);
        SerialUSB.print(F(": "));
        SerialUSB.print(entryName);
        
        strncpy(files[fileCount].name, entryName, FILE_NAME_MAX_LEN - 1);
        files[fileCount].name[FILE_NAME_MAX_LEN - 1] = '\0';
        files[fileCount].isDirectory = entry.isDirectory();
        files[fileCount].size = entry.size();
        
        if (files[fileCount].isDirectory) {
            SerialUSB.println(F(" [DIR]"));
        } else {
            SerialUSB.print(F(" ("));
            SerialUSB.print(files[fileCount].size);
            SerialUSB.println(F(" bytes)"));
        }
        
        entry.close();
        fileCount++;
    }
    
    dir.close();
    
    SerialUSB.print(F("FileBrowser: Total entries loaded: "));
    SerialUSB.println(fileCount);
}

void FileBrowser::goUp() {
    SerialUSB.println(F("FileBrowser: Going up one directory..."));
    
    if (currentPath.length() <= 1) {
        SerialUSB.println(F("FileBrowser: Already at root, cannot go up"));
        return;
    }
    
    int lastSlash = currentPath.lastIndexOf('/');
    if (lastSlash > 0) {
        currentPath = currentPath.substring(0, lastSlash);
    } else {
        currentPath = "/";
    }
    
    SerialUSB.print(F("FileBrowser: New path: "));
    SerialUSB.println(currentPath);
    
    openDirectory(currentPath.c_str());
}

FileEntry* FileBrowser::getFile(int index) {
    if (index < 0 || index >= fileCount) {
        SerialUSB.print(F("FileBrowser: getFile() - Invalid index: "));
        SerialUSB.println(index);
        return nullptr;
    }
    return &files[index];
}

void FileBrowser::scroll(int delta) {
    int oldOffset = scrollOffset;
    scrollOffset += delta;
    if (scrollOffset < 0) scrollOffset = 0;
    
    // Max scroll based on file count
    int maxScroll = fileCount - 5; // Show 5 items at a time
    if (maxScroll < 0) maxScroll = 0;
    if (scrollOffset > maxScroll) scrollOffset = maxScroll;
    
    if (scrollOffset != oldOffset) {
        SerialUSB.print(F("FileBrowser: Scrolled from "));
        SerialUSB.print(oldOffset);
        SerialUSB.print(F(" to "));
        SerialUSB.println(scrollOffset);
    }
}

void FileBrowser::selectFile(int index) {
    if (index < 0 || index >= fileCount) {
        SerialUSB.print(F("FileBrowser: selectFile() - Invalid index: "));
        SerialUSB.println(index);
        return;
    }
    
    selectedIndex = index;
    
    SerialUSB.print(F("FileBrowser: Selected: "));
    SerialUSB.println(files[index].name);
    
    if (files[index].isDirectory) {
        // Navigate into directory
        SerialUSB.println(F("FileBrowser: Entering directory..."));
        String newPath = currentPath;
        if (newPath != "/") newPath += "/";
        newPath += String(files[index].name);
        openDirectory(newPath.c_str());
    } else {
        SerialUSB.println(F("FileBrowser: File selected (not a directory)"));
    }
}