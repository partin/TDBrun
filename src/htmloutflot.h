#ifndef __HTMLOUT_H__
#define __HTMLOUT_H__

#include "parsetcx.h"
#include <string>
#include <algorithm>
#include <iomanip>

namespace {

  struct Distance {
    static double get(const TrackPoint &a) { return atof(a.getDistance().c_str()); }
  };

  struct Heartrate {
    static double get(const TrackPoint &a) { return atof(a.getHeartrate().c_str()); }
  };

  struct Cadence {
    static double get(const TrackPoint &a) { return atof(a.getCadence().c_str()); }
  };

  struct SmoothSpeed {
    static double get(const TrackPoint &a) { return a.getSmoothSpeed(); }
  };

  struct Altitude {
    static double get(const TrackPoint &a) { return atof(a.getAltitude().c_str()); }
  };

  template<class Getter>
  bool comparator(const TrackPoint &a, const TrackPoint &b) {
    return Getter::get(a) < Getter::get(b);
  }

  template<class Getter>
  static std::string toArrayStr(const std::string &name,
    const std::vector<TrackPoint> &tps) {

    double minVal = Getter::get(*std::min_element(tps.begin(), tps.end(), comparator<Getter>));
    double maxVal = Getter::get(*std::max_element(tps.begin(), tps.end(), comparator<Getter>));

    std::stringstream ss;
    ss << "  " << name << " = [" << Getter::get(tps[0]);
    for (size_t i = 1; i < tps.size(); ++i)
      ss << "," << std::setprecision(5) << Getter::get(tps[i]);
    ss << "];\n";

  //  string minname = "min"+name;
  //  string maxname = "max"+name;
  //  ss <<
  //"    var " << name << " = []\n"
  //"    var " << minname << " = Math.min.apply(null, " << name << "Data);\n"
  //"    var " << maxname << " = Math.max.apply(null, " << name << "Data);\n"
  //"    for (i = 0; i < " << name << "Data.length; ++i)" "\n"
  //"      " << name << ".push([startTime+2*60*60*1000+time[i], (" << name << "Data[i]-"<<minname<<")/("<<maxname<<"-"<<minname<<")]);" "\n";
    return ss.str();
  }
}

class HTMLOutFlot {
private:

  std::string toText(std::string time) {
    int hh_val = atoi(time.substr(11, 2).c_str()) + 2;
    std::stringstream s;
    s << hh_val;
    std::string hh = s.str();
    std::string mm = time.substr(14, 2);
    std::string ss = time.substr(17, 2);
    return hh + ":" + mm + ":" + ss;
  }
  std::string toDate(std::string time) {
    std::string y = time.substr(0, 4);
    std::string m = time.substr(5, 2);
    std::string d = time.substr(8, 2);
    int hh_val = atoi(time.substr(11, 2).c_str()) + 2;
    std::stringstream s;
    s << hh_val;
    std::string hh = s.str();
    std::string mm = time.substr(14, 2);
    std::string ss = time.substr(17, 2);
    // stupid JavaScript parses integers starting with 0 as octal
    if (m[0] == '0')
      m = m[1];
    if (d[0] == '0')
      d = d[1];
    if (hh[0] == '0')
      hh = hh[1];
    if (mm[0] == '0')
      mm = mm[1];
    if (ss[0] == '0')
      ss = ss[1];
    return "new Date("+y+","+m+","+d+","+hh+","+mm+","+ss+").getTime()";
  }

  int getSecs(std::string time) {
    int hh_val = atoi(time.substr(11, 2).c_str()) + 2;
    int mm_val = atoi(time.substr(14, 2).c_str());
    int ss_val = atoi(time.substr(17, 2).c_str());
    return 1000*(ss_val + 60*(mm_val + 60*hh_val));
  }

public:
  void out(const std::vector<ParseTCX> &data, std::ofstream &out) {

    std::string date = data[0].getDate();
    bool gotPosition = false;
    bool gotCadence = false;
    bool gotAltitude = false;

    for (size_t i = 0; i < data.size(); ++i) {
      if (data[i].gotPosition())
        gotPosition = true;
      if (data[i].gotCadence())
        gotCadence = true;
      if (data[i].gotAltitude())
        gotAltitude = true;
    }

    out <<
"<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">" "\n"
"<html xmlns=\"http://www.w3.org/1999/xhtml\">" "\n"
"<head>" "\n"
"<meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\" />" "\n"
"<title>Running " << date << "</title>" "\n"
"<script type=\"text/javascript\" src=\"http://www.google.com/jsapi\"></script>" "\n"
"<link rel=\"stylesheet\" href=\"http://ajax.googleapis.com/ajax/libs/jqueryui/1.8.13/themes/base/jquery-ui.css\" type=\"text/css\" media=\"all\" />" "\n"
"<script src=\"http://ajax.googleapis.com/ajax/libs/jquery/1.5.1/jquery.min.js\" type=\"text/javascript\"></script>" "\n"
"<script src=\"http://ajax.googleapis.com/ajax/libs/jqueryui/1.8.13/jquery-ui.min.js\" type=\"text/javascript\"></script>" "\n"
"<script language=\"javascript\" type=\"text/javascript\" src=\"js/jquery.flot.js\"></script>" "\n"
"<script language=\"javascript\" type=\"text/javascript\" src=\"js/jquery.flot.crosshair.js\"></script>" "\n"
"<script language=\"javascript\" type=\"text/javascript\" src=\"js/jquery.flot.selection.js\"></script>" "\n"
"<script language=\"javascript\" type=\"text/javascript\" src=\"js/jquery.flot.navigate.js\"></script>" "\n";

    if (gotPosition) out <<
"<script type=\"text/javascript\" src=\"http://maps.google.com/maps/api/js?sensor=false\"></script>" "\n";

    out <<"<script type=\"text/javascript\">" "\n";

    out << "  var numDatas = " << data.size() << std::endl;
    if (gotPosition) { // lazy: takes mean of first data instead of real mean
      out <<
"  var myLatLng = new google.maps.LatLng(" << data[0].getMeanLatitude() << ", " << data[0].getMeanLongitude() << ");" "\n";
    }

    out << "  var lat = [];" "\n";
    out << "  var lng = [];" "\n";
    out << "  var posindex = [];" "\n";
    out << "  var startTime = [];" "\n";
    out << "  var time = [];" "\n";
    out << "  var distanceData = [];" "\n";
    out << "  var heartrateData = [];" "\n";
    out << "  var speedData = [];" "\n";
    out << "  var cadenceData = [];" "\n";
    out << "  var altitudeData = [];" "\n";
    out << "\n";

    for (size_t j = 0; j < data.size(); ++j) {

      const std::vector<TrackPoint> &tps(data[j].trackpoints);

      std::string indexStr;
      //if (data.size() > 1) {
      {
        std::stringstream ss;
        ss << "[" << j << "]";
        indexStr = ss.str();
      }

      if (gotPosition) {
        std::vector<int> posIndex(tps.size());

        out << "  lat"<<indexStr<<" = [";
        bool first = true;
        size_t index = 0;
        for (size_t i = 0; i < tps.size(); ++i) {
          posIndex[i] = index;
          if (!tps[i].getLatitude().empty() && !tps[i].getLongitude().empty()) {
            if (!first)
              out << ",";
            out << tps[i].getLatitude();
            first = false;
            ++index;
          }
        }
        out << "];\n";

        out << "  lng"<<indexStr<<" = [";
        first = true;
        for (size_t i = 0; i < tps.size(); ++i) {
          if (!tps[i].getLatitude().empty() && !tps[i].getLongitude().empty()) {
            if (!first)
              out << ",";
            out << tps[i].getLongitude();
            first = false;
          }
        }
        out << "];\n";

        out << "  posindex"<<indexStr<<" = [" << posIndex[0];
        first = true;
        for (size_t i = 1; i < tps.size(); ++i)
          out << "," << posIndex[i];
        out << "];\n";
      }

      out << "  startTime"<<indexStr<<" = " << toDate(tps[0].getTime()) << ";\n";
      int firstTime = getSecs(tps[0].getTime());

      out << "  time"<<indexStr<<" = [0";
      for (size_t i = 1; i < tps.size(); ++i)
        out << "," << getSecs(tps[i].getTime()) - firstTime;
      out << "];\n";

      out << toArrayStr<Distance>("distanceData"+indexStr, tps);
      out << toArrayStr<Heartrate>("heartrateData"+indexStr, tps);
      out << toArrayStr<SmoothSpeed>("speedData"+indexStr, tps);

      if (data[j].gotCadence())
        out << toArrayStr<Cadence>("cadenceData"+indexStr, tps);
      else
        out << "  cadenceData"+indexStr+" = null;" "\n";

      if (data[j].gotAltitude())
        out << toArrayStr<Altitude>("altitudeData"+indexStr, tps);
      else
        out << "  altitudeData"+indexStr+" = null;" "\n";
  
      out << "\n";
    }

    std::string space = "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;";

    out << "  var datasets = {" "\n";
    out << "    \"distance\": { label: \"Distance"<<space<<"\", data: distanceData }," "\n";
    out << "    \"heartrate\": { label: \"Heartrate"<<space<<"\", data: heartrateData }," "\n";
    out << "    \"speed\": { label: \"Tempo"<<space<<"\", data: speedData }";
    if (gotCadence)
      out << ",\n" 
            "    \"cadence\": { label: \"Cadence"<<space<<"\", data: cadenceData }";
    if (gotAltitude)
      out << ",\n"
            "    \"altitude\": { label: \"Altitude"<<space<<"\", data: altitudeData }";
    out << "\n" "  };" "\n";

    out << "\n";
    out << "</script>" "\n";

    out << // ################# TEMPORARILY common2.js #####################
"<script language=\"javascript\" type=\"text/javascript\" src=\"common2.js\"></script>" "\n" 
"<link rel=StyleSheet href=\"style.css\" type=\"text/css\">" "\n"
"</head>" "\n";

  // --- <BODY> ----------------------------------------------------------

    out << "<body onload=\"initialize(" << (gotPosition ? "true" : "false") << ")\">" "\n";
 
    out <<
"\n"
"<center>" "\n"
"<table id=\"a\">" "\n"
"<tr>" "\n"
"<th>Date</th><th>Run Time</th><th>Distance (m)</th><th>Avg Speed (/km)</th><th>Max Speed (/km)</th><th>Avg HR (bpm)</th><th>Max HR (bpm)</th>" "\n"
"</tr>" "\n"
"<tr>" "\n";
    out << "<td>" << date << "</td>" "\n";

    out << "<td>";
    for (size_t i = 0; i < data.size(); ++i)
      out << (i > 0 ? "<br>" : "") << data[i].getRunTime();
    out << "</td>" << std::endl;

    out << "<td>";
    for (size_t i = 0; i < data.size(); ++i)
      out << (i > 0 ? "<br>" : "") << data[i].getDistance();
    out << "</td>" << std::endl;

    out << "<td>";
    for (size_t i = 0; i < data.size(); ++i)
      out << (i > 0 ? "<br>" : "") << data[i].getMeanSpeed();
    out << "</td>" << std::endl;

    out << "<td>";
    for (size_t i = 0; i < data.size(); ++i)
      out << (i > 0 ? "<br>" : "") << data[i].getMaxSpeed();
    out << "</td>" << std::endl;

    out << "<td>";
    for (size_t i = 0; i < data.size(); ++i)
      out << (i > 0 ? "<br>" : "") << data[i].getMeanHeartRate();
    out << "</td>" << std::endl;

    out << "<td>";
    for (size_t i = 0; i < data.size(); ++i)
      out << (i > 0 ? "<br>" : "") << data[i].getMaxHeartRate();
    out << "</td>" << std::endl;

    out << "</tr>" "\n" "</table>" "\n" "\n" "<br>" "\n" "\n";

    if (gotPosition) {
      out << "<div id=\"resizable1\" style=\"width: 80%; height: 300px; padding: 0.5em;\" class=\"ui-widget-content\">" "\n";
      out << "<div id=\"map_canvas\" style=\"width: 100%; height: 100%;\"></div>" "\n";
      out << "</div>" "\n";
    }
    //out << "Time: <span id='spantime'>00:00:00</span>" "\n";
    out << "<table id=\"legend\">" "\n";
    out << "<tr><th>Time&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</th>"
           "<th>Distance (m)</th><th>Tempo (/ km)</th><th>Heartrate (bpm)</th>";
    if (gotAltitude)
      out << "<th>Altitude (m)</th>";
    if (gotCadence)
      out << "<th>Cadence</th>";
    out << "</tr>" "\n";
    for (size_t i = 0; i < data.size(); ++i) {
      out << "<tr>" "\n";
      out << "<td><span id='spantime"<<i<<"'>00:00:00</span></td>" "\n";
      out << "<td><span id='spandistance"<<i<<"'>0</span></td>" "\n";
      out << "<td><span id='spanspeed"<<i<<"'>00:00</span></td>" "\n";
      out << "<td><span id='spanheartrate"<<i<<"'>0</span></td>" "\n";
      if (gotAltitude)
        out << "<td><span id='spanaltitude"<<i<<"'>0</span></td>" "\n";
      if (gotCadence)
        out << "<td><span id='spancadence"<<i<<"'>0</span></td>" "\n";
      out << "</tr>" "\n";
    }
    out << "</table>" "\n";

    out << "<div id=\"resizable2\" style=\"width: 80%; height: 300px; padding: 0.5em;\" class=\"ui-widget-content\">" "\n";
    out << "<div id=\"placeholder\" style=\"width: 100%; height: 100%\"></div>" "\n";
    out << "</div>" "\n";

    out << "<button id=\"zoomout\">Zoom Out</button>" "\n";
    out << "<button id=\"plotvstime\">Plot vs Time</button>" "\n";
    out << "<button id=\"plotvsdistance\">Plot vs Distance</button>" "\n";
    out << "<button id=\"updatealt\">Altitude</button>" "\n";
    //out << "<div id=\"legend\" style=\"width:80%;height:300px\"></div>" "\n";
    out << "<p id=\"choices\"></p>" "\n";
    out << "</center>" "\n";
    out <<
      "<a href=\"" << ("run"+date+".m") << "\">Download as MATLAB file</a>" "\n"
      "</body>" "\n"
      "</html>" "\n";
  }
};

#endif // __HTMLOUT_H__
