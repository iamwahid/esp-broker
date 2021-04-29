#pragma once

#include <LittleFS.h>
#include "helpers.h"
#include <vector>
#include <WString.h>

using namespace espbroker;

class FileSystem {
  public:
  FileSystem() {};
  void begin();

  std::vector<String> loadArray(String path);

  bool saveArray(std::vector<String> array, String path);

  bool deleteFile(String path);
};

extern FileSystem FSys;

void FileSystem::begin() { // Start the LittleFS and list all contents
  LittleFS.begin();                             // Start the SPI Flash File System (LittleFS)
  Serial.println("LittleFS started. Contents:");
  {
    Dir dir = LittleFS.openDir("/");
    while (dir.next()) {                      // List the file system contents
      String fileName = dir.fileName();
      size_t fileSize = dir.fileSize();
      Serial.printf("\tFS File: %s, size: %s\r\n", fileName.c_str(), formatBytes(fileSize).c_str());
    }
    Serial.printf("\n");
  }
}

std::vector<String> FileSystem::loadArray(String path) {
  std::vector<String> result;
  if (LittleFS.exists(path)) {
    File file = LittleFS.open(path, "r");
    String line;
    while (file.available()) {
      line = file.readStringUntil('\n');
      line.trim();
      Serial.print("READ LINE: ");
      Serial.println(line);
      result.emplace_back(line);
    }
    file.close();
  }
  return result;
}

bool FileSystem::saveArray(std::vector<String> array, String path) {
  std::vector<String> result;
  File file = LittleFS.open(path, "w");
  if (file) {
    for (String line: array) {
      Serial.print("WRITE LINE: ");
      Serial.println(line);
      file.println(line);
    }
    file.close();
    return true;
  }
  return false;
}

bool FileSystem::deleteFile(String path) {
  std::vector<String> result;
  if (LittleFS.exists(path) && LittleFS.remove(path)) return true;
  return false;
}

FileSystem FSys;
