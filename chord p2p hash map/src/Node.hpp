#ifndef NODE_HPP
#define NODE_HPP

#include "Helper.hpp"
class Node {
	public:
		int add_finger_entry(unsigned int index, unsigned int value); 
		int update_finger_table( 
			unsigned int predecessor_of_theNode, 
			unsigned int theNode, 
			unsigned int update_value); 
		

		int add_file_entry(string file_name, unsigned int ipv4_int);
		int delete_file_entry(string file_name, unsigned int ipv4_int);
		int clear_file_table(void); 

		bool is_file_exist(string file_name);
		unsigned int finger_lookup(unsigned int file_hash);


	 	void display_finger_table(); 
	 	void display_file_table();
	 	void display();

	 	/*newnode's successor has to return a list of <file,ip> pair, 
	 	the file hash range from newnode's predecessor to newnode*/
	 	list< pair<string,unsigned int> > get_file_entry_list(unsigned int predecessor_of_newNode, unsigned int newNode);

	 	//when removing node, dump all you file out as a list
	 	list< pair<string,unsigned int> > get_all_file_entry_list(unsigned int remove_node);
	 	
	 private:
	 	map<unsigned int,unsigned int> finger_table;
	 	map<string,unsigned int> file_table;
		
};

#endif