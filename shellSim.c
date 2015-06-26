/*************************************************************************
	> File Name: shellSim.c
	> Author: Ce Qi
	> Mail: 
	> Created Time: Sun 07 Jun 2015 05:10:26 PM CST
 ************************************************************************/

#include "apue.h"
#include <sys/wait.h>
#include <fcntl.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <errno.h>

typedef enum __bool { false = 0, true = 1, } bool;
typedef struct Back_Jobs{
	pid_t pid;
	char*cmd;
	int status;
}Back_Jobs;

#define MAX_BACK_NUM 10
#define SIGSTCP 20
//处理重定向和一般的命令
int redirect_normal(char *input);
//处理管道命令
int piple_command(char*input);
//统计input命令的参数个数
int count_input_num(const char *input);
//实现cd命令
void do_cd(char*argv[]);
//计算管道数量
int count_pipe_number(const char *input);
//后台处理
bool is_back(char*input);
//以下三个函数为处理一些信号的函数如，ctrl+z
void handle_sigint(int sig);
void handle_sigchld(int sig);
void handle_sigstcp(int sig);
char* trim(char*input);
bool change_symbol(char*input);

Back_Jobs *back_jobs[MAX_BACK_NUM];
bool is_Ctrl_Z = false;
int current_pid;
int back_jobs_index =1;
char Symbol[10];

int main()
{
    char* input,*input_2, buf[100],buff[100],*path;
	int i;
    int piple_count;
	Symbol[0]='$';
	Symbol[1]='\0';
	// pid_t	pid;
   // int		status;
    // Configure readline to auto-complete paths when the tab key is hit.
    signal(SIGCHLD,handle_sigchld);
	signal(SIGINT,handle_sigint);
    signal(SIGSTCP,handle_sigstcp);
	
	
	rl_bind_key('\t', rl_complete);
    
	printf("Welcome to Ce Qi's shell!\n");
	
    while(1)
    {
        // Create prompt string from user name and current working directory.
        path = getcwd(NULL,0);  //get current path
		snprintf(buff, sizeof(buff), "%s $ %s: %s ",path, "shellSim",Symbol); 
        input = readline(buff);
		

		//printf("%s",input);
        // Check for EOF.
        if (!input)
            continue;
		if((int)strlen(input)==0)
		{
			//printf("dddd");
			free(input);
			continue;
		}
		//printf("tt%d\n",(int)strlen(input));
        // Add input to history.
        add_history(input);
 
        // Do stuff...
        piple_count = count_pipe_number(input);
		//printf("isOK?\n");
		//printf("pipe num: %d\n",piple_count);
	    if(!change_symbol(input))
		{
		    if(piple_count!=0)
		    {
			    piple_command(input);
		    }
		    else
		    {
		        redirect_normal(input);
		    }
		}

		
 
        // Free input.
        free(input);
    }
}


int redirect_normal(char*input)
{
	pid_t pid;
	int i,j,t;
	int status;
	bool isBack = false;
	int input_len;
	int para_count;
	char*buf;
	char**arg_para;
	char*ofilename,*ifilename;
	int flag_out=0,flag_in=0;
	int fd_out,fd_in;
	//printf("%d",(int)strlen(input));
	
	//input[strlen(input)]='\0';
	//printf("beforeback\n");
	isBack = is_back(input);
	//printf("back\n");
	input_len = strlen(input);
	//计算参数个数
	para_count = count_input_num(input);
	if((buf=(char*)malloc((input_len+1)*(sizeof(char))))==0)
	{
		fprintf(stderr,"error! can't malloc enough space for buffer\n");
		return 0;
	}
	if((arg_para=(char**)malloc((para_count+1)*sizeof(char*)))==0)
	{
		fprintf(stderr,"error! can't malloc enough space for arg\n");   
		return 0;
	}
	if((ofilename=(char*)malloc((input_len+1)*sizeof(char)))==0)
	{
		fprintf(stderr,"error! can't malloc enough space for arg\n");   
		return 0;
	}
	if((ifilename=(char*)malloc((input_len+1)*sizeof(char)))==0)
	{
		fprintf(stderr,"error! can't malloc enough space for arg\n");   
		return 0;
	}
	//printf("hehh\n");
	ofilename[0] = '\0';
	ifilename[0] = '\0';

	for(i=0;i<input_len;i++)
	{
		if(input[i]=='>')
		{
			flag_out=1;
			break;
		}
		else if(input[i]=='<')
		{
			flag_in=1;
			break;
		}
		
	}
	i++;
	if(flag_out==1&&input[i]=='>')
	{
		flag_out = 2;
		i++;
	}
	else if(flag_in==1&&input[i]=='<')
	{
		flag_in = 2;
		i++;
	}
	//i++;
	//printf("%d\n",flag_in);
	while((input[i]==' ')&&i<input_len)
	{
		i++;
	}
    
	j = 0;
    if(flag_out>0)
	{
		while(i<=input_len)
		{
			if(input[i]=='<')
			//if(input[i]=='<'||input[i]==' ')
			{
				ofilename[j] = '\0';
				break;
				//ofilename[j]
			}
			ofilename[j] = input[i];
			j++;
			i++;
		}
	}
	if(flag_in>0)
	{
		while(i<=input_len)
		{
			if(input[i]=='>')
			//if(input[i]=='>'||input[i]==' ')
			{
				ifilename[j] = '\0';
				break;
			}
			ifilename[j] = input[i];
			j++;
			i++;
		}
	}
	j=0;
    if(i<input_len)
	{
		if(flag_out>0&&input[i]=='<')
		{
			i++;
			flag_in = 1;
			if(input[i]=='<')
			{
				flag_in = 2;
				i++;
			}
			while((input[i]==' ')&&i<input_len)
			{
				i++;
			}
			while(i<=input_len)
			{
				ifilename[j]=input[i];
				i++;
				j++;
			}
		}
		else if(flag_in>0&&input[i]=='>')
		{
			i++;
			flag_out = 1;
			if(input[i]=='>')
			{
				flag_out = 2;
				i++;
			}
			while((input[i]==' ')&&i<input_len)
			{
				i++;
			}
			while(i<=input_len)
			{
				ofilename[j]=input[i];
				i++;
				j++;
			}
		}
		else
		{
			fprintf(stderr,"find file error!\n");
			return -1;
		}

	}
    ofilename = trim(ofilename);
	ifilename = trim(ifilename);
    //printf("in:%s\n",ifilename);
	//printf("out:%s\n",ofilename);
	if(flag_in>0)
	{
		int accid;
		if((accid=access(ifilename,F_OK))!=0)
		{
			printf("No such file!\n");
			return -1;
		}
		//printf("%d\n",accid);
	}


	j=0;
	t=0;
	for(i=0;i<=input_len;i++)
	{
		if(input[i]==' '||input[i]=='<'||input[i]=='>'||input[i]=='\0')
		{

			if(j==0)
			{
				//continue;
				if(input[i]=='<'||input[i]=='>')
				{
					//printf("break\n");
					break;
				}
				else
				{
					continue;
				}
			}
			else
			{
				buf[j] = '\0';
				j++;
				arg_para[t] = (char*)malloc(sizeof(char)*j);
				strcpy(arg_para[t],buf);
				j=0;
				//printf("%s\n",arg_para[t]);
				t++;
				if(input[i]=='<'||input[i]=='>')
				{
					printf("break\n");
					break;
				}

				
			}

			/*
			if(input[i]=='>')
			{
				flag_out=1;
			}
			else if(input[i]=='<')
			{
				flag_in=1;
			}
			*/

		}
		else
		{
			if(input[i]=='&'&&input[i+1]=='\0')
			{
				isBack = true;
				continue;
			}
			buf[j] = input[i];
			j++;
		}
	}
    arg_para[t] = NULL;
	free(buf);
    
	//printf("%s",arg_para[0]);
    //different command
	if(strcmp(arg_para[0],"leave")==0)
	{
		printf("Exit shellSim \n");
		for(i=0;i<para_count;i++)
		{
			free(arg_para[i]);
		}
		free(arg_para);
        free(input);
		//printf("%s","exit command");
		exit(1);
		return 1;
	}
    
    if(strcmp(arg_para[0],"jobs")==0)
	{
		for(i=1;i<MAX_BACK_NUM;i++)
		{
			if(back_jobs[i]!=NULL)
			{
				printf("[%d]%d%d\t\t\t\t%s\n",i,back_jobs[i]->pid,back_jobs[i]->status,back_jobs[i]->cmd);
			}
		}
		

		return 1;
	}


	if(strcmp(arg_para[0],"cd")==0)
	{
		//printf("%s\n",arg_para[0]);
	    do_cd(arg_para);
		for(i=0;i<para_count;i++)
		{
			free(arg_para[i]);
		}
		free(arg_para);
       // free(input);
		//free(input);
		return 1;
	}
    //change display symbol
/*	if(strcmp(arg_para[0],"change")==0)
	{
		if(para_count>2)
		{
			printf("para is too much!");
			return 1;
		}
		for(i=0;i<min(strlen(arg_para[1]),9);i++)
		{
			Symbol[i] = arg_para[1][i];
		}
		Symbol[i] = '\0';
     	return 1;
	}
*/
	//bg command try

	if(strcmp(arg_para[0],"bg")==0)
	{
		sscanf(arg_para[1],"%d",&i);
		if(back_jobs[i]->status==0)
		{
			printf("Already done!\n");

			return 1;
		}
		current_pid = 0;
		kill(back_jobs[i]->pid,SIGCONT);
		back_jobs[i]->status = 1;
		
		
		return 1;
	}

	/***************************************************************************/


	//printf("%d\n",para_count);
    if ((pid = fork()) < 0)
    {
		printf("fork error");
    }
    else if (pid == 0) 
    {		/* child */
		//execlp(arg_para[0], arg_para, (char *)0);
		if(flag_out==1)
		{
			fd_out = open(ofilename,O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR );	
		}
		if(flag_out==2)
		{
			fd_out = open(ofilename,O_WRONLY|O_CREAT|O_APPEND, S_IRUSR|S_IWUSR );	
		}
		if(flag_in==1)
		{
			fd_in = open(ifilename,O_RDONLY, S_IRUSR|S_IWUSR);
		}
		if(flag_in==2)
		{
			fd_in = open(ifilename,O_RDONLY, S_IRUSR|S_IWUSR);
		}
		if(fd_out==-1)
		{
			printf("Open %s error!\n",ofilename);
			return -1;
		}
	    if(fd_in==-1)
		{
			printf("Open %s error!\n",ifilename);
			return -1;
		}

		if(flag_out>0)
		{
			if(dup2(fd_out,STDOUT_FILENO)==-1)
			{
				fprintf(stderr,"Redirect Out Error!\n");
				exit(1);
			}
		}
		if(flag_in>0)
		{
			if(dup2(fd_in,STDIN_FILENO)==-1)
			{
				fprintf(stderr,"Redirect Out Error!\n");
				exit(1);
			}
		}


		execvp(arg_para[0],arg_para);
		printf("couldn't execute: %s\n", input);
		exit(127);
    }
	else
	{
		//主进程开子进程去做主要的任务
		if(!isBack)
		{
			current_pid = pid;
			if((pid = waitpid(pid,&status,WUNTRACED))<0)
				printf("waitpid error!\n");
		}
	}
    /* parent */
   // if ((pid = waitpid(pid, &status, 0)) < 0)
	//	printf("waitpid error");




}
int count_input_num(const char *input)
{
    int i,j;
    int input_len = strlen(input);
    bool isFlag = false;
    j = 0;
    for(i=0;i<input_len;i++)
	{
		if(input[i]==' '||input[i]=='<'||input[i]=='>')
		{
			isFlag = false;
			continue;
		}
		else
		{
			if(!isFlag)
			{
				isFlag = true;
				j++;

			}
		}
	}
	return j;

}

void do_cd(char*argv[])
{
	//printf("%s",argv[1]);
	if(argv[1]!=NULL)
	{
		if(chdir(argv[1])<0)
		{
			switch(errno)
			{
				case ENOENT:
					fprintf(stderr,"DIRECTORY NOT FOUND\n");
					break;
				case ENOTDIR:
					fprintf(stderr,"NOT A REAL FILENAME\n");
					break;
				case ENAMETOOLONG:
					fprintf(stderr,"PATHNAME IS TOO LONG\n");
					break;
				default:
					fprintf(stderr,"UNKNOWN ERROR\n");


			}
		}
	}
	
}

int count_pipe_number(const char*input)
{
	int sum=0;
	int i;
	for(i=0;i<strlen(input);i++)
	{
		if(input[i]=='|')
		{
			sum++;
		}
	}
	return sum;
}



int piple_command(char*input)
{
	//int back=0;
	bool isBack = false;
	int input_len,piple_count;
	int i,j,t,status;
	char **arg_order;
	int **fd;
	int *child;

	isBack = is_back(input);
	input_len = strlen(input);
	piple_count = count_pipe_number(input);
    if((arg_order = (char**)malloc((piple_count+1)*sizeof(char*)))==0)
	{
		fprintf(stderr,"error! can't malloc enough space for arg\n");
		return 0;
	}
	for(i=0;i<=piple_count;i++)
	{
	
		if((arg_order[i] = (char*)malloc((input_len+1)*sizeof(char)))==0)
		{
			fprintf(stderr,"error! can't malloc enough space for arg\n");
			return 0;
		}
	}
    
	if((fd = (int**)malloc(piple_count*sizeof(int*)))==0)
	{
		
		fprintf(stderr,"error! can't malloc enough space for arg\n");
		return 0;
	}
	for(i=0;i<piple_count;i++)
	{
		if((fd[i] = (int*)malloc(2*sizeof(int)))==0)
		{

			fprintf(stderr,"error! can't malloc enough space for arg\n");
			return 0;
		}
	}

	if((child = (int*)malloc((piple_count+1)*sizeof(int)))==0)
	{

		fprintf(stderr,"error! can't malloc enough space for arg\n");
		return 0;
	}

	//divide piple order
    j=0;
	t=0;
	for(i=0;i<=input_len;i++)
	{
		if(input[i]!='|')
		{
			arg_order[t][j]=input[i];
			j++;
		}
		else
		{
			arg_order[t][j]='\0';
			j=0;
			t++;
		}
	}
	//create pipe
	for(i=0;i<piple_count;i++)
	{
		if(pipe(fd[i])==-1)
		{
			fprintf(stderr,"Open pipe fail!\n");
			return 0;
		}
	}

	//create child
	i=0;
	if(((child[i])=fork())==0)
	{
		close(fd[i][0]);
		if(fd[i][1]!=STDOUT_FILENO)
		{
			if(dup2(fd[i][1],STDOUT_FILENO)==-1)
			{
				fprintf(stderr,"Redirect Out error!\n");
				return -1;
			}
			close(fd[i][1]);

		}
		redirect_normal(arg_order[i]);
		exit(1);
	}
	else
	{
		waitpid(child[i],&status,0);
		close(fd[i][1]);
	}
	i++;
	while(i<piple_count)
	{
		if((child[i]=fork())==0)
		{
			if(fd[i][0]!=STDIN_FILENO)
			{
				if(dup2(fd[i-1][0],STDIN_FILENO)==-1)
				{
					fprintf(stderr,"Redirect In error!\n");
					return -1;
				}
				close(fd[i-1][0]);

				if(dup2(fd[i][1],STDOUT_FILENO)==-1)
				{
					fprintf(stderr,"Redirect Out error!\n");
					return -1;
				}
				close(fd[i][1]);
			}
			redirect_normal(arg_order[i]);
			exit(1);
		}
		else
		{
			waitpid(child[i],&status,0);
			close(fd[i][1]);
			i++;
		}
	}


	if((child[i]=fork())==0)
	{
		close(fd[i-1][1]);
		if(fd[i-1][0]!=STDIN_FILENO)
		{
			if(dup2(fd[i-1][0],STDIN_FILENO)==-1)
			{
				fprintf(stderr,"Redirect In error!\n");
				return -1;
			}
			close(fd[i-1][0]);
		}
		redirect_normal(arg_order[i]);
		exit(1);
	}
	else if(!isBack)
	{
		waitpid(child[i],NULL,0);
		close(fd[i-1][1]);
	}
	//free memory
	for(i=0;i<piple_count;i++)
	{
		free(fd[i]);
	}
	free(fd);
	for(i=0;i<piple_count;i++)
	{
		free(arg_order[i]);
	}
	free(arg_order);
	free(child);
	return 1;

}

bool is_back(char*input)
{
	int input_len = strlen(input);
	if(input[input_len]=='&')
	{
		input[input_len] = '\0';
		return true;
	}
	else
		return false;
}

void handle_sigint(int sig)
{
	return;
}


void handle_sigchld(int sig)
{
	//printf("sigchld\n");
	int i = 1;
	if(!is_Ctrl_Z)
	{
		for(i=1;i<MAX_BACK_NUM;i++)
		{
			if(back_jobs[i]==NULL)
			{
				continue;
			}
			if(back_jobs[i]->pid == current_pid)
			{
				back_jobs[i]->status = 0;
				break;
			}
		}

	}
	else
	{
		is_Ctrl_Z = false;
	}
	pid_t pid;
	while((pid=waitpid(0,NULL,WNOHANG))>0)
	//while(1)
	{
		//if((pid=waitpid(0,NULL,WNOHANG))>0)
		{
		printf("%d\n",pid);
		for(i=1;i<MAX_BACK_NUM;i++)
		{
			if(back_jobs[i]==NULL)
			{
				continue;
			}
			printf("%d\n",back_jobs[i]->pid);
			printf("%d\n",pid);
			if(back_jobs[i]->pid==pid)
			{
				back_jobs[i]->status = 0;
				break;
			}
		}
		}
	}
	//printf("out:%d\n",pid);
}


void handle_sigstcp(int sig)
{
	int i = 1;
	is_Ctrl_Z = true;
	int flag = 0;
	printf("\n");

	for(i=1;i<MAX_BACK_NUM;i++)
	{
		if(back_jobs[i]==NULL)
		{
			continue;
		}
		if(back_jobs[i]->pid==current_pid)
		{
			back_jobs[i]->status = 2;
			flag=1;
			break;
		}
	}

	if(flag==0)
	{
		back_jobs[back_jobs_index] = (Back_Jobs*)malloc(sizeof(Back_Jobs*));
		back_jobs[back_jobs_index]->pid = current_pid;
		back_jobs[back_jobs_index]->cmd = (char*)malloc(100);
		strcpy(back_jobs[back_jobs_index]->cmd,"This is a stop process!");
		back_jobs[back_jobs_index]->status = 2;
		printf("[%d]%d%d\t\t\t\t%s\n",back_jobs_index,back_jobs[back_jobs_index]->pid,back_jobs[back_jobs_index]->status,back_jobs[back_jobs_index]->cmd);
		back_jobs_index++;
	}
	return;

}

char* trim(char*input)
{
	int i=strlen(input)-1;
	int j=0;
	char*trim_input;
	int k;
	for(;i>=0;i--)
	{
		if(input[i]!=' ')
			break;
	}
	for(;j<=i;j++)
	{
		if(input[j]!=' ')
			break;
	}
	if((trim_input=(char*)malloc((i-j+2)*sizeof(char)))==0)
	{

		fprintf(stderr,"error! can't malloc enough space for arg\n");
		return 0;
	}
	for(k=0;k<i-j+1;k++)
		trim_input[k]=input[j+k];
	trim_input[k]='\0';
	return trim_input;
}

bool change_symbol(char*input)
{
	int i,j,t;
	int input_len = strlen(input);
	bool isFlag = false;	
    int para_count;
	char**arg_order;
	char *buffer;
	bool isChange = false;
	j=0;
	for(i=0;i<input_len;i++)
	{
		if(input[i]==' '||input[i]=='<'||input[i]=='>')
		{
			isFlag = false;
			continue;
		}
		else
		{
			if(!isFlag)
			{
				isFlag = true;
				j++;

			}
		}
	}
	//记录有多少个参数
	para_count = j;
	//printf("%d\n",para_count);
	if((buffer = (char*)malloc((input_len+1)*sizeof(char)))==0)
	{

		fprintf(stderr,"error! can't malloc enough space for arg\n");
		return 0;
	}
/*	if(para_count>2)
	{
		printf("para is too many!\n");
		return;
	}
	else if(para_count<2)
	{
		printf("para is too few!\n");
		return;
	}
*/
	if((arg_order = (char**)malloc((para_count+1)*sizeof(char*)))==0)
	{

		fprintf(stderr,"error! can't malloc enough space for arg\n");
		return 0;
	}
	for(i=0;i<para_count;i++)
	{
                //为了避免空间不够，直接都是分配了最大的需要的内存空间
		if((arg_order[i] = (char*)malloc((input_len+1)*sizeof(char)))==0)
		{

			fprintf(stderr,"error! can't malloc enough space for arg\n");
			return 0;
		}
	}
    for(i=0;i<input_len;i++)
	{
		if(input[i]!=' ')
			break;
	}
	j=0;
	t=0;
	//printf("isrun_0\n");
	for(;i<=input_len;i++)
	{
		if(input[i]==' '||input[i]=='>'||input[i]=='<'||input[i]=='\0')
		{
			if(j==0)
				continue;
			else
			{
				buffer[j] = '\0';
				j++;
				strcpy(arg_order[t],buffer);
				//printf("%s\n",arg_order[t]);
				j=0;
				t++;
			}
		}
		else
		{
			buffer[j] = input[i];
			j++;
		}
	//	if(j>para_count)
	//		break;
	}
	arg_order[t]=NULL;
	//printf("isrun_1:%s\n",arg_order[0]);	
	if(strcmp(arg_order[0],"change")==0)
	{
		isChange = true;
		if(para_count>2)
		{
			printf("para is too much!\n");

			for(i=0;i<=para_count;i++)
			{
				free(arg_order[i]);
			}
			free(arg_order);
			free(buffer);
			return isChange;
		}
		if(para_count<2)
		{
			printf("para is too few!\n");
			for(i=0;i<=para_count;i++)
			{
				free(arg_order[i]);
			}
			free(arg_order);
			free(buffer);
			return isChange;
		}
		//最多取9个，不应太长
		for(i=0;i<min(strlen(arg_order[1]),9);i++)
		{
			Symbol[i] = arg_order[1][i];
		}
		Symbol[i] = '\0';
     	
	}
//	printf("isrun\n");
	for(i=0;i<=para_count;i++)
	{
		free(arg_order[i]);
	}
	free(arg_order);
	free(buffer);
	return isChange;
}
//int count_input_num(const char *input);
