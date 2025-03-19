#include <dirent.h>
#include <unistd.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>

#include "linux_parser.h"

using std::stof;
using std::string;
using std::to_string;
using std::vector;

// DONE: An example of how to read data from the filesystem
string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

// DONE: An example of how to read data from the filesystem
string LinuxParser::Kernel() {
  string os, version, kernel;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  return kernel;
}

// BONUS: Update this to use std::filesystem
vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}

// TODO: Read and return the system memory utilization
float LinuxParser::MemoryUtilization() 
{ 
  int memTotal = 0, memFree = 0;
  std::ifstream fileStream(kProcDirectory + kMeminfoFilename);
  if(fileStream.is_open())
  {
    string line;
    while (getline(fileStream, line)) {
      string key;
      float value;
      std::istringstream lstream(line);
      
      lstream >> key >> value;
      if(key == "MemTotal:")
        memTotal = value;
      else if(key == "MemFree:")
        memFree = value;
    }
  }
  else
  {
      std::cout << "FILE NOT FOUND \n";
  }

  if(memTotal > 0)
    return float((memTotal - memFree) / ((float) memTotal));
  else
    return 0.0; 
}

// TODO: Read and return the system uptime
long LinuxParser::UpTime() 
{ 
  long uptime = 0;
  std::ifstream fileStream(kProcDirectory + kUptimeFilename);
  if(fileStream.is_open())
  {
    string line;
    std::getline(fileStream, line);
    std::istringstream lstream(line);
    lstream >> uptime;
  }
  return uptime; 
}

// TODO: Read and return the number of jiffies for the system
long LinuxParser::Jiffies() 
{ 
  long jiffies = 0;
  std::ifstream fileStream(kProcDirectory + kStatFilename);
  if(fileStream.is_open())
  {
    string line, cpu;
    long user, nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice;

    std::getline(fileStream, line);
    std::istringstream lstream(line);
    lstream >> cpu >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal >> guest >> guest_nice;
    jiffies = user + nice + system + idle + iowait + irq + softirq + steal;
  }

  return jiffies;
}

// TODO: Read and return the number of active jiffies for a PID
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::ActiveJiffies(int pid) 
{
  long jiffies = 0;
  std::ifstream fileStream(kProcDirectory + to_string(pid) + kStatFilename);
  if(fileStream.is_open())
  {
    string line, col, starttime;
    long utime, stime, cutime, cstime;
    
    getline(fileStream,line);
    std::stringstream linestream(line);
    for(int i=0; i<13; i++)
    {
      linestream >> col;
    }
    linestream >> utime >> stime >> cutime >> cstime;
    jiffies = utime + stime + cutime + cstime;
  }
  return jiffies;
}

// TODO: Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() 
{
  long active_jiffies = 0;
  std::ifstream fileStream(kProcDirectory + kStatFilename);
  if(fileStream.is_open())
  {
    std::string line, cpu;
    long user, nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice;

    std::getline(fileStream,line);
    std::istringstream linestream(line);
    linestream >> cpu >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal >> guest >> guest_nice;

    active_jiffies = user + nice + system + irq + softirq + steal;
  }

  return active_jiffies;
}  

// TODO: Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() 
{
  long idle_jiffies = 0;
  std::ifstream fileStream(kProcDirectory + kStatFilename);
  if(fileStream.is_open())
  {
    std::string line, cpu;
    long user, nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice;

    std::getline(fileStream,line);
    std::istringstream linestream(line);
    linestream >> cpu >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal >> guest >> guest_nice;

    idle_jiffies = idle + iowait;
  }

  return idle_jiffies;
}
// TODO: Read and return CPU utilization
float LinuxParser::CpuUtilization() 
{
  float utilization = 0.0;
  static long pre_jiffies = 0;
  static long pre_active_jiffies = 0;
  // Current values.
  long jiffies = Jiffies();
  long active_jiffies = ActiveJiffies();

  if(jiffies-pre_jiffies != 0)
  {
    utilization = (float)(active_jiffies-pre_active_jiffies)/(jiffies-pre_jiffies);
  }
  // Store the previous values.
  pre_jiffies = jiffies;
  pre_active_jiffies = active_jiffies;
  return utilization;
}

// TODO: Read and return the total number of processes
int LinuxParser::TotalProcesses() 
{
  int totalProcesses = 0;
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    string line, key, value;
    while (getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "processes") {
          totalProcesses = stoi(value);
          break;
        }
      }
    }
    filestream.close();
  }else{
      std::cout << "FILE NOT FOUND \n";
  }
  return totalProcesses;
}  

// TODO: Read and return the number of running processes
int LinuxParser::RunningProcesses() 
{
  int runningProcesses = 0;
    std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    string line, key, value;
    while (getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "procs_running") {
          runningProcesses = stoi(value);
          break;
        }
      }
    }
    filestream.close();
  }else{
      std::cout << "FILE NOT FOUND \n";
  }
  return runningProcesses;
}

// TODO: Read and return the command associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Command(int pid)
{
  string line="";
  std::ifstream filestream(kProcDirectory + to_string(pid) +
                           kCmdlineFilename);
  if (filestream.is_open()) {
    getline(filestream, line);
    filestream.close();
  }else{
      std::cout << "FILE NOT FOUND \n";
  }
  return line;
}

// TODO: Read and return the memory used by a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Ram(int pid)
{
  int ram = 0;
  std::ifstream filestream(kProcDirectory + to_string(pid) +
                           kStatusFilename);
  if (filestream.is_open()) {
    string line, key, value;
    while (getline(filestream, line)) {
      replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "VmSize") {
          ram = stoi(value) / 1000;
        }
      }
    }
    filestream.close();
  }else{
      std::cout << "FILE NOT FOUND \n";
  }
  return to_string(ram);
}

// TODO: Read and return the user ID associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Uid(int pid)
{
  string uid = "";
  string line, key, value;
  std::ifstream filestream(kProcDirectory + to_string(pid) +
                           kStatusFilename);
  if (filestream.is_open()) {
    while (getline(filestream, line)) {
      replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "Uid") {
          uid = value;
          break;
        }
      }
    }
    filestream.close();
  }else{
      std::cout << "FILE NOT FOUND \n";
  }
  return uid;
}  

// TODO: Read and return the user associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::User(int pid)
{
  string user = "u";
  string uid_of_pid = Uid(pid);
  std::ifstream filestream(kPasswordPath);
  if (filestream.is_open()) {
    string line;
    while (getline(filestream, line)) {
      replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      string first_val, x, sec_val;
      while (linestream >> first_val >> x >> sec_val) {
        if (sec_val == uid_of_pid) {
          user = first_val;
          break;
        }
      }
    }
    filestream.close();
  }else{
      std::cout << "FILE NOT FOUND \n";
  }
  return user;
}

// TODO: Read and return the uptime of a process
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::UpTime(int pid) 
{ 
  long clockTicks = 0;
  std::ifstream filestream(kProcDirectory + to_string(pid) +
                           kStatFilename);
  if (filestream.is_open()) {
    string line, key, value;
    while (getline(filestream, line)) {
      replace(line.begin(), line.end(), '(', '_');
      replace(line.begin(), line.end(), ')', '_');
      replace(line.begin(), line.end(), '-', '_');
      std::istringstream linestream(line);
      std::istream_iterator<string> beg(linestream), end;
      vector<string> vec(beg, end);
      if(vec.size() > 22)
        clockTicks = stol(vec[21]);
    }
    filestream.close();
  }
  else
  {
      std::cout << (kProcDirectory + to_string(pid) + kStatFilename) << " FILE NOT FOUND \n";
  }
  return clockTicks / sysconf(_SC_CLK_TCK);
}