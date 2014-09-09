#include "Node.hpp"


int Node::add_finger_entry(unsigned int index, unsigned int value){
	if (finger_table.find(index) != finger_table.end()) {
		cout<<"Trying to add dulicate index in finger_table! Exit now."<<endl;
		exit(-1);
	}

	finger_table[index] = value;
	return 0;
}

int Node::update_finger_table(unsigned int predecessor_of_theNode, unsigned int theNode, unsigned int update_value){
	for(map<unsigned int,unsigned int>::iterator it=finger_table.begin();
		it != finger_table.end(); it++){
		
		//to deal with case like thenode = 1, predecessor = 14, jsut cross the 0 point of the circle
		if (theNode > predecessor_of_theNode){
			if (it->first > predecessor_of_theNode && it->first <= theNode){
				finger_table[it->first] = update_value;
			}
		}
		else if (theNode < predecessor_of_theNode){
			if (it->first > predecessor_of_theNode || it->first <= theNode){
				finger_table[it->first] = update_value;
			}
		}
	}
	return 0;
}

int Node::add_file_entry(string file_name, unsigned int ipv4_int){
	if (file_table.find(file_name) != file_table.end()) {
		cout<<"Trying to add dulicate file_name in file_table! Exit now."<<endl;
		exit(-1);
	}

	file_table[file_name] = ipv4_int;
	return 0;
}

int Node::delete_file_entry(string file_name, unsigned int ipv4_int){
	map<string,unsigned int>::iterator it_file = file_table.find(file_name);

	if ( it_file == file_table.end()) {
		cout<<"Trying to delete a file that doesn't exist in file_table! Exit now."<<endl;
		exit(-1);
	}

	if ( it_file->second != ipv4_int){
		cout<<"You are not the user who inserted the item. No right to delete it! Exit now."<<endl;
		exit(-1);
	}

	file_table.erase(file_name);
	return 0;
}

int Node::clear_file_table(void){
	file_table.clear();
	return 0;
}

bool Node::is_file_exist(string file_name){
	if (file_table.find(file_name) == file_table.end())
		return false;
	else return true;
}

list< pair<string,unsigned int> > Node::get_file_entry_list(unsigned int predecessor_of_newNode, unsigned int newNode){
	list< pair<string,unsigned int> > res;

	if (file_table.empty()){ //empty table returns empty list
		return res;
	}

	for ( map<string,unsigned int>::iterator it_file = file_table.begin();
		it_file != file_table.end(); it_file++){

		unsigned int file_hash = hashString(it_file->first);
		
		if (newNode > predecessor_of_newNode){
			if(file_hash > predecessor_of_newNode && file_hash <= newNode){
				res.push_back( make_pair(it_file->first,it_file->second) );
			}
		}
		else if(newNode < predecessor_of_newNode){
			if(file_hash > predecessor_of_newNode || file_hash <= newNode){
				res.push_back( make_pair(it_file->first,it_file->second) );
			}
		}
	}

	return res;
}

list< pair<string,unsigned int> > Node::get_all_file_entry_list(unsigned int remove_node){
	list< pair<string,unsigned int> > res;

	if (file_table.empty()){ //empty table returns empty list
		return res;
	}

	for ( map<string,unsigned int>::iterator it_file = file_table.begin();
		it_file != file_table.end(); it_file++){

		res.push_back( make_pair(it_file->first,it_file->second) );
	}

	return res;
}

void Node::display_finger_table(void){
	if ( finger_table.empty()){
		cout<<"Empty finger_table."<<endl;
	}
	else{
		cout<<"finger table:"<<endl;

		int j=0;
		for(map<unsigned int,unsigned int>::iterator it = finger_table.begin();
			it != finger_table.end(); it++){
			cout << "j(" << dec << j++ << "):" 
					<<ip_intToStr(it->first) << " => " 
					<<ip_intToStr(it->second) << '\n';
		}
	}
}

void Node::display_file_table(void){
	if (file_table.empty()) cout<<"Empty file_table."<<endl;
	else{
		cout<<"file table:"<<endl;

		for(map<string,unsigned int>::iterator it = file_table.begin();
			it != file_table.end(); it++){
			cout << it->first << " => " << ip_intToStr(it->second) << '\n';
		}
	}
}

void Node::display(void){
	Node::display_finger_table();
	Node::display_file_table();
}

unsigned int Node::finger_lookup(unsigned int file_hash){
	
	map<unsigned int,unsigned int>::iterator it_begin = finger_table.begin();
	map<unsigned int,unsigned int>::iterator it_last = finger_table.end();
	it_last--;


	if(file_hash > it_last->first){
		return it_last->second;
	}
	else if (file_hash < it_begin->first){
		return it_begin->second;
	}
	else{
		map<unsigned int,unsigned int>::iterator it = finger_table.upper_bound(file_hash);
		it--;
		return it->second;
	}
}