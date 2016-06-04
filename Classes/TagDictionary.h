#ifndef __TAGDICTIONARY_H__
#define __TAGDICTIONARY_H__

#include "cocostudio/CocoStudio.h"

USING_NS_CC;

class TagDictionary
{
public:
    static TagDictionary *getInstance();
    void add(const std::string &);
    int get(const std::string &);
private:
    TagDictionary();
    TagDictionary(const TagDictionary &);
    int count;
    std::map<std::string, int> map;
};

#endif // __TAGDICTIONARY_H__
