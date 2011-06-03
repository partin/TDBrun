#ifndef __PARCETCX_H__
#define __PARCETCX_H__

#include "parse.h"
#include "smooth_poly.h"

#include <string>
#include <vector>
#include <sstream>
#include <iomanip>

class ParseTCX;

class TrackPoint {
  friend class ParseTCX;
private:
  std::string time;
  std::string distance;
  std::string heartrate;
  std::string speed;
  double smoothSpeed;
  std::string cadence;
  std::string altitude;
  std::string latitude;
  std::string longitude;

public:

  TrackPoint() : smoothSpeed(0.0) {}

  std::string getTime() const { return time; }
  std::string getDistance() const { return distance.empty() ? "0" : distance; }
  std::string getHeartrate() const { return heartrate.empty() ? "0" : heartrate; }
  std::string getSpeed() const { return speed.empty() ? "0" : speed; }
  double getSmoothSpeed() const { return smoothSpeed; }
  std::string getCadence() const { return cadence.empty() ? "0" : cadence; }
  std::string getAltitude() const { return altitude.empty() ? "0" : altitude; }
  std::string getLatitude() const { return latitude; }
  std::string getLongitude() const { return longitude; }

  std::string getRawAltitude() const { return altitude; }
  void setAltitude(const std::string &s) { altitude = s; }

  void clear() {
    time.clear();
    distance.clear();
    heartrate.clear();
    speed.clear();
    cadence.clear();
    altitude.clear();
    latitude.clear();
    longitude.clear();
    smoothSpeed = 0.0;
  }

  bool isEmpty() {
    return 
      distance.size() == 0 &&
      heartrate.size() == 0 && 
      speed.size() == 0 &&
      cadence.size() == 0 &&
      altitude.size() == 0 &&
      latitude.size() == 0 &&
      longitude.size() == 0;
  }
  bool equals(TrackPoint &tp) const {
    return 
      time == tp.time &&
      distance == tp.distance &&
      heartrate == tp.heartrate &&
      cadence == tp.cadence &&
      altitude == tp.altitude &&
      latitude == tp.latitude &&
      longitude == tp.longitude;
  }
};

class ParseTCX {
private:
  double maxSpeed;
  double meanSpeed;
  double maxHeartRate;
  double meanHeartRate;

  bool hasGotCadence;
  bool hasGotAltitude;
  bool hasGotPosition;

  size_t timeToSec(std::string time) const {
    std::string hh = time.substr(11, 2);
    std::string mm = time.substr(14, 2);
    std::string ss = time.substr(17, 2);

    return 
      atoi(ss.c_str()) +
      60 * atoi(mm.c_str()) +
      60 * 60 * atoi(hh.c_str());
  }

  std::string secsToTime(size_t secs) const {
    size_t h = secs / 60 / 60;
    secs -= h * 60 * 60;
    size_t m = secs / 60;
    secs -= m * 60;
    size_t s = secs;

    std::stringstream ss;
    ss << std::setw(2) << std::setfill('0') << h << ":" 
      << std::setw(2) << std::setfill('0') << m << ":" 
      << std::setw(2) << std::setfill('0') << s;
    return ss.str();
  }

  std::string speedconv(double meter_per_second) const {
    double secs_per_kilometer = 1000.0 / meter_per_second;
    size_t mins_per_kilometer = (size_t) (secs_per_kilometer/60.0);
    secs_per_kilometer -= mins_per_kilometer * 60.0;

    std::stringstream ss;
    ss << std::setw(2) << std::setfill('0') << mins_per_kilometer << ":"
       << std::setw(2) << std::setfill('0') << (size_t) (secs_per_kilometer + 0.5);
    return ss.str();
  }

public:
  std::vector<TrackPoint> trackpoints;

  bool parse(const std::vector<Row> &rows) {

    hasGotCadence = false;
    hasGotAltitude = false;
    hasGotPosition = false;

    maxSpeed = 0.0;
    meanSpeed = 0.0;
    maxHeartRate = 0.0;
    meanHeartRate = 0.0;

    TrackPoint tp;
    bool inTrackpoint = false;

    for (size_t i = 0; i < rows.size(); ++i) {
      if (!rows[i].isTag)
        continue;

      if (!inTrackpoint) {
        if (rows[i].str == "Trackpoint")
          inTrackpoint = true;
        continue;
      }

      if (rows[i].str == "Time")
        tp.time = rows[i+1].str;
      else if (rows[i].str == "DistanceMeters")
        tp.distance = rows[i+1].str;
      else if (rows[i].str == "AltitudeMeters") {
        tp.altitude = rows[i+1].str;
        hasGotAltitude = true;
      }
      else if (rows[i].str == "LatitudeDegrees") {
        tp.latitude = rows[i+1].str;
        hasGotPosition = true;
      }
      else if (rows[i].str == "LongitudeDegrees")
        tp.longitude = rows[i+1].str;
      else if (rows[i].str == "Value")
        tp.heartrate = rows[i+1].str;
      else if (rows[i].str == "Speed")
        tp.speed = rows[i+1].str;
      else if (rows[i].str == "RunCadence") {
        tp.cadence = rows[i+1].str;
        hasGotCadence = true;
      }
      else if (rows[i].str == "/Trackpoint") {
        if (!tp.isEmpty()) {
          if (tp.speed.empty()) {
            if (trackpoints.empty())
              tp.speed = "0";
            else {
              int endTime = timeToSec(tp.time);
              int startTime = timeToSec(trackpoints[trackpoints.size()-1].time);
              double endDist = atof(tp.distance.c_str());
              double startDist = atof(trackpoints[trackpoints.size()-1].distance.c_str());
              int dTime = endTime-startTime;
              if (dTime == 0)
                tp.speed = "0";
              else {
                double speed = (endDist-startDist) / dTime;
                std::stringstream ss;
                ss << speed;
                tp.speed = ss.str();
              }
            }
          }
          if (trackpoints.empty() || !tp.equals(trackpoints[trackpoints.size()-1])) {
            trackpoints.push_back(tp);
          }
        }
        tp.clear();
        inTrackpoint = false;
      }
    }

    for (size_t i = 0; i < trackpoints.size(); ++i) {
      double speed = atof(trackpoints[i].speed.c_str());
      if (speed > maxSpeed)
        maxSpeed = speed;
      //meanSpeed += speed;
      double heartRate = atof(trackpoints[i].heartrate.c_str());
      if (heartRate > maxHeartRate)
        maxHeartRate = heartRate;
      meanHeartRate += heartRate;
    }
    
    //meanSpeed /= trackpoints.size();

    size_t secs = timeToSec(trackpoints[trackpoints.size()-1].time) - timeToSec(trackpoints[0].time);
    meanSpeed = atof(trackpoints[trackpoints.size()-1].distance.c_str()) / secs;

    meanHeartRate /= trackpoints.size();

    // calculate smooth speed

    if (gotPosition()) {
      std::vector<double> time(trackpoints.size());
      std::vector<double> dist(trackpoints.size());
      std::vector<double> vel(trackpoints.size());
      std::vector<double> s(trackpoints.size());
      std::vector<double> lat(trackpoints.size());
      std::vector<double> lng(trackpoints.size());
      std::vector<double> alt(trackpoints.size());
      for (size_t i = 0; i < trackpoints.size(); ++i) {
        time[i] = timeToSec(trackpoints[i].time);
        dist[i] = atof(trackpoints[i].distance.c_str());
        vel[i] = atof(trackpoints[i].speed.c_str());
        lat[i] = atof(trackpoints[i].getLatitude().c_str());
        lng[i] = atof(trackpoints[i].getLongitude().c_str());
        alt[i] = atof(trackpoints[i].getAltitude().c_str());
      }
      Smooth::smooth(time.size(), &time[0], &dist[0], &vel[0], &lat[0], &lng[0], &alt[0], &s[0]);
      for (size_t i = 0; i < trackpoints.size(); ++i)
        trackpoints[i].smoothSpeed = s[i];
    }
    else {
      for (size_t i = 0; i < trackpoints.size(); ++i)
        trackpoints[i].smoothSpeed = atof(trackpoints[i].speed.c_str());
    }

    return !trackpoints.empty();
  }

  std::string getDate() const {
    return trackpoints[0].time.substr(0,10);
  }

  std::string getRunTime() const {
    size_t secs = timeToSec(trackpoints[trackpoints.size()-1].time) - timeToSec(trackpoints[0].time);
    return secsToTime(secs);
  }

  std::string getDistance() const {
    const std::string distance = trackpoints[trackpoints.size()-1].distance;
    std::stringstream ss;
    ss << std::setprecision(1) << std::fixed << atof(distance.c_str());
    return ss.str();
  }

  std::string getMaxSpeed() const {
    return speedconv(maxSpeed);
  }
  std::string getMeanSpeed() const {
    return speedconv(meanSpeed);
  }

  std::string getMaxHeartRate() const {
    std::stringstream ss;
    ss << std::setprecision(1) << std::fixed << maxHeartRate;
    return ss.str();
  }

  std::string getMeanHeartRate() const {
    std::stringstream ss;
    ss << std::setprecision(1) << std::fixed << meanHeartRate;
    return ss.str();
  }

  std::string getMeanLatitude() const {
    double mean = 0.0;
    size_t num = 0;
    for (size_t i = 0; i < trackpoints.size(); ++i) {
      if (trackpoints[i].latitude.empty())
        continue;
      ++num;
      mean += atof(trackpoints[i].latitude.c_str());
    }
    std::stringstream ss;
    ss << mean/num;
    return ss.str();
  }
  std::string getMeanLongitude() const {
    double mean = 0.0;
    size_t num = 0;
    for (size_t i = 0; i < trackpoints.size(); ++i) {
      if (trackpoints[i].longitude.empty())
        continue;
      ++num;
      mean += atof(trackpoints[i].longitude.c_str());
    }
    std::stringstream ss;
    ss << mean/num;
    return ss.str();
  }

  bool gotCadence() const {
    return hasGotCadence;
  }
  bool gotAltitude() const {
    return hasGotAltitude;
  }
  bool gotPosition() const {
    return hasGotPosition;
  }
};

#endif // __PARCETCX_H__
