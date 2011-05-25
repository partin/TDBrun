#ifndef __MATLABOUT_H__
#define __MATLABOUT_H__

#include "parsetcx.h"

class MATLABOut {
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
    return y+" "+m+" "+d+" "+hh+" "+mm+" "+ss;
  }

public:
  void out(const std::vector<ParseTCX> &ptcx, std::ofstream &out) {

    for (size_t j = 0; j < ptcx.size(); ++j) {

      const std::vector<TrackPoint> &tps(ptcx[j].trackpoints);

      std::string indexStr = "";
      if (ptcx.size() > 1) {
        std::stringstream ss;
        ss << "{" << (j+1) << "}";
        indexStr = ss.str();
      }

      out << "time"<<indexStr<<"=datenum([" << toDate(tps[0].getTime());
      for (size_t i = 1; i < tps.size(); ++i)
        out << "; " << toDate(tps[i].getTime());
      out << "]);" << std::endl;

      out << "distance"<<indexStr<<"=[" << (tps[0].getDistance());
      for (size_t i = 1; i < tps.size(); ++i)
        out << ", " << (tps[i].getDistance());
      out << "];" << std::endl;

      out << "heartrate"<<indexStr<<"=[" << (tps[0].getHeartrate());
      for (size_t i = 1; i < tps.size(); ++i)
        out << ", " << (tps[i].getHeartrate());
      out << "];" << std::endl;

      out << "speed"<<indexStr<<"=[" << (tps[0].getSpeed());
      for (size_t i = 1; i < tps.size(); ++i)
        out << ", " << (tps[i].getSpeed());
      out << "];" << std::endl;

      out << "speedsmooth"<<indexStr<<"=[" << (tps[0].getSmoothSpeed());
      for (size_t i = 1; i < tps.size(); ++i)
        out << ", " << (tps[i].getSmoothSpeed());
      out << "];" << std::endl;

      if (ptcx[j].gotCadence()) {
        out << "cadence"<<indexStr<<"=[" << (tps[0].getCadence());
        for (size_t i = 1; i < tps.size(); ++i)
          out << ", " << (tps[i].getCadence());
        out << "];" << std::endl;
      }
      if (ptcx[j].gotAltitude()) {
        out << "altitude"<<indexStr<<"=[" << (tps[0].getAltitude());
        for (size_t i = 1; i < tps.size(); ++i)
          out << ", " << (tps[i].getAltitude());
        out << "];" << std::endl;
      }
      if (ptcx[j].gotPosition()) {
        out << "longitude"<<indexStr<<"=[" << (tps[0].getLongitude());
        for (size_t i = 1; i < tps.size(); ++i)
          out << ", " << (tps[i].getLongitude());
        out << "];" << std::endl;
        out << "latitude"<<indexStr<<"=[" << (tps[0].getLatitude());
        for (size_t i = 1; i < tps.size(); ++i)
          out << ", " << (tps[i].getLatitude());
        out << "];" << std::endl;
      }
      out << std::endl;
    }

    std::string indexStr;
    if (ptcx.size() > 1)
      indexStr = "{1}";
    
    out << "% Example plots:" << std::endl;
    out << "figure; plot(time"<<indexStr<<", distance"<<indexStr<<"); datetick('x','HH:MM:SS'); ylabel('Distance (m)');" << std::endl;
    out << "figure; plot(time"<<indexStr<<", heartrate"<<indexStr<<"); datetick('x','HH:MM:SS'); ylabel('Heart Rate (bpm)');" << std::endl;
    out << "figure; plot(time"<<indexStr<<", speed"<<indexStr<<"); datetick('x','HH:MM:SS'); ylabel('Speed (m/s)');" << std::endl;
    if (ptcx[0].gotCadence()) {
      out << "figure; plot(time"<<indexStr<<", cadence"<<indexStr<<"); datetick('x','HH:MM:SS'); ylabel('Cadence');" << std::endl;
    }
    if (ptcx[0].gotAltitude()) {
      out << "figure; plot(time"<<indexStr<<", altitude"<<indexStr<<"); datetick('x','HH:MM:SS'); ylabel('Altitude (m)');" << std::endl;
    }
    if (ptcx[0].gotPosition()) {
      out << "figure; plot(longitude"<<indexStr<<", latitude"<<indexStr<<");" << std::endl;
    }
  }
};

#endif // __MATLABOUT_H__
