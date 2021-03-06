#include "parse.h"
#include "parsetcx.h"
#include "parsegpx.h"
#include "matlabout.h"
#include "htmloutflot.h"

#include <string>
#include <fstream>

using namespace std;

#include <stdlib.h>
#include <stdio.h>

int main(int argc, char *argv[]) {

  vector<string> filenames;

  if (argc < 2) {
    //cerr << "Usage: " << argv[0] << " <filename>" << endl;
    //return 1;
    filenames.push_back("C:\\Dokument\\Projects\\ParseTCX\\Debug\\2011-05-31-Martin.tcx");
    filenames.push_back("C:\\Dokument\\Projects\\ParseTCX\\Debug\\2011-05-31-Magnus.tcx");
  }
  else {
    for (int i = 1; i < argc; ++i)
      filenames.push_back(argv[i]);
  }

  string outname;

  vector<ParseTCX> data(filenames.size());

  for (size_t i = 0; i < filenames.size(); ++i) {

    Parser p;
    if (!p.readFile(filenames[i].c_str())) {
      cerr << "Failed to read file \"" << filenames[i] << "\"" << endl;
      return 1;
    }

    if (!data[i].parse(p.result)) {
      cerr << "Failed to parse \"" << filenames[i] << "\"" << endl;
      return 1;
    }

    Parser pgpx;
    string gpxfile = filenames[i].substr(0, filenames[i].length()-4) + ".gpx";
    if (pgpx.readFile(gpxfile.c_str())) {
      ParseGPX gpx;
      if (gpx.parse(pgpx.result) && !gpx.altitude.empty()) {
        size_t num = 0;
        for (size_t j = 0; j < data[i].trackpoints.size(); ++j) {
          if (!data[i].trackpoints[j].getAltitude().empty())
            ++num;
        }
        if (num == gpx.altitude.size()) {
          num = 0;
          for (size_t j = 0; j < data[i].trackpoints.size(); ++j) {
            if (!data[i].trackpoints[j].getAltitude().empty())
              data[i].trackpoints[j].setAltitude(gpx.altitude[num++]);
          }
        }
      }
    }



    outname = data[i].getDate();
    for (size_t i = 0; i < outname.size(); ++i)
      if (outname[i] == ':')
        outname[i] = '_';
  }

  {
    ofstream f( ("run" + outname + ".html").c_str(), ios::out);
    HTMLOutFlot out;
    out.out(data, f);
    f.close();
  }

  {
    ofstream f( ("run" + outname + ".m").c_str(), ios::out);
    MATLABOut out;
    out.out(data, f);
    f.close();
  }

  {
    ofstream f("index_line.txt", ios::out);

    std::string date = data[0].getDate();

    f << "<tr>" << endl
      << "<td><a href=\"run" << date << ".html\">" << date << "</a></td>" << endl;

    f << "<td>";
    for (size_t i = 0; i < data.size(); ++i)
      f << (i > 0 ? "<br>" : "") << data[i].getRunTime();
    f << "</td>" << endl;

    f << "<td>";
    for (size_t i = 0; i < data.size(); ++i)
      f << (i > 0 ? "<br>" : "") << data[i].getDistance();
    f << "</td>" << endl;

    f << "<td>";
    for (size_t i = 0; i < data.size(); ++i)
      f << (i > 0 ? "<br>" : "") << data[i].getMeanSpeed();
    f << "</td>" << endl;

    f << "<td>";
    for (size_t i = 0; i < data.size(); ++i)
      f << (i > 0 ? "<br>" : "") << data[i].getMaxSpeed();
    f << "</td>" << endl;

    f << "<td>";
    for (size_t i = 0; i < data.size(); ++i)
      f << (i > 0 ? "<br>" : "") << data[i].getMeanHeartRate();
    f << "</td>" << endl;

    f << "<td>";
    for (size_t i = 0; i < data.size(); ++i)
      f << (i > 0 ? "<br>" : "") << data[i].getMaxHeartRate();
    f << "</td>" << endl;

    f << "<td><a href=\"run" << date << ".m\"><img src=\"save.png\" alt=\"Save as MATLAB file\"/></a></td>" << endl
      << "</tr>" << endl;
    f.close();
  }

  return 0;

}
