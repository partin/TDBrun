import os
import sys
import re
import time
import ConfigParser

from urllib import urlretrieve

scriptdir = os.path.dirname(__file__)
if len(scriptdir) > 0:
  os.chdir(scriptdir)

config = ConfigParser.ConfigParser()
config.read('config.ini')

tcxpath = config.get('Paths', 'tcxpath')
parserpath = config.get('Paths', 'parserpath')
indexfile = config.get('Paths', 'indexfile')
dest = config.get('Paths', 'dest')

def getPadded(num):
  string = str(num)
  if len(string) < 2:
    string = '0' + string
  return string

def readfile(filename):
  f = open(filename, 'r')
  lines = f.readlines()
  f.close()
  return lines

def insertlines(source, lines):
  f = open(source, 'r')
  newlines = []
  for line in f:
    newlines.append(line)
    if 'INSERT HERE' in line:
      for insertline in lines:
        newlines.append(insertline)
  return newlines

def writefile(lines, filename):
  f = open(filename, 'w')
  f.writelines(lines)
  f.close()
  
if len(sys.argv) < 2:
  dir = tcxpath
  localtime = time.localtime(time.time())
  datestr = str(localtime.tm_year) + '-' + getPadded(localtime.tm_mon) + "-" + getPadded(localtime.tm_mday)
  dirList = os.listdir(dir)
  files = []
  for file in dirList:
    m = re.match(datestr + '-.*\.TCX', file)
    if m:
      files.append(dir + "\\" + file)
else:
  m = re.match('.*(....)-(..)-(..)-(......)\.TCX', sys.argv[1])
  if m:
    datestr = m.group(1)+"-"+m.group(2)+"-"+m.group(3)
  else:
    print "bad filename: ", sys.argv[1]
    exit(0)
  files = [sys.argv[1]]

for fname in files:
  print fname
  os.system(parserpath + " " + fname)
  if not os.path.exists("index_line.txt"): 
    print "error: no index_line.txt"
    exit
  lines=readfile("index_line.txt")
  for line in lines: print line,
  (a,b)=urlretrieve(indexfile, "index.tmp")
  if not os.path.exists("index_line.txt"): 
    print "error: no index.tmp"
    exit
  newlines=insertlines("index.tmp", lines)
  writefile(newlines, "index.html")
  os.system("scp run" + datestr + ".html run" + datestr + ".m index.html " + dest)

