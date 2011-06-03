#ifndef __PARCEGPX_H__
#define __PARCEGPX_H__

#include "parse.h"

#include <string>
#include <vector>
#include <sstream>
#include <iomanip>

class ParseGPX {
public:

  struct TrackPoint {
    std::string altitude;
    std::string time;
    void clear() {
      altitude = "";
      time = "";
    }
  };

  std::vector<std::string> altitude;

  bool parse(const std::vector<Row> &rows) {

    bool inTrackpoint = false;
    TrackPoint prev;
    TrackPoint tp;

    for (size_t i = 0; i < rows.size(); ++i) {
      if (!rows[i].isTag)
        continue;

      if (!inTrackpoint) {
        if (rows[i].str.substr(0,5) == "trkpt")
          inTrackpoint = true;
        continue;
      }

      if (rows[i].str == "ele")
        tp.altitude = rows[i+1].str;
      else if (rows[i].str == "time")
        tp.time = rows[i+1].str;
      else if (rows[i].str == "/trkpt") {
        if (tp.time != prev.time) {
          altitude.push_back(tp.altitude);
          prev = tp;
        }
        tp.clear();
        inTrackpoint = false;
      }
    }
    return !altitude.empty();
  }

};

#endif // __PARCEGPX_H__
