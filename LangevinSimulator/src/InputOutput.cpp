/**
LangevinSimulator Version 1.0

Copyright 2020-2022 Christoph Widder and Fabian Glatzel

Christoph Widder <christoph.widder[at]merkur.uni-freiburg.de>
Fabian Glatzel <fabian.glatzel[at]physik.uni-freiburg.de>

This file is part of LangevinSimulator.

LangevinSimulator is free software: you can redistribute it and/or modify it under the terms of
the GNU General Public License as published by the Free Software Foundation, either
version 3 of the License, or (at your option) any later version.

LangevinSimulator is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with LangevinSimulator.
If not, see <https://www.gnu.org/licenses/>.
**/

#include "InputOutput.hpp"

#include <iostream>
#include <fstream>
#include <iomanip>  // setprecision
#include <stdlib.h>  // atof
#include <math.h>  // sqrt
#include <set>
#include <dirent.h>
#include <stdexcept>

using namespace std;
using namespace TensorUtils;

vector<string> InputOutput::getDataFilenames(
    string file_range,
    string in_folder,
    string in_prefix)
{
    vector<string> dataFiles;
    if(file_range == "") {
        set<filesystem::path> all_paths;
        set<string> valid_paths;

        DIR *dir;
        struct dirent *ent;
        if ((dir = opendir (in_folder.c_str())) != NULL) {
            /* print all the files and directories within directory */
            while ((ent = readdir (dir)) != NULL) {
                string dummy = in_folder;
                if (dummy.back() != '/') {
                    dummy.append("/");
                }
                dummy.append(ent->d_name);
                all_paths.insert(dummy);
            }
            closedir (dir);
        } else {
            /* could not open directory */
            cout << "Error: Could not open directory" << endl;
            return dataFiles;
        }
        cout << "Found " << all_paths.size() << " paths." << endl;

        // get all valid paths from all_paths and insert into valid_paths, if the file number is part of any given range
        for(auto it=all_paths.begin(); it!=all_paths.end(); it++)
        {
            string temp = it->filename();

            if ( (in_prefix=="") || (temp.substr(0,in_prefix.length())==in_prefix))
            {
                if (in_prefix!="") {
                    temp = temp.erase(0,in_prefix.length());
                }
                if (temp.length()>4) {
                    if (temp.substr(temp.length()-4,temp.length())==".txt") {
                        temp = temp.substr(0,temp.length()-4);
                        bool path_ok = true;
                        try {
                            stoi(temp);
                        } catch (invalid_argument &e) {
                            path_ok = false;
                        }
                        if(path_ok) {
                            valid_paths.insert(*it);
                        }
                    }
                }
            }
        }
        cout << "Found " << valid_paths.size() << " valid paths." << endl;
        //store valid paths
        for(auto it=valid_paths.begin(); it!= valid_paths.end(); it++) {
            dataFiles.push_back(*it);
        }
    }
    else {
        set<filesystem::path> all_paths;
        set<string> valid_paths;
        // get string ranges from file_range command line argument
        vector<string> string_ranges;
        stringstream ss(file_range);
        string temp;
        while(getline(ss, temp, ',')) {
            string_ranges.push_back(temp);
        }
        // convert string ranges into vector<vector<int>>
        vector<vector<int>> ranges;
        vector<int> tmp_range;
        for(auto it=string_ranges.begin(); it!=string_ranges.end(); it++) {
            ss = stringstream(*it);
            tmp_range.clear();
            while(getline(ss, temp, '-')){
                try {
                    tmp_range.push_back(stoi(temp));
                } catch (invalid_argument &e) {
                    break;
                }
            }
            if (tmp_range.size() == 2) {
                ranges.push_back(tmp_range);
            }
        }

        DIR *dir;
        struct dirent *ent;
        if ((dir = opendir (in_folder.c_str())) != NULL) {
            /* print all the files and directories within directory */
            while ((ent = readdir (dir)) != NULL) {
                string dummy = in_folder;
                if (dummy.back() != '/') {
                    dummy.append("/");
                }
                dummy.append(ent->d_name);
                all_paths.insert(dummy);
            }
            closedir (dir);
        } else {
            /* could not open directory */
            cout << "Error: Could not open directory" << endl;
            return dataFiles;
        }
        cout << "Found " << all_paths.size() << " paths." << endl;

        // get all valid paths from all_paths and insert into valid_paths, if the file number is part of any given range
        for(auto it=all_paths.begin(); it!=all_paths.end(); it++)
        {
            temp = it->filename();

            if ( (in_prefix=="") || (temp.substr(0,in_prefix.length())==in_prefix))
            {
                if (in_prefix!="") {
                    temp = temp.erase(0,in_prefix.length());
                }
                if (temp.length()>4) {
                    if (temp.substr(temp.length()-4,temp.length())==".txt") {
                        temp = temp.substr(0,temp.length()-4);
                        int file_nr;
                        try {
                            file_nr = stoi(temp);
                        } catch (invalid_argument &e) {
                            break; // path does not match file format
                        }
                        // check if file number is part of a given range from file_range
                        for(auto it2=ranges.begin(); it2!=ranges.end(); it2++)
                        {
                            if ( (*it2)[0] <= file_nr && file_nr <= (*it2)[1]){
                                //cout << *it << endl;
                                valid_paths.insert(*it);
                            }
                        }
                    }
                }
            }
        }
        cout << "Found " << valid_paths.size() << " valid paths." << endl;
        // store valid paths
        for(auto it=valid_paths.begin(); it!= valid_paths.end(); it++) {
            dataFiles.push_back(*it);
        }
    }
    return dataFiles;
};

tensor<double,3> InputOutput::readTrajectories(
    vector<string> &data_files,
    double t_min,
    double t_max,
    size_t increment,
    size_t num_obs)
{
    if(data_files.size()==0)
    {
        throw invalid_argument( "Unable to load input files: No valid paths." );
    }
    tensor<double,3> trajectories;
    for (auto it=data_files.begin(); it!=data_files.end(); it++) {
        ifstream in(*it);
        string line;
        int count_lines = -1;
        while (getline(in, line))
        {
            count_lines++;
            if(count_lines%increment==0) {
                stringstream ss(line);
                double tmp;
                if(ss >> tmp && t_min <= tmp && tmp < t_max) {
                    trajectories.push_back(tmp);
                    for(size_t i=0; i<num_obs; i++) {
                        if (ss >> tmp) {
                            trajectories.push_back(tmp);
                        }
                        else {
                            cout << "Error in readTrajectories: Number of observables less than expected." << endl;
                            break;
                        }
                    }
                }
            }
        }
    }
    size_t num_ts = (trajectories.size()/data_files.size())/(num_obs+1);
    trajectories.reshape({data_files.size(), num_ts, num_obs+1});
    return trajectories;
};

tensor<double,1> InputOutput::popTimes(tensor<double,3> &trajectories)
{
    tensor<double,1> times({trajectories.shape[1]});
    for(size_t i=0; i<times.size(); i++)
    {
        times(i) = trajectories(0,i,0);
    }

    tensor<double,3> buffer({trajectories.shape[0], trajectories.shape[1],trajectories.shape[2]-1});
    for(size_t i=0; i<buffer.shape[0]; i++)
    {
        for(size_t j=0; j<buffer.shape[1]; j++)
        {
            for(size_t k=0; k<buffer.shape[2]; k++)
            {
                buffer(i,j,k) = trajectories(i,j,k+1);
            }
        }
    }
    trajectories = buffer;

    return times;
};

void InputOutput::write(
    tensor<double,1> &times,
    tensor<double,2> &traj,
    filesystem::path out_path)
{
    ofstream out;
    out << scientific;
    out << setprecision(15);
    out.open(out_path);
    for(size_t t=0;t<traj.shape[0];t++){
        out << times[t] << '\t';
        for(size_t o=0;o<traj.shape[1];o++){
            out << traj(t,o) << '\t';
        }
        out << '\n';
    }
    out.close();
}

void InputOutput::write(tensor<double,1> &times, tensor<double,4> &corr, filesystem::path out_path)
{
    ofstream out;
    out << scientific;
    out << setprecision(15);
    out.open(out_path);
    size_t num_ts = corr.shape[0];
    size_t num_obs = corr.shape[1];
    tensor<double,3> buffer;
    buffer = corr.transpose({0,2,1,3}).reshape({num_ts,num_ts,num_obs*num_obs});
    for(size_t t1=0; t1<num_ts; t1++)
    {
        for(size_t t2=0; t2<num_ts; t2++)
        {
            out << times[t1] << '\t' << times[t2];
            for(size_t k=0; k<num_obs*num_obs; k++)
            {
                out << '\t' << buffer(t1,t2,k);
            }
            out << '\n';
        }
        out << '\n';
    }
}

void InputOutput::write(tensor<double,1> &times, tensor<double,3> &corr, filesystem::path out_path)
{
    ofstream out;
    out << scientific;
    out << setprecision(15);
    out.open(out_path);
    size_t num_ts = corr.shape[0];
    size_t num_obs = corr.shape[1];
    tensor<double,2> buffer;
    buffer = corr.transpose({0,2,1}).reshape({num_ts,num_obs*num_obs});
    for(size_t t1=0; t1<num_ts; t1++)
    {
        out << times[t1];
        for(size_t k=0; k<num_obs*num_obs; k++)
        {
            out << '\t' << buffer(t1,k);
        }
        out << '\n';
    }
}
