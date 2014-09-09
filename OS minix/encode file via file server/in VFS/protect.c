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



/*===========================================================================*
 *				do_chmod				     *
 *===========================================================================*/
int do_chmod()
{
/* Perform the chmod(name, mode) and fchmod(fd, mode) system calls.
 * syscall might provide 'name' embedded in the message.
 */

  struct filp *flp;
  struct vnode *vp;
  struct vmnt *vmp;
  int r, rfd;
  mode_t result_mode;
  char fullpath[PATH_MAX];
  struct lookup resolve;
  vir_bytes vname;
  size_t vname_length;
  mode_t new_mode;

  flp = NULL;
  vname = (vir_bytes) job_m_in.name;
  vname_length = (size_t) job_m_in.name_length;
  rfd = job_m_in.fd;
  new_mode = (mode_t) job_m_in.mode;

  lookup_init(&resolve, fullpath, PATH_NOFLAGS, &vmp, &vp);
  resolve.l_vmnt_lock = VMNT_WRITE;
  resolve.l_vnode_lock = VNODE_WRITE;

  if (job_call_nr == CHMOD) {
	/* Temporarily open the file */
	if (copy_name(vname_length, fullpath) != OK) {
		/* Direct copy failed, try fetching from user space */
		if (fetch_name(vname, vname_length, fullpath) != OK)
			return(err_code);
	}
	if ((vp = eat_path(&resolve, fp)) == NULL) return(err_code);
  } else {	/* call_nr == FCHMOD */
	/* File is already opened; get a pointer to vnode from filp. */
	if ((flp = get_filp(rfd, VNODE_WRITE)) == NULL) return(err_code);
	vp = flp->filp_vno;
	dup_vnode(vp);
  }

  /* Only the owner or the super_user may change the mode of a file.
   * No one may change the mode of a file on a read-only file system.
   */
  if (vp->v_uid != fp->fp_effuid && fp->fp_effuid != SU_UID)
	r = EPERM;
  else
	r = read_only(vp);

  if (r == OK) {
	/* Now make the change. Clear setgid bit if file is not in caller's
	 * group */
	if (fp->fp_effuid != SU_UID && vp->v_gid != fp->fp_effgid)
		new_mode &= ~I_SET_GID_BIT;

	r = req_chmod(vp->v_fs_e, vp->v_inode_nr, new_mode, &result_mode);
	if (r == OK)
		vp->v_mode = result_mode;
  }

  if (job_call_nr == CHMOD) {
	unlock_vnode(vp);
	unlock_vmnt(vmp);
  } else {	/* FCHMOD */
	unlock_filp(flp);
  }

  put_vnode(vp);
  return(r);
}


/*===========================================================================*
 *				do_chown				     *
 *===========================================================================*/
int do_chown()
{
/* Perform the chown(path, owner, group) and fchmod(fd, owner, group) system
 * calls. */
  struct filp *flp;
  struct vnode *vp;
  struct vmnt *vmp;
  int r, rfd;
  uid_t uid, new_uid;
  gid_t gid, new_gid;
  mode_t new_mode;
  char fullpath[PATH_MAX];
  struct lookup resolve;
  vir_bytes vname1;
  size_t vname1_length;

  flp = NULL;
  vname1 = (vir_bytes) job_m_in.name1;
  vname1_length = (size_t) job_m_in.name1_length;
  rfd = job_m_in.fd;
  uid = job_m_in.owner;
  gid = job_m_in.group;

  lookup_init(&resolve, fullpath, PATH_NOFLAGS, &vmp, &vp);
  resolve.l_vmnt_lock = VMNT_WRITE;
  resolve.l_vnode_lock = VNODE_WRITE;

  if (job_call_nr == CHOWN) {
	/* Temporarily open the file. */
	if (fetch_name(vname1, vname1_length, fullpath) != OK)
		return(err_code);
	if ((vp = eat_path(&resolve, fp)) == NULL) return(err_code);
  } else {	/* call_nr == FCHOWN */
	/* File is already opened; get a pointer to the vnode from filp. */
	if ((flp = get_filp(rfd, VNODE_WRITE)) == NULL)
		return(err_code);
	vp = flp->filp_vno;
	dup_vnode(vp);
  }

  r = read_only(vp);
  if (r == OK) {
	/* FS is R/W. Whether call is allowed depends on ownership, etc. */
	/* The super user can do anything, so check permissions only if we're
	   a regular user. */
	if (fp->fp_effuid != SU_UID) {
		/* Regular users can only change groups of their own files. */
		if (vp->v_uid != fp->fp_effuid) r = EPERM;
		if (vp->v_uid != uid) r = EPERM;	/* no giving away */
		if (fp->fp_effgid != gid) r = EPERM;
	}
  }

  if (r == OK) {
	/* Do not change uid/gid if new uid/gid is -1. */
	new_uid = (uid == (uid_t)-1 ? vp->v_uid : uid);
	new_gid = (gid == (gid_t)-1 ? vp->v_gid : gid);

	if (new_uid > UID_MAX || new_gid > GID_MAX)
		r = EINVAL;
	else if ((r = req_chown(vp->v_fs_e, vp->v_inode_nr, new_uid, new_gid,
				&new_mode)) == OK) {
		vp->v_uid = new_uid;
		vp->v_gid = new_gid;
		vp->v_mode = new_mode;
	}
  }

  if (job_call_nr == CHOWN) {
	unlock_vnode(vp);
	unlock_vmnt(vmp);
  } else {	/* FCHOWN */
	unlock_filp(flp);
  }

  put_vnode(vp);
  return(r);
}

/*===========================================================================*
 *				do_umask				     *
 *===========================================================================*/
int do_umask()
{
/* Perform the umask(co_mode) system call. */
  mode_t complement, new_umask;

  new_umask = job_m_in.co_mode;

  complement = ~fp->fp_umask;	/* set 'r' to complement of old mask */
  fp->fp_umask = ~(new_umask & RWX_MODES);
  return(complement);		/* return complement of old mask */
}


/*===========================================================================*
 *				do_access				     *
 *===========================================================================*/
int do_access()
{
/* Perform the access(name, mode) system call.
 * syscall might provide 'name' embedded in the message.
 */
  int r;
  struct vnode *vp;
  struct vmnt *vmp;
  char fullpath[PATH_MAX];
  struct lookup resolve;
  vir_bytes vname;
  size_t vname_length;
  mode_t access;

  vname = (vir_bytes) job_m_in.name;
  vname_length = (size_t) job_m_in.name_length;
  access = job_m_in.mode;

  lookup_init(&resolve, fullpath, PATH_NOFLAGS, &vmp, &vp);
  resolve.l_vmnt_lock = VMNT_READ;
  resolve.l_vnode_lock = VNODE_READ;

  /* First check to see if the mode is correct. */
  if ( (access & ~(R_OK | W_OK | X_OK)) != 0 && access != F_OK)
	return(EINVAL);

  /* Temporarily open the file. */
  if (copy_name(vname_length, fullpath) != OK) {
	/* Direct copy failed, try fetching from user space */
	if (fetch_name(vname, vname_length, fullpath) != OK)
		return(err_code);
  }
  if ((vp = eat_path(&resolve, fp)) == NULL) return(err_code);

  r = forbidden(fp, vp, access);

  unlock_vnode(vp);
  unlock_vmnt(vmp);

  put_vnode(vp);
  return(r);
}


/*===========================================================================*
 *				forbidden				     *
 *===========================================================================*/
int forbidden(struct fproc *rfp, struct vnode *vp, mode_t access_desired)
{
/* Given a pointer to an vnode, 'vp', and the access desired, determine
 * if the access is allowed, and if not why not.  The routine looks up the
 * caller's uid in the 'fproc' table.  If access is allowed, OK is returned
 * if it is forbidden, EACCES is returned.
 */

  register mode_t bits, perm_bits;
  uid_t uid;
  gid_t gid;
  int r, shift;

  if (vp->v_uid == (uid_t) -1 || vp->v_gid == (gid_t) -1) return(EACCES);

  /* Isolate the relevant rwx bits from the mode. */
  bits = vp->v_mode;
  uid = (job_call_nr == ACCESS ? rfp->fp_realuid : rfp->fp_effuid);
  gid = (job_call_nr == ACCESS ? rfp->fp_realgid : rfp->fp_effgid);

  if (uid == SU_UID) {
	/* Grant read and write permission.  Grant search permission for
	 * directories.  Grant execute permission (for non-directories) if
	 * and only if one of the 'X' bits is set.
	 */
	if ( S_ISDIR(bits) || bits & ((X_BIT << 6) | (X_BIT << 3) | X_BIT))
		perm_bits = R_BIT | W_BIT | X_BIT;
	else
		perm_bits = R_BIT | W_BIT;
  } else {
	if (uid == vp->v_uid) shift = 6;		/* owner */
	else if (gid == vp->v_gid) shift = 3;		/* group */
	else if (in_group(fp, vp->v_gid) == OK) shift = 3; /* suppl. groups */
	else shift = 0;					/* other */
	perm_bits = (bits >> shift) & (R_BIT | W_BIT | X_BIT);
  }

  /* If access desired is not a subset of what is allowed, it is refused. */
  r = OK;
  if ((perm_bits | access_desired) != perm_bits) r = EACCES;

  /* Check to see if someone is trying to write on a file system that is
   * mounted read-only.
   */
  if (r == OK)
	if (access_desired & W_BIT)
		r = read_only(vp);

  return(r);
}

/*===========================================================================*
 *				read_only				     *
 *===========================================================================*/
int read_only(vp)
struct vnode *vp;		/* ptr to inode whose file sys is to be cked */
{
/* Check to see if the file system on which the inode 'ip' resides is mounted
 * read only.  If so, return EROFS, else return OK.
 */
  return((vp->v_vmnt->m_flags & VMNT_READONLY) ? EROFS : OK);
}
