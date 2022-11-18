#include<bits/stdc++.h>
#include<pthread.h>
#include<stdlib.h>
#include<unistd.h>
#include<dirent.h>
#include<sys/stat.h>
#include <openssl/sha.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sstream>
#include <fstream>

using namespace std;
vector<string>splitstring(string,char);
vector<string>splitstring2(string str,char delim)
{
	std::vector<string> vec;
	int i=0;
	string s="";
	while(i<str.length())
	{
		if(str[i]==delim)
		{
			vec.push_back(s);
			s="";
			i++;
		}
		else
		{
			s=s+str[i];
			i++;
		}
	}
	vec.push_back(s);
	return vec;
}
void * serve_client(void *);

////////////////////////////////////////////////////////////////////////////////////

struct file_data
{
	string userid;
	string hash;
	string ip;
	string port;
	long int file_size;
	string grpid;
	bool isactive;
};
struct grp_file_data{
	string userid;
	string fname;
	string ip;
	string port;
	bool isactive;
};
struct group
{
	string admin;


};

map<string,vector<string>>admin_req;//map for admin to pending requests
map<string,string>user_pass;
map<string,group>grp_map;//map group id to group;
map<string,vector<string>>grp_users;//map grp id to users
map<string,vector<grp_file_data>>grp_file_map;//group_name map to file data
map<string,vector<pair<string,string>>>user_files;//map userid to to pair of grpid and filename
std::map<string, vector<file_data>> file_map;// map filename to filedata
set<string>file_set;//set contains filenames present in group
map<string,vector<pair<string,string>>>downloads;
std::vector<string> command;
string tracker1_ip,tracker1_port,tracker2_ip,tracker2_port,log_file;
pthread_t client_name;

int main(int argc,char* argv[])
{
	int opt=1;

	if(argc !=3)
	{
		cout<<"invalid arguments"<<endl;
		exit(1);
	}
	log_file="log";
	ofstream output;
	output.open(log_file,fstream::out);
	output.close();

	//populating hashmap for user-password
	ifstream inp_file("user_pass.txt");
	string line;
	while(getline(inp_file,line))
	{
		command=splitstring2(line,':');
		user_pass[command[0]]=command[1];
	}

	//////////////////////////////////////////////////////////////////////
	FILE *fp=fopen(argv[1],"r");
    	int tracker_no=atoi(argv[2]);
    	char tracker_details[21];
    	while(tracker_no--){
        fscanf(fp,"%s",tracker_details);
    	}
    	fclose(fp);
    	char *token=strtok(tracker_details,":");
    	string ip=token;
    	token=strtok(NULL,":");
    	string port_from_arg = token;//atoi(token);
	///////////////////////////////////////////////////////////////////////

	string t1_ip,t1_port;
	string t2_ip,t2_port;

	t1_ip = ip;
	t1_port = port_from_arg;
	//t1_ip="10.0.2.15",t1_port="5000";

	t2_ip="10.0.2.15",t2_port="6600";
	//cout<<t1_ip<<" "<<t1_port<<" "<<t2_ip<<" "<<" "<<t2_port<<endl;

	char* t1_ip_char=new char[t1_ip.length()+1];
	strcpy(t1_ip_char,t1_ip.c_str());

	int server_fd=socket(AF_INET,SOCK_STREAM,0);
	if(server_fd==0)
	{
		cout<<"error in connection";
		exit(1);
	}

	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        cout<<"setsockopt"<<endl;
             exit(1);
    }
    int port=stoi(t1_port);
    struct sockaddr_in addr;
	addr.sin_family=AF_INET;
	addr.sin_port=htons(port);
	addr.sin_addr.s_addr=INADDR_ANY;
	int addrlen = sizeof(sockaddr);

	if (inet_pton(AF_INET, t1_ip_char, &addr.sin_addr) <= 0)
    {
        printf("\nInvalid address/ Address not supported \n");
        //logprinter("Invalid address/ Address not supported");
        return -1;
    }


	if(bind (server_fd  , (struct sockaddr *)&addr , sizeof ( addr ) )<0)
	{
		cout<<"error in binding"<<endl;
		exit(1);
	}

	if(listen (server_fd, 34)==0)
	{
		cout<<"In  Listening state"<<endl;

	}
	else
	{
		cout<<"error in Listening"<<endl;

		exit(1);
	}

	int new_sock;

	while(1)
	{
		if((new_sock=accept( server_fd , (struct sockaddr *)&addr , (socklen_t*)&addrlen))<0)
		{
			cout<<"error in connection establishment"<<endl;

			exit(1);
		}
		int client_port_num=ntohs(addr.sin_port);
		string client_port_str=to_string(client_port_num);
		int* argument=(int *)malloc(sizeof(*argument));
		*argument=new_sock;

		if(pthread_create(&client_name,0,serve_client,argument)<0)
		{
			cout<<"could not create thread"<<endl;

			return 1;
		}



	}

	return 0;

}

std::vector<string> splitstring(string str,char c)
{
	std::vector<string> v;
	string first,second;
	for(int i=0;i<str.length();i++)
	{
		//cout<<str[i]<<endl;
		if(str[i]==c)
		{
			//cout<<<<i<<endl;
			first=str.substr(0,i);
			second=str.substr(i+1);
			break;
		}
	}

	v.push_back(first),v.push_back(second);
	return v;
}


void *serve_client(void* sock_num)
{
		int conn_sock=*((int *)sock_num);

		while(1)
		{
			char buffer[1024]={0};
			int status=read(conn_sock,buffer,1024);
			if(status==0)
				return sock_num;


			string buffer_str=string(buffer);
			command=splitstring2(buffer_str,':');
			struct file_data fdata;
			struct grp_file_data gdata;
			if(command[0]=="login")
			{

				int valid=0;

				if(user_pass[command[1]]==command[2])
				{

					valid=1;

				}
				if(valid==1)
				{
					string uid=command[1];
					std::vector<pair<string,string>> v=user_files[uid];
				if(v.size()!=0)
				{

					for(auto j=v.begin();j!=v.end();j++)
					{
						pair<string,string>p=*j;
						string logout_file=p.first;
						string logout_grp=p.second;
						std::vector<grp_file_data> logout_grp_vec=grp_file_map[logout_grp];
						for(auto k=logout_grp_vec.begin();k!=logout_grp_vec.end();k++)
						{
							struct grp_file_data gfd=*k;
							if(gfd.userid==uid)
							{
								cout<<gfd.fname<<endl;
								gfd.isactive=true;
							}
							*k=gfd;
						}
						grp_file_map[logout_grp]=logout_grp_vec;
						std::vector<file_data> logout_file_vec=file_map[logout_file];
						for(auto x=logout_file_vec.begin();x!=logout_file_vec.end();x++)
						{
							struct file_data fd=*x;
							if(fd.userid==uid)
								fd.isactive=true;
							*x=fd;

						}
						file_map[logout_file]=logout_file_vec;

					}
				}
			}

				send(conn_sock,&valid,sizeof(valid),0);

		}

			else if(command[0]=="create_user")
			{
				int valid=0;
				user_pass[command[1]]=command[2];

				/*for(auto i:user_pass)
				{
					cout<<i.first<<" "<<i.second<<endl;
				}*/
				ofstream outf;
				outf.open("user_pass.txt", ios::app|ios::out);
				outf<<command[1]<<":"<<command[2]<<endl;
				outf.close();
				valid=1;
				send(conn_sock,&valid,sizeof(valid),0);
			}
			else if(command[0]=="upload_file")
			{
				string reply;

				int flag=0;
				if(grp_map.find(command[5])==grp_map.end())
				{
					reply="group does not exist";
					char* reply_char_err=new char[reply.length()+1];
					strcpy(reply_char_err,reply.c_str());
					send(conn_sock,reply_char_err,strlen(reply_char_err),0);
					continue;
				}
				std::vector<string> gusers=grp_users[command[5]];
				int gflag=0;
				for(auto it=gusers.begin();it!=gusers.end();it++)
				{
					string ga=*it;
					if(ga==command[7])
					{
						gflag=1;
						break;
					}

				}
				if(gflag==0)
				{
					reply="you are not a member of this group";
					char* reply_char_err=new char[reply.length()+1];
					strcpy(reply_char_err,reply.c_str());
					send(conn_sock,reply_char_err,strlen(reply_char_err),0);
					continue;

				}
				string fname=command[3];
				fdata.ip=command[2];
				fdata.hash=command[1];
				fdata.port=command[4];
				fdata.grpid=command[5];
				fdata.userid=command[7];
				stringstream so(command[6]);
				long int x=0;
				so>>x;
				fdata.file_size=x;
				fdata.isactive=true;
				gdata.fname=command[3];
				gdata.ip=command[2];
				gdata.port=command[4];
				gdata.userid=command[7];
				gdata.isactive=true;
				std::vector<file_data> v=file_map[fname];
				pair<string,string>p;
				p.first=fname;
				p.second=command[5];

				if(v.size()==0)
				{

					file_map[fname].push_back(fdata);
					grp_file_map[command[5]].push_back(gdata);
					user_files[command[7]].push_back(p);
					reply="successfully shared";

				}
				else
				{
				for (auto j=v.begin();j!=v.end();j++)
				{
					struct file_data f=*j;
					if(f.userid==fdata.userid && f.grpid==fdata.grpid)
					{
						flag=1;
						break;
					}
				}
				if(flag)
				{
					reply="already shared file";

				}
				else
				{

					file_map[fname].push_back(fdata);
					grp_file_map[command[5]].push_back(gdata);
					user_files[command[7]].push_back(p);
					reply="successfully shared";

				}
			}
				char* reply_char=new char[reply.length()+1];
				strcpy(reply_char,reply.c_str());
				send(conn_sock,reply_char,strlen(reply_char),0);

			}
			else if(command[0]=="create_group")
			{
				string grp_id=command[1];
				group g;
				g.admin=command[2];
				grp_map[grp_id]=g;
				//cout<<grp_map[grp_id].admin<<endl;
				string reply;
				std::vector<string> req;
				if(grp_map.find(grp_id)!=grp_map.end())
				{
					admin_req[g.admin]=req;
					grp_users[grp_id].push_back(command[2]);
					reply="group is created is successfully";
				}
				else
				{
					reply="group cannot be created";
				}
				char* reply_char=new char[reply.length()+1];
				strcpy(reply_char,reply.c_str());
				send(conn_sock,reply_char,strlen(reply_char),0);
			}
			else if(command[0]=="list_groups")
			{
				string grps="";
				string del=":";
				if(grp_map.empty())
				{
					string reply="No groups are present";
					char* reply_char_err=new char[reply.length()+1];
					strcpy(reply_char_err,reply.c_str());
					send(conn_sock,reply_char_err,strlen(reply_char_err),0);
					continue;
				}
				for(auto i:grp_map)
				{
					cout<<i.first<<endl;
					grps=grps+i.first+del;
				}
				grps.pop_back();
				char* grps_char=new char[grps.length()+1];
				strcpy(grps_char,grps.c_str());
				send(conn_sock,grps_char,strlen(grps_char),0);
			}
			else if(command[0]=="join_group")
			{
				string reply;
				if(grp_map.find(command[1])==grp_map.end())
				{
					reply="group does not exist";
					char* reply_char_err=new char[reply.length()+1];
					strcpy(reply_char_err,reply.c_str());
					send(conn_sock,reply_char_err,strlen(reply_char_err),0);
					continue;
				}
				else
				{
					admin_req[grp_map[command[1]].admin].push_back(command[2]);
					reply="join request is send to admin";
					char* reply_char_err=new char[reply.length()+1];
					strcpy(reply_char_err,reply.c_str());
					send(conn_sock,reply_char_err,strlen(reply_char_err),0);

				}

			}
			else if(command[0]=="leave_group")
			{
				string reply;
				if(grp_map.find(command[1])==grp_map.end())
				{
					reply="group does not exist";
					char* reply_char_err=new char[reply.length()+1];
					strcpy(reply_char_err,reply.c_str());
					send(conn_sock,reply_char_err,strlen(reply_char_err),0);
					continue;
				}
				else
				{
					int user_ex_fl=0;
					vector<string>:: iterator it1;
					std::vector<string> v=grp_users[command[1]];
					int i=0;
					for(i=0;i<v.size();i++)
					{
						cout<<v[i]<<" ";
						if(v[i]==command[2])
						{
							user_ex_fl=1;
							break;
						}
					}
					cout<<i;
					cout<<endl;
					if(user_ex_fl==0)
						reply="you are not a member of this group";
					else
					{
						grp_users[command[1]].erase(grp_users[command[1]].begin()+i);
						reply="you have left group "+command[1];
					}
					char* reply_char_err=new char[reply.length()+1];
					strcpy(reply_char_err,reply.c_str());
					send(conn_sock,reply_char_err,strlen(reply_char_err),0);

				}

			}
			else if(command[0]=="list_requests")
			{
				string ad=command[2];
				string grp=command[1];
				string reply="";
				string del=":";
				//cout<<"admin "<<grp_map[grp].admin<<endl;
				//cout<<"req "<<ad<<endl;
				if(grp_map[grp].admin!=ad)
				{
					reply="you are not a admin of this group";
					char* reply_char_err=new char[reply.length()+1];
					strcpy(reply_char_err,reply.c_str());
					send(conn_sock,reply_char_err,strlen(reply_char_err),0);

				}
				else
				{
					std::vector<string> temp_v=admin_req[ad];
					if(temp_v.size()==0)
						reply="No pending requests";
					else
					{
						for(int i=0;i<temp_v.size();i++)
						{
							reply=reply+temp_v[i]+del;
						}
						reply.pop_back();
					}
					char* reply_char_err=new char[reply.length()+1];
					strcpy(reply_char_err,reply.c_str());
					send(conn_sock,reply_char_err,strlen(reply_char_err),0);
				}
			}
			else if(command[0]=="accept_request")
			{
				string grp=command[1];
				string uid=command[2];
				string ad=command[3];
				string reply="";
				if(grp_map[grp].admin!=ad)
				{
					reply="you are not a admin of this group";
					char* reply_char_err=new char[reply.length()+1];
					strcpy(reply_char_err,reply.c_str());
					send(conn_sock,reply_char_err,strlen(reply_char_err),0);

				}
				else
				{
					int user_ex_fl=0;
					grp_users[grp].push_back(uid);
					std::vector<string> v=admin_req[ad];
					int i=0;
					for(i=0;i<v.size();i++)
					{
						//cout<<v[i]<<" ";
						if(v[i]==uid)
						{
							user_ex_fl=1;
							break;
						}
					}
					cout<<i;
					cout<<endl;
					if(user_ex_fl==0)
						reply="user is not there in join_request list";
					else
					{
						admin_req[ad].erase(admin_req[ad].begin()+i);
						reply=uid+" is now member of "+grp;
					}
					char* reply_char_err=new char[reply.length()+1];
					strcpy(reply_char_err,reply.c_str());
					send(conn_sock,reply_char_err,strlen(reply_char_err),0);

				}

			}
			else if(command[0]=="list_files")
			{
				string grp=command[1];
				string reply="";
				string del=":";
				std::vector<grp_file_data> v= grp_file_map[grp];
				file_set.clear();
				if(v.size()==0)
				{
					reply="NO files are present in group to share";
				}
				else
				{
					for (auto j=v.begin();j!=v.end();j++)
					{
						//cout<<"size"<<v.size()<<endl;
						struct grp_file_data g=*j;
						if(g.isactive==true)
						{

							file_set.insert(g.fname);
						}
					}
					set<string>::iterator itr;
					for(itr=file_set.begin();itr!=file_set.end();++itr)
					{
						reply=reply+(*itr)+del;
					}
					reply.pop_back();
		        }
		        	char* reply_char_err=new char[reply.length()+1];
					strcpy(reply_char_err,reply.c_str());
					send(conn_sock,reply_char_err,strlen(reply_char_err),0);

			}
			else if(command[0]=="stop_share")
			{
				string grp=command[1];
				string stop_file=command[2];
				string uid=command[3];
				string reply="";
				int flag=0;
				std::vector<pair<string,string>> v=user_files[uid];
				if(v.size()!=0)
				{
					for(int j=0;j<v.size();j++)
					{
						pair<string,string>p=v[j];
						if(p.first==stop_file && p.second==grp)
						{
							flag=1;
							user_files[uid].erase(user_files[uid].begin()+j);
							break;
						}
					}
					if(flag==0)
					{
						reply="you have not shared this file in this group";
						char* reply_char_err=new char[reply.length()+1];
						strcpy(reply_char_err,reply.c_str());
						send(conn_sock,reply_char_err,strlen(reply_char_err),0);
						continue;
					}


				std::vector<struct grp_file_data> v1=grp_file_map[grp];
				for(int j=0;j<v1.size();j++)
				{
					struct grp_file_data g=v1[j];
					if(g.userid==uid && g.fname==stop_file)
					{
						grp_file_map[grp].erase(grp_file_map[grp].begin()+j);
						break;
					}
				}
				std::vector<struct file_data> v2=file_map[stop_file];
				for(int j=0;j<v1.size();j++)
				{
					struct file_data f=v2[j];
					if(f.userid==uid && f.grpid==grp)
					{
						file_map[stop_file].erase(file_map[stop_file].begin()+j);
						break;
					}
				}
				reply="file is now not sharable in "+grp;

			}
			else{
				reply="You have not shared any file yet";
			}
				char* reply_char_err=new char[reply.length()+1];
				strcpy(reply_char_err,reply.c_str());
				send(conn_sock,reply_char_err,strlen(reply_char_err),0);

			}
			else if(command[0]=="download_file")
			{
				string grp=command[2];
				string down_file=command[1];
				string reply="";
				string del=":";
				vector<struct file_data> v=file_map[down_file];
				if(v.size()!=0)
				{
					string fsize="";
					for(auto j=v.begin();j!=v.end();j++)
					{
						struct file_data f=*j;
						fsize=to_string(f.file_size);
						if(f.grpid==grp && f.isactive==true)
						{
							reply=reply+f.ip+del+f.port+"|";
						}
					}
					if(reply.length()>0)
						reply=reply+fsize;
				}
				else
					reply="file is not there in group";

				char* reply_char_err=new char[reply.length()+1];
				strcpy(reply_char_err,reply.c_str());
				send(conn_sock,reply_char_err,strlen(reply_char_err),0);


			}
			else if(command[0]=="logout")
			{
				string uid=command[1];
				string reply="";
				std::vector<pair<string,string>> v=user_files[uid];
				if(v.size()!=0)
				{
					reply="logout successfull";
					for(auto j=v.begin();j!=v.end();j++)
					{
						pair<string,string>p=*j;
						string logout_file=p.first;
						string logout_grp=p.second;
						std::vector<grp_file_data> logout_grp_vec=grp_file_map[logout_grp];
						for(auto k=logout_grp_vec.begin();k!=logout_grp_vec.end();k++)
						{
							struct grp_file_data gfd=*k;
							if(gfd.userid==uid)
							{
								cout<<gfd.fname<<endl;
								gfd.isactive=false;
							}
							*k=gfd;
						}
						grp_file_map[logout_grp]=logout_grp_vec;
						std::vector<file_data> logout_file_vec=file_map[logout_file];
						for(auto x=logout_file_vec.begin();x!=logout_file_vec.end();x++)
						{
							struct file_data fd=*x;
							if(fd.userid==uid)
								fd.isactive=false;
							*x=fd;

						}
						file_map[logout_file]=logout_file_vec;

					}

				}
				else
				{

					reply="logout successfully";
				}
				char* reply_char_err=new char[reply.length()+1];
				strcpy(reply_char_err,reply.c_str());
				send(conn_sock,reply_char_err,strlen(reply_char_err),0);


			}
			else if(command[0]=="show_downloads")
			{
				string reply="";
				std::vector<pair<string,string>> v=downloads[command[1]];
				for(auto j=v.begin();j!=v.end();j++)
				{
					pair<string,string>p=*j;
					reply=reply+p.first+":"+p.second+"|";
				}
				reply.pop_back();
				char* reply_char_err=new char[reply.length()+1];
				strcpy(reply_char_err,reply.c_str());
				send(conn_sock,reply_char_err,strlen(reply_char_err),0);
			}
			else if(command[0]=="add_download")
			{
				pair<string,string>p;
				p.first=command[2];
				p.second=command[3];
				downloads[command[1]].push_back(p);
			}

		}


	return sock_num;
}

