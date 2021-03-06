/*********************************
Trace Replay tools for research----Raidmeter
First write by raysmile, Second modify by maobo
----------------------------2007.11.9
*****************************/
#include <sstream>
#include <iostream>
#include <aio.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "sha256.h"

using namespace std;
//#define DEBUG

#ifdef DEBUG
#define debug2(v1,v2) {cout<<v1<<"    "<<v2<<endl;}
#define debugr3(v1,v2,v3) {cout<<v1<<"    "<<v2<<"    "<<v3<<endl;}
#define debug4(v1,v2,v3,v4) {cout<<v1<<"    "<<v2<<"    "<<v3<<"    "<<v4<<endl;}
#else
#define debug2(v1,v2) ;
#define debug3(v1,v2,v3) ;
#define debug4(v1,v2,v3,v4) ;
#endif

const	int	debug		=	1;
const	long	BLOCK_SIZE	=	512;
const	long 	MAX_BLOCK	=	256;
const	long	MAX_TRACE_COUNT	= 	500000;
const	long	WRITE		=	0;
const	long	READ		=	1;

//trace struct
typedef	struct	_io_trace
{
	double			time;
	unsigned long    	blkno;
	int				blkcount;
	unsigned int	flag;
}io_trace;

typedef	struct	_io_time
{
	double	start_time;
	double	end_time;
	double	elpsd_time;
	unsigned int	flag;
}io_time;

//signal
typedef struct _sig_data
{
	int 		number;
	struct	aiocb64	*aio_req;
}sig_data;

//functions
double get_time(void);
void do_io();
//void callback(sigval_t sigval);
void aio_complete_note( int signo, siginfo_t *info, void *context );
int trace_reader(char *file_name, io_trace *trace, unsigned long max_trace_num, float timescale, float rangescale);
void usage(void);
void deal_by_num();
void deal_by_time();
unsigned long trace_stat(char *file_name, unsigned long  *max_dev_addr);

//define
struct aiocb64 my_aiocb[MAX_TRACE_COUNT];
sig_data	my_data[MAX_TRACE_COUNT];
io_trace	trace[MAX_TRACE_COUNT];
io_time		my_time[MAX_TRACE_COUNT];
static	int 		total=0;
static	double	start=0;
char 	*exit_code;

static	unsigned long	long	dev_size=0;
//defines
char 	*buf_max;
char 	trace_file_name[255];
char 	result_file_name[255];
char 	myname[255];
char	dev_name[255];
int 	deal_time,deal_num;
int		trace_num;
float		timescale;
float		rangescale;
//unsigned long rangescale;
double trace_end_time =0 ;

unsigned long   max_trace_addr;

int main(int argc,char **argv)
{
	int i=0;
	FILE *fp;

	if(argc==1)
	{
		printf("error: no parameter!\n");
		usage();
		exit(0);
	}
	argc--;
	for(i=1; i<argc; i++)
	{
		if(argv[i][0]=='-')
		{
			if(strlen(argv[i])==2)
			{
				switch(argv[i][1])
				{
					case 't':
						i++;
						if(i<argc)
						{
							strcpy(trace_file_name,argv[i]);
						}
						else
						{
							printf("bad trace filename:\"%s\".\n",argv[i-1]);
							usage();
							exit(0);
						}
						break;
					case 'r':
						i++;
						if(i<argc)
						{
							strcpy(result_file_name,argv[i]);
						}
						else
						{
							printf("bad result filename:\"%s\".\n",argv[i-1]);
							usage();
							exit(0);
						}
						break;
					case 'm':
						i++;
						if(i>=argc)
						{
							printf("bad deal time:\"%s\".\n",argv[i-1]);
							usage();
							exit(0);
						}
						deal_time=atoi(argv[i]);
						if (deal_time<0)
						{
							printf("wrong time!\n");
							exit(0);
						}
						break;
					case 'n':
						i++;
						if(i>=argc)
						{
							printf("bad deal number:\"%s\".\n",argv[i-1]);
							usage();
							exit(0);
						}
						deal_num=atoi(argv[i]);
						if (deal_time<0)
						{
							printf("wrong number!\n");
							exit(0);
						}
						break;

					case 'c':

						i++;
						if(i>=argc)
						{
							printf("bad RAID capacity:\"%s\".\n",argv[i-1]);
							usage();
							exit(0);
						}
						dev_size=atoi(argv[i]);
						if (dev_size<0)
						{
							printf("wrong capacity!\n");
							exit(0);
						}
						break;

					case 'i':

						i++;
						if(i>=argc)
						{
							printf("bad timescale:\"%s\".\n",argv[i-1]);
							usage();
							exit(0);
						}
						timescale=atof(argv[i]);
						if (timescale<0)
						{
							printf("wrong timescale!\n");
							exit(0);
						}
						break;

					case 'a':

						i++;
						if(i>=argc)
						{
							printf("bad rangescale:\"%s\".\n",argv[i-1]);
							usage();
							exit(0);
						}
						rangescale= 0;      //atof(argv[i]);
						if (rangescale<0)
						{
							printf("wrong rangescale!\n");
							exit(0);
						}
						break;




					default:
						printf("bad parameter:\"%s\".\n",argv[i]);
						usage();
						exit(0);
						break;
				}
			}
			else
			{
				printf("bad parameter:\"%s\".\n",argv[i]);
				usage();
				exit(0);
			}
		}
		else
		{
			printf("bad parameter:\"%s\".\n",argv[i]);
			usage();
			exit(0);
		}
	}
	if(strlen(trace_file_name)==0)
	{
		strcpy(trace_file_name,"./Financial1.spc");
		printf("no trace file name,use default: [./Financial1.spc]\n");
	}
	if((fp=fopen(trace_file_name,"r"))==NULL)
	{
		printf("Can't open \"%s\"!\n",trace_file_name);
		exit(0);
	}
	fclose(fp);
	if(strlen(result_file_name)==0)
	{
		strcpy(result_file_name,"./results");
		printf("no result filename input,use defult: [./results]\n");
	}
	if (!deal_time)
	{
		deal_time=10;
		printf("no deal time input, use defult: [10s]\n");
	}
	if(!deal_num)
	{
		deal_num=1000;
		printf("no deal number input, use default:[1000]\n");
	}

	if(!timescale)
	{
		timescale=1;
		printf("no timescale input, use default:[1]\n");
	}

	if(!rangescale)
	{
		rangescale=0;
		printf("no rangescale input, use default:[1]\n");
	}

	if(!dev_size)
	{
		dev_size=15*1024*1024;
		printf("no raid capacity input, use default:[15GB]\n");
	}

	strcpy(dev_name,argv[argc]);
	if(strlen(dev_name)==0)
	{
		printf("error: no dev!\n");
		usage();
		exit(0);
	}

	exit_code=(char*)mmap(NULL,sizeof(char),PROT_READ|PROT_WRITE,MAP_SHARED|MAP_ANON,-1,(off_t)0);
	if(exit_code==MAP_FAILED)
	{
		printf("Can't mmap shared memory!\n");
		exit(0);
	}
	*exit_code=10;


	trace_stat(trace_file_name, &max_trace_addr);

	//read trace file amd converte the trace adative to the raid capacity and intensity
	//!!! rangescale needs to be re-implemented, it is not suitable now!
 	//rangescale= (float)max_trace_addr /(dev_size * 1024 * 1024 *2) * 1.0;
 	rangescale= (float)dev_size*2/max_trace_addr;
       // cout<<"max"<<max_trace_addr;
       cout<<"rangescale:"<<rangescale;
	total=trace_reader(trace_file_name, trace, MAX_TRACE_COUNT, timescale, rangescale);

	//cout<<total<<"	records"<<endl;
	printf("\nbenchmark is initialized, press any character to begin the evaluation...");
	getchar();

	pid_t pid=fork();
	if(pid==0)
	{
		while(*exit_code==10)
		{
			printf("please wait,press a character to stop...");
			*exit_code=getchar();
		}
		exit(0);
	}
	do_io();
	double total_size=0;
	double total_time=0;
	for(i=0;i<total;i++)
	{
		total_size+=trace[i].blkcount*BLOCK_SIZE;
		total_time+=my_time[i].elpsd_time;
	}
	cout<<"device size: "<<dev_size<<"KB"<<endl;
	cout<<total<<" requests sent"<<"("<<total_size/1024<<"MB)"<<endl;
//	cout<<"speed:  "<<total_size/1024/my_time[total].end_time<<" KB/s"<<endl;
	cout<<"avg response time:"<<total_time/total*1000<<" ms"<<endl;

	deal_by_time();
	deal_by_num();
	if((fp=fopen(result_file_name,"w"))==NULL)
	{
		cout<<"can't open "<<result_file_name<<endl;
		exit(0);
	}
	for(i=0; i<total; i++)
	{
		fwrite(&my_time[i],sizeof(io_time),1,fp);
	}
	fclose(fp);
	cout<<"time is saved in "<<result_file_name<<endl;
	return 0;
}
//get time
double get_time(void)
{
	struct	timeval	mytime;
	gettimeofday(&mytime,NULL);
	return (mytime.tv_sec*1.0+mytime.tv_usec/1000000.0);
}

unsigned long trace_stat(char *file_name, unsigned long  *max_dev_addr)
{
	FILE	*fp	=	NULL;
	char line[101];
	int  read_count=0;
	int  write_count=0;
	int  total_count=0;
	unsigned long   max_address=0;
	float start_timestamp=0.0;
	float end_timestamp=0.0;
	float io_per_sec=0.0;
	float read_prop=0.0;
	int 	max_length=0;
	int					devno;
	unsigned long	          	address;
	int					length;
	char				op_code;
	char				time_str[20];
	float				timestamp;

	long					count_threshold=MAX_TRACE_COUNT;


	if((fp=fopen(file_name, "r"))==NULL)
	{
		cout<<"open trace file error:"<<file_name<<"can't open!"<<endl;
		return 0;
	}

	while(1){
		if (fgets(line, 100, fp) == NULL || count_threshold<=0) {


			break;
		}

		if (sscanf(line, "%d,%ld,%d,%c,%s\n",&devno,&address,&length,&op_code,time_str) != 5) {
			printf("devno=%d time=%s op=%c address=%ld length=%d\n", devno,time_str,op_code,address,length);

			fprintf(stderr, "Wrong number of arguments for I/O trace event type\n");
			fprintf(stderr, "line: %s", line);
			exit(0);
		}
		if(length > max_length)
			max_length = length;

#if 0
		printf("devno=%d time=%s op=%c address=%ld length=%d\n", devno,time_str,op_code,address,length);
#endif
		//added by smile
		//if(devno!=0)
		//	continue;

		timestamp=atof(time_str);

		total_count++;
		count_threshold--;

		if(total_count==1)
			start_timestamp=timestamp;

		end_timestamp=timestamp;

		if(op_code=='R'||op_code=='r')
			read_count++;
		else
			write_count++;
		if(length/BLOCK_SIZE>MAX_BLOCK)
			length=BLOCK_SIZE*MAX_BLOCK;

		if (address + length/ BLOCK_SIZE > max_address)
		{
			max_address = address + length / BLOCK_SIZE;
		}

	}

	if ((end_timestamp-start_timestamp)>0)
	{
		io_per_sec=(float)total_count * 1.0 / (end_timestamp-start_timestamp);
	}
	if (total_count>0)
	{
		read_prop=(float)read_count * 1.0 / total_count;
	}

	*max_dev_addr=max_address;
	printf("max_address=%ld, total_count=%d(iops=%f), read_count=%d(%f%%), write_count=%d(%f%%)\n\n",
		max_address, total_count, io_per_sec, read_count, read_prop * 100, write_count, (1-read_prop)*100);
	buf_max = (char *)malloc(max_length*sizeof(char));
	memset(buf_max , 1 , max_length);

	fclose(fp);
	return total_count;
}


//read and convert the trace file into memory
int trace_reader(char *file_name, io_trace *trace, unsigned long max_trace_num, float timescale, float rangescale)
{
	FILE	*fp	=	NULL;
	char line[101];
	int i=0;

	int					devno;
	unsigned long		   address;
	int					length;
	char				op_code;
	char				time_str[20];
	float				timestamp;


	if((fp=fopen(file_name, "r"))==NULL)
	{
		cout<<"open trace file error:"<<file_name<<"can't open!"<<endl;
		return 0;
	}

	while(1){
		if (fgets(line, 100, fp) == NULL || max_trace_num<=0) {
			break;
		}

		if (sscanf(line, "%d,%ld,%d,%c,%s\n",&devno,&address,&length,&op_code,time_str) != 5) {
			printf("devno=%d time=%s op=%c address=%ld length=%d\n", devno,time_str,op_code,address,length);
			fprintf(stderr, "Wrong number of arguments for I/O trace event type\n");
			fprintf(stderr, "line: %s", line);
			exit(0);
		}

#if 0
		printf("devno=%d time=%s op=%c address=%d length=%d\n", devno,time_str,op_code,address,length);
#endif
		//added by maobo 2007.11.2
		//if(devno!=0)
                  //      continue;



		max_trace_num--;

		timestamp=atof(time_str);
		trace[i].time=timestamp;
		trace[i].time=trace[i].time / timescale;

		if(op_code=='R'||op_code=='r')
			trace[i].flag=0;
		else
			trace[i].flag=1;

		trace[i].blkno=address;
		trace[i].blkno =(unsigned long)(trace[i].blkno * rangescale);
		trace[i].blkno =(unsigned long)(trace[i].blkno*8)/8; //add by maobo
//		trace[i].blkno =(unsigned long)(devno * rangescale + trace[i].blkno);
	      //	cout<<"blkno:"<<trace[i].blkno<<",";
                trace[i].blkcount=length /BLOCK_SIZE;
                if(trace[i].blkcount>MAX_BLOCK)
		trace[i].blkcount=MAX_BLOCK;
		i++;
	}

	return i;
}
//callback
/*
void callback(sigval_t sigval)
{
	sig_data	*ptr;
	ptr	=	(sig_data *)sigval.sival_ptr;
	if(aio_error64(ptr->aio_req)==0)
	{
		aio_return64(ptr->aio_req);
              if(ptr->number<1)
                {cout<<"L:"<<my_aiocb[ptr->number].aio_nbytes;
                 cout<<",offset:"<<my_aiocb[ptr->number].aio_offset<<endl;}
		my_time[ptr->number].end_time=get_time()-start;
		my_time[ptr->number].elpsd_time=my_time[ptr->number].end_time-my_time[ptr->number].start_time;
		debug4("req:",ptr->number,"time use:",my_time[ptr->number].elpsd_time);
		debug4("start time:",my_time[ptr->number].start_time,"end time:",my_time[ptr->number].end_time);
	}
	else
	{
		cout<<"io error,req number:"<<ptr->number<<" size:"<<ptr->aio_req->aio_nbytes<<" offset:"<<ptr->aio_req->aio_offset<<endl;
	}
//	free((void*)(ptr->aio_req)->aio_buf);
	return;
}
*/
//notify
void aio_complete_note( int signo, siginfo_t *info, void *context )
{
	sig_data *req;
	int ret;

	/* Ensure it's our signal */
	if (info->si_signo == SIGIO) {
		req = (sig_data *)info->si_value.sival_ptr;
		/* Did the request complete? */
	if (aio_error64(req->aio_req) == 0) {
		/* Request completed successfully, get the return status */
	      	ret = aio_return64(req->aio_req);
		my_time[req->number].end_time=get_time()-start;
		my_time[req->number].elpsd_time=my_time[req->number].end_time-my_time[req->number].start_time;
//		printf("--we get here--used time =%lf---\n",my_time[req->number].elpsd_time);
    		}
	}
	return;
}

//do io
void do_io()
{
	int i=0;
	struct sigaction sig_act; //add by maobo
	int fd=open(dev_name,O_RDWR|O_LARGEFILE);
	if(fd==-1)
	{
		cout<<"open "<<dev_name<<" error!"<<endl;
		exit(0);
	}
	struct stat st;
	fstat(fd,&st);
	//dev_size = lseek(fd, 0, SEEK_END)/BLOCK_SIZE;
	//printf("\ndev_size=%ld\n",dev_size);
	struct aiocb64 myaio;
	unsigned long max=0;
	for(i=0;i<total;i++)
		if(trace[i].blkcount>max)
			max=trace[i].blkcount;
	myaio.aio_buf=malloc(max*BLOCK_SIZE+1);
	for(i=0;i<total;i++)
	{
			bzero((char *)&my_aiocb[i],sizeof(struct aiocb));

			my_aiocb[i].aio_fildes = fd;
			my_aiocb[i].aio_buf = myaio.aio_buf;
			my_aiocb[i].aio_nbytes = trace[i].blkcount*BLOCK_SIZE;
			my_aiocb[i].aio_offset = trace[i].blkno*BLOCK_SIZE;

			sigemptyset(&sig_act.sa_mask);//add by maobo
  			sig_act.sa_flags = SA_SIGINFO;
 			sig_act.sa_sigaction = aio_complete_note;
			my_aiocb[i].aio_sigevent.sigev_notify = SIGEV_SIGNAL;
			my_aiocb[i].aio_sigevent.sigev_signo = SIGIO;


			//link callback
			if(i<1)
                        {cout<<"L:"<<my_aiocb[i].aio_nbytes<<",";
                         cout<<"blkno:"<<trace[i].blkno<<",";
                         cout<<"offset:"<<my_aiocb[i].aio_offset<<endl;
                        }
//                  my_aiocb[i].aio_sigevent.sigev_notify =	SIGEV_THREAD;
//			my_aiocb[i].aio_sigevent.sigev_notify_function = callback;
//			my_aiocb[i].aio_sigevent.sigev_notify_attributes = NULL;
			my_data[i].number=i;
			my_data[i].aio_req=&my_aiocb[i];
			my_aiocb[i].aio_sigevent.sigev_value.sival_ptr = &my_data[i];

			sigaction( SIGIO, &sig_act, NULL );//add by maobo

	}
	double temp_time;
	uint64_t a1=1, a2=1, a3;
	i=0;
	start=get_time();
	while(i < total&& *exit_code==10)
	{
		if((temp_time=(get_time()-start))>=(trace[i].time))
			//	if((temp_time=(get_time()-start))>i*0.05)
		{
			int blk_num = my_aiocb[i].aio_nbytes/4096;
			int blk_left = my_aiocb[i].aio_nbytes%4096;
			int xor_time;
			my_time[i].start_time=temp_time;
			while(1)
                        {
                                blk_num--;
				for(xor_time = 0; xor_time < 64; xor_time++)
				{
					a3 = a1 ^ a2;
				}
                                if(blk_num < 0)
                                        break;
                                else
                                {
                                        if(blk_num > 0)
                                                continue;
                                        if(blk_num == 0 && blk_left > 0)
                                                continue;
                                        else
                                                break;
                                }
                        }
			if(0)
			{
				aio_write64(&my_aiocb[i]);
				my_time[i].flag=1;
			}
			else
			{
				aio_read64(&my_aiocb[i]);
				my_time[i].flag=0;
			}
			i++;
			debug4("time:",temp_time,"send req:",i-1);
		}
	}
	total=i-1;
	trace_end_time= get_time();
	sleep(1);
	close(fd);
}

/*function deal with the test result*/
 void deal_by_time(void)
 {
 	FILE *fp;
// 	int n=(int)my_time[total-1].end_time/deal_time+1;
	int n =(int) (trace_end_time - start)/deal_time +1;
	int *n_of_ti=(int *)(malloc(sizeof(int)*n));
	double *time_use=(double *)(malloc(sizeof(double)*n));
	long *block_count=(long *)(malloc(sizeof(long)*n));
	double endt=0;
	double time_temp=0;
	long   count=0;
	char file_name[255];
	int i=0,j=0;
	printf("maobo---deal by time --n=%d--\n",n);
	for(i=0; i<n; i++)
	{
		n_of_ti[i]=0;
		time_use[i]=0;
		block_count[i]=0;
	}
	strcpy(file_name,result_file_name);
	strcat(file_name,".txt");
	if((fp=fopen(file_name,"w"))==NULL)
	{
		printf("open result file error!\n");
		exit(0);
	}
//	printf("\n*************************total result(by time)*********************************\n");
//	printf("   time(s)        ios      average time(ms)    ios per sec(io/s)    spend(kb/s)\n");
	fprintf(fp,"\n****************************result(by time)***********************************\n");
	fprintf(fp,"   time(s)        ios    average  time(ms)    ios per sec(io/s)    spend(kb/s)\n");
	for(i=0; i<n; i++)
	{
		endt=deal_time*(i+1);
		while(my_time[j].end_time<endt&&j<total)
		{
			n_of_ti[i]++;
			time_use[i]+=my_time[j].elpsd_time;
			block_count[i]+=trace[j].blkcount;
			j++;
		}
		time_temp=my_time[count+n_of_ti[i]-1].end_time-my_time[count].start_time;
		count+=n_of_ti[i];
/*		printf("%4d--%4d      %5d            %4.6lf           %10d     %10d\n",
			(int)(endt-deal_time),
			(int)(endt),  n_of_ti[i],
			n_of_ti[i]==0?0:(time_use[i]*1000/n_of_ti[i]),
			(n_of_ti[i]==0?0:(int)(1/(time_temp/n_of_ti[i]))),
			n_of_ti[i]==0?0:(int)(block_count[i]*BLOCK_SIZE/1024.0/time_temp));*/
		fprintf(fp,"%4d--%4d      %5d           %4.6lf           %10d     %10d\n",
			(int)(endt-deal_time),
			(int)(endt),  n_of_ti[i],
			n_of_ti[i]==0?0:(time_use[i]*1000/n_of_ti[i]),
			(n_of_ti[i]==0?0:(int)(1/(time_temp/n_of_ti[i]))),
			n_of_ti[i]==0?0:(int)(block_count[i]*BLOCK_SIZE/1024.0/time_temp));
	}
	fclose(fp);
 }

 void deal_by_num()
 {
	 FILE *fp;
	 int n=total/deal_num+1;
	 int *num=(int *)(malloc(sizeof(int)*n));
	 double *time_use=(double *)(malloc(sizeof(double)*n));
	 long *block_count=(long *)(malloc(sizeof(long)*n));
	 double time_total = 0.0;
	 char file_name[255];
	 int i=0;
	 for(i=0; i<n; i++)
	 {
	 	time_use[i]=0;
		num[i]=0;
		block_count[i]=0;
	 }
	 for (i=0; i<total; i++)
	 {
		 time_use[i/deal_num]+=my_time[i].elpsd_time;
		 time_total+=my_time[i].elpsd_time;
		 num[i/deal_num]++;
		 block_count[i/deal_num]+=trace[i].blkcount;
	 }
	 strcpy(file_name,result_file_name);
	 strcat(file_name,".num");
	 cout<<"result is saved in "<<file_name<<endl;
	 if((fp=fopen(file_name,((deal_time)?"a":"w")))==NULL)
  	 {
		 printf("open result file error!\n");
		 exit(0);
 	 }
//	 printf("\n*******************************result(by number)*******************************\n");
//	 printf("     num          ios      average time(ms)    ios per sec(io/s)    spend(kb/s)\n");
	 fprintf(fp,"\n*****************************result(by number)********************************\n");
	 fprintf(fp,"     num          ios     average time(ms)    ios per sec(io/s)    spend(kb/s)\n");
	 if(total%deal_num==0)
	 {
	 	n--;
	 }
	 for (i=0; i<n; i++)
	 {
	 	 double time_temp=my_time[i*deal_num+num[i]-1].end_time-my_time[i*deal_num].start_time;
/*		 printf("%5d--%5d     %4d            %4.6lf           %10d     %10d\n",
			 i*deal_num,  (i+1)*deal_num-1,  num[i],
			 time_use[i]/num[i]*1000,
			 (int)(1.0/(time_temp/num[i])),
			 (int)(block_count[i]*BLOCK_SIZE/1024.0/time_temp));*/
		 fprintf(fp,"%5d--%5d     %4d           %4.6lf           %10d     %10d\n",
			 i*deal_num,  (i+1)*deal_num-1,  num[i],
			 time_use[i]/num[i]*1000,
			 (int)(1.0/(time_temp/num[i])),
			 (int)(block_count[i]*BLOCK_SIZE/1024.0/time_temp));

	 }
	fprintf(fp,"average time(ms):%4.6lf\n",time_total/total*1000);
	 fclose(fp);

 }
/*show help*/
 void usage(void)
 {
 	printf("usage:%s [-t tracefilename] [-r resultfilename] [-p processnumber] [-m time] [-nnumber] dev\n",myname);
	printf("      [-t tracefilename]:\n");
	printf("      [-r resultfilename]:\n");
	printf("      [-c raid capacity(KB)]:\n");
	printf("      [-i timescale(default:1, faster 2+)]:\n");
	printf("      [-a rangescale(default:0, bigger 2+)]:\n");
	printf("      [-m deal time]:\n");
	printf("      [-n deal number]:\n");
	printf("      dev:\n");
}
