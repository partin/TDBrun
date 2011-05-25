#ifndef __HTMLOUTD_H__
#define __HTMLOUTD_H__

#include "parsetcx.h"

class HTMLOutD {
private:

  std::string toDate(std::string time) {
    std::string y = time.substr(0, 4);
    std::string m = time.substr(5, 2);
    std::string d = time.substr(8, 2);
    std::string hh = time.substr(11, 2);
    std::string mm = time.substr(14, 2);
    std::string ss = time.substr(17, 2);
    return "new Date("+y+","+m+","+d+","+hh+","+mm+","+ss+")";
  }

public:
  void out(const std::vector<TrackPoint> &tps) {

    std::cout <<
"<html>" "\n"
"<head>" "\n"
"<script type=\"text/javascript\"" "\n"
"  src=\"http://danvk.org/dygraphs/dygraph-combined.js\"></script>" "\n"
"</head>" "\n"
"<body>" "\n"
"<div id=\"graphdiv\"></div>" "\n"
"<script type=\"text/javascript\">" "\n"
"  g = new Dygraph(" "\n"
    "// containing div" "\n"
    "document.getElementById(\"graphdiv\")," "\n"
    ;

    std::cout << 
      "\"Date,HeartRate,Speed,Cadence\\n\" +" << std::endl;

    // CSV or path to a CSV file.

    for (size_t i = 0; i < tps.size(); ++i) {
      std::cout 
        << "\""
        << tps[i].time.substr(0, 10) + " " + tps[i].time.substr(11, 8) << "," 
//        << tps[i].distance << ","
        << tps[i].heartrate << ","
        << tps[i].speed << ","
        << tps[i].cadence;

      if (i == tps.size() - 1)
        std::cout << "\\n\"" << std::endl;
      else
        std::cout << "\\n\" + " << std::endl;
    }

    std::cout << 
"  );" "\n"
"</script>" "\n"
"</body>" "\n"
"</html>" "\n"
;
  }
};

#endif // __HTMLOUT_H__
