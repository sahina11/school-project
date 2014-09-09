#include "Helper.hpp"

unsigned int ip_strToInt(string ipv4){
	int end,begin;
	begin = 0;

	unsigned int res;
	char *p;
	p = (char *)&res;

	for(int i=0;i<4;i++){
		end = ipv4.find(".",begin+1);
		*(p+3-i) =stoi(ipv4.substr(begin,end-begin));
		begin = end+1;
	}

	return res;
}

string ip_intToStr(int ipv4){
	string res = "";
	int ip = ipv4;
	char *p;
	
	p=(char *)&ip;

	for(int i=3;i>0;i--){
		res.append(to_string((long long)int(*(p+i)) & 0x000000ff)); // need this mask to filter out weired part
		res.append(".");
	}

	res.append(to_string((long long)int(*(p)) & 0x000000ff));

	return res;
}

unsigned int reverse(register unsigned int x){
    x = (((x & 0xaaaaaaaa) >> 1) | ((x & 0x55555555) << 1));
    x = (((x & 0xcccccccc) >> 2) | ((x & 0x33333333) << 2));
    x = (((x & 0xf0f0f0f0) >> 4) | ((x & 0x0f0f0f0f) << 4));
    x = (((x & 0xff00ff00) >> 8) | ((x & 0x00ff00ff) << 8));
    return((x >> 16) | (x << 16));
}

unsigned int hashString(string file_name){
	int round = int(file_name.length() / 4) + 1; //how much round to divide the string by 4
	int begin = 0; //for substr
	unsigned int x; //fill in x byte by byte for every char
	string part; //placeholder for substr
	char *p; //used to fill in x one byte at a time
	p = (char *)&x;
	int add_zero; //how much zero have to filled up
	unsigned int res; //result to return

	for(int i = 1; i <= round; i++){
		part = file_name.substr(begin,4);

		if (part.length() == 4){
			for (int j = 3; j>=0; j--){
				*(p+j) = part[3-j];
			}
			if ( i%2) x = reverse(x); //reverse odd substr int
		}
		else{ //incomplete substr: len < 4
			add_zero = 4 - part.length(); 
			for (int j = 3; j>=add_zero; j--){ //fill in char
				*(p+j) = part[3-j];
			}

			for (int j = 0; j<add_zero; j++){ //fill in 0
				*(p+j) = 0;
			}
			if ( i%2) x = reverse(x);
		}

		if (i ==1) res = x;
		else res ^= x; //xor each int

		begin +=4; //update substr starting postion
	}
	return res;
}

string find_ipv4_str(string line){
	int begin, end;

	begin = line.find("\"");
	end = line.find("\"", begin+1);
	
	return line.substr(begin+1, end-begin-1);
}

string find_file_name(string line){
	int begin, end;

	begin = line.find("\"");
	end = line.find("\"", begin+1);
	begin = line.find("\"",end+1);
	end = line.find("\"", begin+1);

	return line.substr(begin+1, end-begin-1);
}

void print_res_list(list<string> res){
	cout<<"\t";
	for(list<string>::iterator it=res.begin();
		it != res.end(); it++){
		cout<<" -> "<<*it;
	}
	cout<<endl;
}