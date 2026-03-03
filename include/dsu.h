#ifndef DSU_H
#define DSU_H
#include <map>
#include <string>
#include <vector>

class DSU {
    std::map<std::string, std::string> parent;
public:
    std::string find(std::string i);
    void unite(std::string i, std::string j);
};
#endif