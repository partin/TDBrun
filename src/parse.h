#ifndef __PARSE_H__
#define __PARSE_H__

#include <string>
#include <vector>
#include <fstream>
#include <iostream>

//-----------------------------------------------------------------------------------------------------------
class Row
//-----------------------------------------------------------------------------------------------------------
{
public:
  std::string str;
  bool isTag;
  Row(const std::string & str_, bool isTag_) {
    this->str = str_;
    this->isTag = isTag_;
  }
  operator std::string() {
    return str;
  }
};

class Parser {

//-----------------------------------------------------------------------------------------------------------
  void splitFile(const std::string &bigstr)
//-----------------------------------------------------------------------------------------------------------
  {
    size_t tagpos;
    size_t endpos = 0;
    result.clear();
    while ( (tagpos = bigstr.find("<", endpos)) != std::string::npos) {
      result.push_back(Row(bigstr.substr(endpos, tagpos++ - endpos), false));
      endpos = bigstr.find(">", tagpos);
      std::string str = bigstr.substr(tagpos, endpos++ - tagpos);
      if (str.substr(0, 3) != "!--" || str.substr(str.size()-2) != "--")
        result.push_back(Row(str, true));
    }
  }

public:
  std::vector<Row> result;

//-----------------------------------------------------------------------------------------------------------
  bool readFile(const char * filename)
//-----------------------------------------------------------------------------------------------------------
  {
    std::string str;
    std::ifstream in(filename);
    if (in.eof())
      return false;

    std::string output;
    in >> str;
    output += str;
    while (in.good() && !in.eof()) {
      std::string str;
      in >> str;
      output += ' ';
      output += str;
    }

    splitFile(output);
    return true;
  }
};

#endif // __PARSE_H__
