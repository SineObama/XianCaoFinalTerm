#include "TagDictionary.h"

TagDictionary *TagDictionary::getInstance() {
    static TagDictionary instance;
    return &instance;
}

TagDictionary::TagDictionary() {
    count = 0;
}

void TagDictionary::add(const std::string &s) {
    map.insert(std::pair<std::string, int>(s, count++));
}

int TagDictionary::get(const std::string &s) {
    return map[s];
}
