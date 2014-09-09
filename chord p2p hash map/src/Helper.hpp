#ifndef HELPER_HPP
#define HELPER_HPP

#include <iostream>
#include <string>
#include <map>
#include <cmath>
#include <list>
#include <utility>      // std::pair, std::make_pair


using namespace std;

//convert string to 32bit int
unsigned int ip_strToInt(string ipv4);

//conver int to ipv4 string format
string ip_intToStr(int ipv4);

//reverse the bit order in int
unsigned int reverse(register unsigned int x);

//hash file name string into an int
unsigned int hashString(string file_name);

//parse to find ipv4 string in line
string find_ipv4_str(string line);

//parse to find file name string in line
string find_file_name(string line);

void print_res_list(list<string> res);


#endif