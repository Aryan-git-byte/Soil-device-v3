/**
 * @file file_browser.h
 * @brief SD Card File Browser for A9G Board
 */

#ifndef FILE_BROWSER_H
#define FILE_BROWSER_H

#include <Arduino.h>
#include <SD.h>
#include "config.h"

#define MAX_FILES_DISPLAY 20
#define FILE_NAME_MAX_LEN 32

typedef struct {
    char name[FILE_NAME_MAX_LEN];
    bool isDirectory;
    uint32_t size;
} FileEntry;

class FileBrowser {
private:
    FileEntry files[MAX_FILES_DISPLAY];
    int fileCount;
    int scrollOffset;
    int selectedIndex;
    String currentPath;
    
public:
    FileBrowser();
    bool begin(uint8_t csPin);
    void openDirectory(const char* path);
    void goUp();
    int getFileCount() { return fileCount; }
    int getScrollOffset() { return scrollOffset; }
    FileEntry* getFile(int index);
    void scroll(int delta);
    void selectFile(int index);
    int getSelectedIndex() { return selectedIndex; }
    const char* getCurrentPath() { return currentPath.c_str(); }
    bool canGoUp() { return currentPath.length() > 1; }
};

#endif // FILE_BROWSER_H