/**
 * @file file_browser.cpp
 * @brief SD Card File Browser Implementation
 */

#include "file_browser.h"

FileBrowser::FileBrowser() {
    fileCount = 0;
    scrollOffset = 0;
    selectedIndex = -1;
    currentPath = "/";
}

bool FileBrowser::begin(uint8_t csPin) {
    if (!SD.begin(csPin)) {
        return false;
    }
    openDirectory("/");
    return true;
}

void FileBrowser::openDirectory(const char* path) {
    fileCount = 0;
    scrollOffset = 0;
    selectedIndex = -1;
    currentPath = String(path);
    
    File dir = SD.open(path);
    if (!dir || !dir.isDirectory()) {
        dir.close();
        return;
    }
    
    // Read all files
    while (fileCount < MAX_FILES_DISPLAY) {
        File entry = dir.openNextFile();
        if (!entry) break;
        
        strncpy(files[fileCount].name, entry.name(), FILE_NAME_MAX_LEN - 1);
        files[fileCount].name[FILE_NAME_MAX_LEN - 1] = '\0';
        files[fileCount].isDirectory = entry.isDirectory();
        files[fileCount].size = entry.size();
        
        entry.close();
        fileCount++;
    }
    
    dir.close();
}

void FileBrowser::goUp() {
    if (currentPath.length() <= 1) return;
    
    int lastSlash = currentPath.lastIndexOf('/');
    if (lastSlash > 0) {
        currentPath = currentPath.substring(0, lastSlash);
    } else {
        currentPath = "/";
    }
    
    openDirectory(currentPath.c_str());
}

FileEntry* FileBrowser::getFile(int index) {
    if (index < 0 || index >= fileCount) {
        return nullptr;
    }
    return &files[index];
}

void FileBrowser::scroll(int delta) {
    scrollOffset += delta;
    if (scrollOffset < 0) scrollOffset = 0;
    
    // Max scroll based on file count
    int maxScroll = fileCount - 5; // Show 5 items at a time
    if (maxScroll < 0) maxScroll = 0;
    if (scrollOffset > maxScroll) scrollOffset = maxScroll;
}

void FileBrowser::selectFile(int index) {
    if (index < 0 || index >= fileCount) return;
    
    selectedIndex = index;
    
    if (files[index].isDirectory) {
        // Navigate into directory
        String newPath = currentPath;
        if (newPath != "/") newPath += "/";
        newPath += String(files[index].name);
        openDirectory(newPath.c_str());
    }
}