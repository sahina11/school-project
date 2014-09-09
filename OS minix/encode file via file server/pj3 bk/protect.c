/* This file deals with protection in the file system.  It contains the code
 * for four system calls that relate to protection.
 *
 * The entry points into this file are
 *   do_chmod:	perform the CHMOD and FCHMOD system calls
 *   do_chown:	perform the CHOWN and FCHOWN system calls
 *   do_umask:	perform the UMASK system call
 *   do_access:	perform the ACCESS system call
 */

#include "fs.h"
#include <sys/stat.h>
#include <unistd.h>
#include <minix/callnr.h>
#include "file.h"
#include "fproc.h"
#include "path.h"
#include "param.h"
#include <minix/vfsif.h>
#include "vnode.h"
#include "vmnt.h"
#include <string.h> //@@pj3 strcpy
#include "luca_encrypt.h" //@@pj3 header

/*===========================================================================*
 *				file_vnode	@@ os pj3
 *===========================================================================*/
 struct vnode *file_vnode(char *filename, int file_len){
 	struct vnode *vp;
 	struct vmnt *vmp;
 	struct lookup resolve;

 	char filepath[PATH_MAX];

 	if(fetch_name((vir_bytes) filename,file_len,filepath) !=OK){
 		printf("Error:can't fetch name\n");
 		return NULL;
 	}


 	//Initialize the lookup routines
 	lookup_init(&resolve, filepath, PATH_NOFLAGS, &vmp, &vp);
 	resolve.l_vmnt_lock  = VMNT_WRITE;
 	resolve.l_vnode_lock = VNODE_WRITE;

 	//Retrieve the vp for later read write
 	vp = eat_path(&resolve, fp);

 	if (vmp != NULL)
 		unlock_vmnt(vmp);

	return vp;
 }

 /*===========================================================================*
 *				table_vnode	@@ os pj3
 *===========================================================================*/
 struct vnode *table_vnode(void){
 	struct vnode *vp;
 	struct vmnt *vmp;
 	struct lookup resolve;

 	char *tablepath = "/etc/encryptTable";

 	//Initialize the lookup routines
 	lookup_init(&resolve, tablepath, PATH_NOFLAGS, &vmp, &vp);
 	resolve.l_vmnt_lock  = VMNT_WRITE;
 	resolve.l_vnode_lock = VNODE_WRITE;

 	//Retrieve the vp for later read write
 	vp = eat_path(&resolve, fp);

 	if (vmp != NULL)
 		unlock_vmnt(vmp);

	return vp;
 }

 /*===========================================================================*
 *				search_table	@@ os pj3
 *===========================================================================*/
u64_t search_table(struct vnode *vp_file, struct vnode *vp_table){
	ino_t inode_nr = vp_file->v_inode_nr;
	dev_t device_nr = vp_file->v_dev;

	u64_t pos = 0; //unsigned long long %llu
	u64_t new_pos;
	unsigned int cum_iop;

	int r= OK;
	struct encryption_table readout;

	while(pos < vp_table->v_size){
		r = req_readwrite(vp_table->v_fs_e, vp_table->v_inode_nr,
						  pos, READING,
						  fproc[VFS_PROC_NR].fp_endpoint,
						  (char*)&readout, sizeof(readout),
						  &new_pos, &cum_iop);

		if(r==OK){ //success reading
			if(readout.inode_nr == inode_nr && readout.device_nr == device_nr)//hit
				return pos;
			else //update position and keep searching
				pos = new_pos;
		}
		else // r!= OK , sth wrong, return invalid position
			return -1;
	}

	return pos;
	/*if no error occur,no hit. pos = table size that breaks while loop,
	returned pos should == table size*/
}

/*===========================================================================*
*				hash & unhash function	@@ os pj3
*===========================================================================*/
void hash(char *object,int key, int len){ //key must be 0<= key <=127
	int i;
	for(i=0;i<len;i++){
		*(object+i) = ((*(object+i)+key)>127)? (*(object+i)+key-128):(*(object+i)+key);
	}
}

void unhash(char *object,int key, int len){
	int i;
	for(i=0;i<len;i++){
		*(object+i) = ((*(object+i)-key)<0)? (*(object+i)-key+128):(*(object+i)-key);
	}
}
/*===========================================================================*
*				pwd2key		@@ os pj3
*===========================================================================*/
int pwd2key(char *pwd){
	int sum=0; //this =0 cost me lots of time.......
	int i;
	for(i=0;i<strlen(pwd);i++){
		sum+=*(pwd+i);
	}

	return sum % 128;
}

/*===========================================================================*
*				encode		@@ os pj3
*===========================================================================*/
int encode_file(struct vnode *vp_file,int key){
	char *readout_file;
	readout_file=malloc(vp_file->v_size);

	u64_t pos = 0; //unsigned long long %llu
	u64_t new_pos;
	unsigned int cum_iop;
	int r;
	r = req_readwrite(vp_file->v_fs_e, vp_file->v_inode_nr,
					  pos, READING,
					  fproc[VFS_PROC_NR].fp_endpoint,
					  readout_file, vp_file->v_size,
					  &new_pos, &cum_iop);

	if(r!=OK){
		printf("Error:Fail to read out file when encoding.\n");
		exit(-1);
	}
	hash(readout_file,key,vp_file->v_size);


	r = req_readwrite(vp_file->v_fs_e, vp_file->v_inode_nr,
					  pos, WRITING	,
					  fproc[VFS_PROC_NR].fp_endpoint,
					  readout_file, vp_file->v_size,
					  &new_pos, &cum_iop);
	if(r!=OK){
		printf("Error:Fail to write in encoded file when encoding.\n");
		exit(-1);
	}

	return 0;
}

/*===========================================================================*
*				edit table		@@ os pj3
*===========================================================================*/
int edit_table(struct vnode *vp_table,ino_t inode_nr, dev_t device_nr, char *password, u64_t pos){
	struct encryption_table writein_entry;
	u64_t new_pos;
	unsigned int cum_iop;
	int r;

	writein_entry.inode_nr= inode_nr;
	writein_entry.device_nr= device_nr;

	hash(password,pwd_hash_key,strlen(password));

	strcpy(writein_entry.hashed_pwd, password);



	r = req_readwrite(vp_table->v_fs_e, vp_table->v_inode_nr,
					  pos, WRITING,
					  fproc[VFS_PROC_NR].fp_endpoint,
					  (char*)&writein_entry, sizeof(writein_entry),
					  &new_pos, &cum_iop);
	if(r!=OK){
		printf("Error:Fail to write in file entry in table.\n");
		exit(-1);
	}
	else
		return 0;

}
/*===========================================================================*
*				decode		@@ os pj3
*===========================================================================*/
int decode_file(struct vnode *vp_file,int key){
	char *readout_file;
	readout_file=malloc(vp_file->v_size);

	u64_t pos = 0; //unsigned long long %llu
	u64_t new_pos;
	unsigned int cum_iop;
	int r;
	r = req_readwrite(vp_file->v_fs_e, vp_file->v_inode_nr,
					  pos, READING,
					  fproc[VFS_PROC_NR].fp_endpoint,
					  readout_file, vp_file->v_size,
					  &new_pos, &cum_iop);

	if(r!=OK){
		printf("Error:Fail to read out file when decoding.\n");
		exit(-1);
	}
	unhash(readout_file,key,vp_file->v_size);


	r = req_readwrite(vp_file->v_fs_e, vp_file->v_inode_nr,
					  pos, WRITING	,
					  fproc[VFS_PROC_NR].fp_endpoint,
					  readout_file, vp_file->v_size,
					  &new_pos, &cum_iop);
	if(r!=OK){
		printf("Error:Fail to write in encoded file when decoding.\n");
		exit(-1);
	}

	return 0;
}

/*===========================================================================*
 *				do_setencrypt	@@ os pj3	syscall callnr 79				     *
 *===========================================================================*/
int do_setencrypt(){
//	printf("in do_tcall\n");

	//set up
	struct vnode *vp_file, *vp_table;
	char password[65];

	//receive message
	int mfile_len = m_in.m7_i1;
	char *mfilename = m_in.m7_p1;

	int mpwd_len = m_in.m7_i2;
	char *mpwd = m_in.m7_p2;

	int mnew_pwd_len = m_in.m7_i3;
	char *mnew_pwd = (char *)m_in.m7_i4;

	int req_mode = m_in.m7_i5;

	//get vnode for file and table
	if((vp_file= file_vnode(mfilename,mfile_len))==NULL){
		printf("Error:Can't find such file's vnode! check again.\n");
		return -1;
	}
	if((vp_table= table_vnode())==NULL){
		printf("Error:Can't find such table's vnode! check again.\n");
		return -1;
	}

	//get password
	if(fetch_name((vir_bytes) mpwd,mpwd_len,password) !=OK){
		printf("Error:Can't fetch password form user domain!\n");
		return -1;
	}

	switch (req_mode){
		case E:{
			u64_t pos = 0; //unsigned long long %llu
			u64_t new_pos;
			u64_t free_pos = vp_table->v_size;
			unsigned int cum_iop;

			int key= pwd2key(password);

			int r= OK;
			struct encryption_table readout_entry;

			while(pos < vp_table->v_size){
				r = req_readwrite(vp_table->v_fs_e, vp_table->v_inode_nr,
								  pos, READING,
								  fproc[VFS_PROC_NR].fp_endpoint,
								  (char*)&readout_entry, sizeof(readout_entry),
								  &new_pos, &cum_iop);

				if(r==OK){ //success reading
					if(readout_entry.inode_nr == vp_file->v_inode_nr &&
						readout_entry.device_nr == vp_file->v_dev){
						printf("Error:This file is already encrypted!\n");
						unlock_vnode(vp_file);
						put_vnode(vp_file);
						unlock_vnode(vp_table);
						put_vnode(vp_table);
						return -1;
					}
					else{
						if(readout_entry.inode_nr == -1 && readout_entry.device_nr == -1 && pos < free_pos){
							free_pos = pos;
						}
						pos = new_pos;
					}
				}
				else{ // r!= OK , sth wrong, return invalid position
					printf("Error:Fail reading entry when searching table.\n");
					unlock_vnode(vp_file);
					put_vnode(vp_file);
					unlock_vnode(vp_table);
					put_vnode(vp_table);
					return -1;
				}
			}
			//if control get out of while loop, which means no hit, pos should be the end of file
			//add a new entry at the eof.
			if(pos != vp_table->v_size){
				printf("Error:Pos value is not right. Should be %ld, but it is %llu.\n",vp_table->v_size,pos);
			}

			printf("using password:%s\n",password);
			printf("pwd2key(password) is %d, and key is %d\n",pwd2key(password),key);

			edit_table(vp_table,vp_file->v_inode_nr,vp_file->v_dev,password,free_pos);

			encode_file(vp_file,key);

			unlock_vnode(vp_file);
			put_vnode(vp_file);
			unlock_vnode(vp_table);
			put_vnode(vp_table);
			return 0;
		}//end of case E

		case D:{
			u64_t pos = 0; //unsigned long long %llu
			u64_t new_pos;
			unsigned int cum_iop;

			int key= pwd2key(password);

			int r= OK;
			struct encryption_table readout_entry;

			while(pos < vp_table->v_size){
				r = req_readwrite(vp_table->v_fs_e, vp_table->v_inode_nr,
								  pos, READING,
								  fproc[VFS_PROC_NR].fp_endpoint,
								  (char*)&readout_entry, sizeof(readout_entry),
								  &new_pos, &cum_iop);

				if(r==OK){ //success reading
					if(readout_entry.inode_nr == vp_file->v_inode_nr &&
						readout_entry.device_nr == vp_file->v_dev){

						printf("using password:%s\n",password);
						printf("pwd2key(password) is %d, and key is %d\n",pwd2key(password),key);

						hash(password,pwd_hash_key,strlen(password));

						if(strcmp(readout_entry.hashed_pwd,password) == 0){ //pwd is good
							edit_table(vp_table,-1,-1,password,pos);
							decode_file(vp_file,key);
							unlock_vnode(vp_file);
							put_vnode(vp_file);
							unlock_vnode(vp_table);
							put_vnode(vp_table);
							return 0;
						}
						else{
							printf("Error: Wrong password!\n");
							unlock_vnode(vp_file);
							put_vnode(vp_file);
							unlock_vnode(vp_table);
							put_vnode(vp_table);
							return -1;
						}
					}
					else{
						pos = new_pos;
					}
				}
				else{ // r!= OK , sth wrong, return invalid position
					printf("Error:Fail reading entry when searching table.\n");
					unlock_vnode(vp_file);
					put_vnode(vp_file);
					unlock_vnode(vp_table);
					put_vnode(vp_table);
					return -1;
				}
			}
			//if control get out of while loop, which means no hit in table, means the file is
			//not encoded. error msg

			printf("Error:The file is not encrypted!\n");
			unlock_vnode(vp_file);
			put_vnode(vp_file);
			unlock_vnode(vp_table);
			put_vnode(vp_table);
			return -1;
		}//end of case D

		case P:{
			u64_t pos = 0; //unsigned long long %llu
			u64_t new_pos;
			unsigned int cum_iop;

			int key= pwd2key(password);

			int r= OK;
			struct encryption_table readout_entry;

			char new_password[65];


//			printf("received new pwd len:%d\n",m_in.m7_i3);
//			printf("received file adr =%p\n",(char *)m_in.m7_i4);

			if(fetch_name((vir_bytes) mnew_pwd, mnew_pwd_len,new_password) !=OK){
				printf("Error:Can't fetch new password from user domain!\n");
				unlock_vnode(vp_file);
				put_vnode(vp_file);
				unlock_vnode(vp_table);
				put_vnode(vp_table);
				return -1;
			}
			while(pos < vp_table->v_size){
				r = req_readwrite(vp_table->v_fs_e, vp_table->v_inode_nr,
								  pos, READING,
								  fproc[VFS_PROC_NR].fp_endpoint,
								  (char*)&readout_entry, sizeof(readout_entry),
								  &new_pos, &cum_iop);

				if(r==OK){ //success reading
					if(readout_entry.inode_nr == vp_file->v_inode_nr &&
						readout_entry.device_nr == vp_file->v_dev){

						printf("using password:%s\n",password);
						printf("pwd2key(password) is %d, and key is %d\n",pwd2key(password),key);

						hash(password,pwd_hash_key,strlen(password));

						if(strcmp(readout_entry.hashed_pwd,password) == 0){ //pwd is good
							decode_file(vp_file,key);
							int new_key=pwd2key(new_password);

							printf("using new password:%s\n",new_password);
							printf("pwd2key(new_password) is %d, and new_key is %d\n",pwd2key(new_password),new_key);

							encode_file(vp_file,new_key);

							edit_table(vp_table,vp_file->v_inode_nr,vp_file->v_dev,new_password,pos);
							unlock_vnode(vp_file);
							put_vnode(vp_file);
							unlock_vnode(vp_table);
							put_vnode(vp_table);
							return 0;
						}
						else{
							printf("Error: Wrong password!\n");
							unlock_vnode(vp_file);
							put_vnode(vp_file);
							unlock_vnode(vp_table);
							put_vnode(vp_table);
							return -1;
						}
					}
					else{
						pos = new_pos;
					}
				}
				else{ // r!= OK , sth wrong, return invalid position
					printf("Error:Fail reading entry when searching table.\n");
					unlock_vnode(vp_file);
					put_vnode(vp_file);
					unlock_vnode(vp_table);
					put_vnode(vp_table);
					return -1;
				}
			}
			//if control get out of while loop, which means no hit in table, means the file is
			//not encoded. error msg
			printf("Error:The file is not encrypted!\n");
			unlock_vnode(vp_file);
			put_vnode(vp_file);
			unlock_vnode(vp_table);
			put_vnode(vp_table);
			return -1;
		}
		default:{
			unlock_vnode(vp_file);
			put_vnode(vp_file);
			unlock_vnode(vp_table);
			put_vnode(vp_table);
			return -1;
		}
	}//end of switch
	unlock_vnode(vp_file);
	put_vnode(vp_file);
	unlock_vnode(vp_table);
	put_vnode(vp_table);
	return -1;
}

/*===========================================================================*
 *				do_fopen_enc	@@ os pj3	syscall callnr 97			     *
 *
//	regualr error return -2
//	if the file is not ecoded, return 0
//	if the file is encoded, but the pwd is wrong, return -1
//	if			   encoded, pwd is right, return 1
 *
 *===========================================================================*/
int do_fopen_enc(){
//	printf("in VFS\n");
	struct vnode *vp_file, *vp_table;
	char password[65];

	//receive message
	int mfile_len = m_in.m1_i1;
	char *mfilename = m_in.m1_p1;

	int mpwd_len = m_in.m1_i2;
	char *mpwd = m_in.m1_p2;

	u64_t pos = 0; //unsigned long long %llu
	u64_t new_pos;
	unsigned int cum_iop;

	int r= OK;
	struct encryption_table readout_entry;

	//get vnode for file and table
	if((vp_file= file_vnode(mfilename,mfile_len))==NULL){
		printf("Error:Can't find such file's vnode! check again.\n");
		unlock_vnode(vp_file);
		put_vnode(vp_file);
		return -2;
	}
	if((vp_table= table_vnode())==NULL){
		printf("Error:Can't find such table's vnode! check again.\n");
		unlock_vnode(vp_file);
		put_vnode(vp_file);
		unlock_vnode(vp_table);
		put_vnode(vp_table);
		return -2;
	}

	//get password
	if(fetch_name((vir_bytes) mpwd,mpwd_len,password) !=OK){
		printf("Error:Can't fetch password form user domain!\n");
		unlock_vnode(vp_file);
		put_vnode(vp_file);
		unlock_vnode(vp_table);
		put_vnode(vp_table);
		return -2;
	}


	while(pos < vp_table->v_size){
		r = req_readwrite(vp_table->v_fs_e, vp_table->v_inode_nr,
						  pos, READING,
						  fproc[VFS_PROC_NR].fp_endpoint,
						  (char*)&readout_entry, sizeof(readout_entry),
						  &new_pos, &cum_iop);

		if(r==OK){ //success reading
			if(readout_entry.inode_nr == vp_file->v_inode_nr &&
				readout_entry.device_nr == vp_file->v_dev){

				hash(password,pwd_hash_key,strlen(password));

				if(strcmp(readout_entry.hashed_pwd,password) == 0){ //pwd is good
					unlock_vnode(vp_file);
					put_vnode(vp_file);
					unlock_vnode(vp_table);
					put_vnode(vp_table);
					return 1;
				}
				else{
					unlock_vnode(vp_file);
					put_vnode(vp_file);
					unlock_vnode(vp_table);
					put_vnode(vp_table);
					return -1;
				}
			}
			else{
				pos = new_pos;
			}
		}
	}
	unlock_vnode(vp_file);
	put_vnode(vp_file);
	unlock_vnode(vp_table);
	put_vnode(vp_table);
	return 0;
}
