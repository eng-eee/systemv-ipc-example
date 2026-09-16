#ifndef INI_PARSER_H
#define INI_PARSER_H

#include <stdio.h>
#include <string.h>
#include <fstream>
#include <unordered_map>
#include <algorithm>

#define INI_FILE_HEADER "[shared_memory]"


constexpr int INI_SUCCESS =  0;
constexpr int FAILED_TO_OPEN_FILE =  -1;
constexpr int FAILED_TO_READ_FILE =  -2;
constexpr int FAILED_TO_PARSE_FILE =  -3;


class IniParser {
    std::string filename_;
    std::unordered_map<std::string, std::string> config_map_;
public:
    IniParser(const char* filename) : filename_(filename) {}
    int ParseIni() {
        FILE* file = fopen(filename_.c_str(), "r");
        if (!file) {
            perror("Failed to open configuration file");
            return FAILED_TO_OPEN_FILE;
        }

        //Read first line and check for header
        char buff[256];
        if (!fgets(buff, sizeof(buff), file) || strncmp(buff, INI_FILE_HEADER, strlen(INI_FILE_HEADER)) != 0) {
            fclose(file);
            return FAILED_TO_PARSE_FILE;
        }
        // Reset the buffer for reading key-value pairs
        memset(buff, 0, sizeof(buff));
        // Read the rest of the file line by line
        while (fgets(buff, sizeof(buff), file)) {
            //find the '=' character
            auto equal_pos = std::find(buff, buff + strlen(buff), '=');
            if(equal_pos == buff + strlen(buff)) {
                fclose(file);
                return FAILED_TO_PARSE_FILE;
            }

            // Split the line into key and value
            std::string key(buff, equal_pos);
            std::string value(equal_pos + 1, buff + strlen(buff));
            config_map_[key] = value;
        }
        fclose(file);
        return INI_SUCCESS;
    }

    std::string GetValue(const std::string& key) {
        auto it = config_map_.find(key);
        if (it != config_map_.end()) {
            return it->second;
        }
        return "";
    }
};



#endif