#ifndef __HTMLOUT_H__
#define __HTMLOUT_H__

#include "parsetcx.h"
#include <string>

class HTMLOut {
private:

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
    return "new Date("+y+","+m+","+d+","+hh+","+mm+","+ss+")";
  }

public:
  void out(const ParseTCX &ptcx, std::ofstream &out) {

    std::string date = ptcx.getDate();
    std::string runtime = ptcx.getRunTime();
    std::string distance = ptcx.getDistance();
    std::string maxSpeedStr = ptcx.getMaxSpeed();
    std::string meanSpeedStr = ptcx.getMeanSpeed();
    std::string maxHeartRate = ptcx.getMaxHeartRate();
    std::string meanHeartRate = ptcx.getMeanHeartRate();

    const std::vector<TrackPoint> &tps(ptcx.trackpoints);

    out <<
      "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">" "\n"
"<html xmlns=\"http://www.w3.org/1999/xhtml\">" "\n"
"<head>" "\n"
"  <meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\" />" "\n"
"  <title>Running " << date << "</title>" "\n"
"  <script type=\"text/javascript\" src=\"http://www.google.com/jsapi\"></script>" "\n"
"  <link type=\"text/css\" href=\"css/start/jquery-ui-1.8.2.custom.css\" rel=\"stylesheet\" />" "\n"
"  <script type=\"text/javascript\" src=\"js/jquery-1.4.2.min.js\"></script>" "\n"
"  <script type=\"text/javascript\" src=\"js/jquery-ui-1.8.2.custom.min.js\"></script>" "\n"
"  <script type=\"text/javascript\">" "\n"
"    google.load('visualization', '1', {packages: ['annotatedtimeline']});" "\n"
"    function drawVisualization() {" "\n"
"      var data = new google.visualization.DataTable();" "\n"
"      data.addColumn('datetime', 'Date');" "\n"
"      data.addColumn('number', 'Distance (m)');" "\n"
"      data.addColumn('number', 'Heart Rate (bps)');" "\n"
"      data.addColumn('number', 'Speed (m/s)');" "\n";
    if (ptcx.gotCadence())
      out << "      data.addColumn('number', 'Cadence');" "\n";
    if (ptcx.gotAltitude())
      out << "      data.addColumn('number', 'Altitude (m)');" "\n";

    out << std::endl;

    out << 
"      data.addRows(" << tps.size() << ");" << std::endl;

  for (size_t i = 0; i < tps.size(); ++i) {
    //out << "      t = new DateTime();" << std::endl;
    //out << "      t.date = "<<toDate(tps[i].time)<<";" << std::endl;
    //out << "      t.TimeVisible = true;" << std::endl;
    //out << "      data.setValue("<<i<<", 0, t);" << std::endl;
    out << 
"      data.setValue("<<i<<", 0, "<<toDate(tps[i].getTime())<< ");" << std::endl;
    out << 
"      data.setValue("<<i<<", 1, "<<tps[i].getDistance()<< ");" << std::endl;
    out << 
"      data.setValue("<<i<<", 2, "<<tps[i].getHeartrate()<< ");" << std::endl;
    out << 
"      data.setValue("<<i<<", 3, "<<tps[i].getSmoothSpeed()<< ");" << std::endl;
    if (ptcx.gotCadence())
      out << "      data.setValue("<<i<<", 4, "<<tps[i].getCadence()<< ");" << std::endl;
    if (ptcx.gotAltitude())
      out << "      data.setValue("<<i<<", 4, "<<tps[i].getAltitude()<< ");" << std::endl;
  }

  out << 
"      " "\n"
"      var annotatedtimeline = new google.visualization.AnnotatedTimeLine(" "\n"
"          document.getElementById('visualization'));" "\n"
"      annotatedtimeline.draw(data, {"
//"'zoomStartTime': " << toDate(tps[0].time) << ", " <<
//"'zoomEndTime': " << toDate(tps[tps.size()-1].time) << ", " <<
"'displayAnnotations': true, "
"'scaleType': 'allmaximized'"
//"'scaleColumns': [1,2,3,4]"
"});" "\n"
"    }" "\n"
"    " "\n"
"    google.setOnLoadCallback(drawVisualization);" "\n"
"  </script>" "\n";

  if (ptcx.gotPosition()) {
    out <<
"<script type=\"text/javascript\" src=\"http://maps.google.com/maps/api/js?sensor=false\"></script>" "\n"
"<script type=\"text/javascript\">" "\n"
"" "\n"
"  function initialize() {" "\n"
"    var myLatLng = new google.maps.LatLng(" << ptcx.getMeanLatitude() << ", " << ptcx.getMeanLongitude() << ");" "\n"
"    var myOptions = {" "\n"
"      zoom: 12," "\n"
"      center: myLatLng," "\n"
"      mapTypeId: google.maps.MapTypeId.TERRAIN" "\n"
"    };" "\n"
"" "\n"
"    var map = new google.maps.Map(document.getElementById(\"map_canvas\"), myOptions);" "\n"
"" "\n"
"    var flightPlanCoordinates = [" "\n";
  for (size_t i = 0; i < tps.size(); ++i) {
    if (!tps[i].getLatitude().empty() && !tps[i].getLongitude().empty())
      out << "        new google.maps.LatLng(" << tps[i].getLatitude() << ", " << tps[i].getLongitude()<< ")," "\n";
  }
  out <<
"    ];" "\n"
"    var flightPath = new google.maps.Polyline({" "\n"
"      path: flightPlanCoordinates," "\n"
"      strokeColor: \"#FF0000\"," "\n"
"      strokeOpacity: 1.0," "\n"
"      strokeWeight: 2" "\n"
"    });" "\n"
"" "\n"
"   flightPath.setMap(map);" "\n"
"   marker = new google.maps.Marker({position: flightPlanCoordinates[0], map: map});" "\n"
"" "\n"
"  $(function() {" "\n"
"    $(\"#slider\").slider({" "\n"
"      slide: function(event, ui) { " "\n"
"          marker.setPosition(flightPlanCoordinates[ui.value]);" "\n"
"        }," "\n"
"      min: 0," "\n"
"      max: flightPlanCoordinates.length-1" "\n"
"    });" "\n"
"  });" "\n"
"}" "\n"
"</script>" "\n";

  }

  out <<
"<link rel=StyleSheet href=\"style.css\" type=\"text/css\">" "\n"
"</head>" "\n";
  if (ptcx.gotPosition())
    out << "<body onload=\"initialize()\">" "\n";
  else
    out << "<body>" "\n";
  out <<
"\n"
"<center>" "\n"
"<table id=\"a\">" "\n"
"<tr>" "\n"
"<th>Date</th><th>Run Time</th><th>Distance (m)</th><th>Avg Speed (/km)</th><th>Max Speed (/km)</th><th>Avg HR (bpm)</th><th>Max HR (bpm)</th>" "\n"
"</tr>" "\n"
"<tr>" "\n"
"<td>" << date << "</td>" "\n"
"<td>" << runtime << "</td>" "\n"
"<td>" << distance << "</td>" "\n"
"<td>" << meanSpeedStr << "</td>" "\n"
"<td>" << maxSpeedStr << "</td>" "\n"
"<td>" << meanHeartRate << "</td>" "\n"
"<td>" << maxHeartRate << "</td>" "\n"
"</tr>" "\n"
"</table>" "\n"
"" "\n"
"<br>" "\n"
"\n";
  if (ptcx.gotPosition()) {
    out << "<div id=\"map_canvas\" style=\"width: 80%; height: 300px;\"></div>" "\n";
    out << "<div id=\"slider\" style=\"margin: 10pt; width: 80%\"></div>" "\n";
  }
  out << 
"<div id=\"visualization\" style=\"width: 100%; height: 400px;\"></div>" "\n"
"</center>" "\n"
"<a href=\"" << ("run"+date+".m") << "\">Download as MATLAB file</a>" "\n"
"</body>" "\n"
"</html>" "\n"
  << std::endl;
  }
};

#endif // __HTMLOUT_H__
