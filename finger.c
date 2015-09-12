//串口相关的头文件
#include<stdio.h>      /*标准输入输出定义*/
#include<stdlib.h>     /*标准函数库定义*/
#include<unistd.h>     /*Unix 标准函数定义*/
#include<sys/types.h> 
#include<sys/stat.h>   
#include<fcntl.h>      /*文件控制定义*/
#include<termios.h>    /*PPSIX 终端控制定义*/
#include<errno.h>      /*错误号定义*/
#include<string.h>


#define BUFFSIZE 100
#define FALSE  -1
#define TRUE   0

int GIMG[12] = {0xef, 0X01 ,0Xff,0xff,0xff,0xff, 0x01, 0,3,1,0x00,0x05};

int GENT1[13] = {0xef,0X01 ,0Xff,0xff,0xff,0xff,0x01,0x00,0x04,0x02,0x01,0x00,0x08};
int GENT2[13] = {0xef,0X01 ,0Xff,0xff,0xff,0xff,0x01,0x00,0x04,0x02,0x02,0x00,0x09};
int MERG[14]={0xef,0X01 ,0Xff,0xff,0xff,0xff, 0x01,  0x00,0x03,0x05, 0x00,0x09};
int STOR[16] = {0xef,0X01,0Xff,0xff,0xff,0xff, 0x01,0x00,0x06,0x06,0x02,0x00,0x00,0x00,0x0f};
int UART0_Set(int fd,int speed,int flow_ctrl,int databits,int stopbits,int parity)
{
   
      int   i;
         int   status;
         int   speed_arr[] = { B115200, B19200, B9600, B4800, B2400, B1200, B300};
     int   name_arr[] = {115200,  19200,  9600,  4800,  2400,  1200,  300};
         
    struct termios options;
   
    /*tcgetattr(fd,&options)得到与fd指向对象的相关参数，并将它们保存于options,该函数还可以测试配置是否正确，该串口是否可用等。若调用成功，函数返回值为0，若调用失败，函数返回值为1.
    */
    if  ( tcgetattr(fd,&options)  !=  0)
       {
          perror("SetupSerial 1");    
          return(FALSE); 
       }
  
    //设置串口输入波特率和输出波特率
    for ( i= 0;  i < sizeof(speed_arr) / sizeof(int);  i++)
                {
                     if  (speed == name_arr[i])
                            {             
                                 cfsetispeed(&options, speed_arr[i]); 
                                 cfsetospeed(&options, speed_arr[i]);  
                            }
              }     
   
    //修改控制模式，保证程序不会占用串口
    options.c_cflag |= CLOCAL;
    //修改控制模式，使得能够从串口中读取输入数据
    options.c_cflag |= CREAD;
  
    //设置数据流控制
    switch(flow_ctrl)
    {
      
       case 0 ://不使用流控制
              options.c_cflag &= ~CRTSCTS;
              break;   
      
       case 1 ://使用硬件流控制
              options.c_cflag |= CRTSCTS;
              break;
       case 2 ://使用软件流控制
              options.c_cflag |= IXON | IXOFF | IXANY;
              break;
    }
    //设置数据位
    //屏蔽其他标志位
    options.c_cflag &= ~CSIZE;
    switch (databits)
    {  
       case 5    :
                     options.c_cflag |= CS5;
                     break;
       case 6    :
                     options.c_cflag |= CS6;
                     break;
       case 7    :    
                 options.c_cflag |= CS7;
                 break;
       case 8:    
                 options.c_cflag |= CS8;
                 break;  
       default:   
                 fprintf(stderr,"Unsupported data size\n");
                 return (FALSE); 
    }
    //设置校验位
    switch (parity)
    {  
       case 'n':
       case 'N': //无奇偶校验位。
                 options.c_cflag &= ~PARENB; 
                 options.c_iflag &= ~INPCK;    
                 break; 
       case 'o':  
       case 'O'://设置为奇校验    
                 options.c_cflag |= (PARODD | PARENB); 
                 options.c_iflag |= INPCK;             
                 break; 
       case 'e': 
       case 'E'://设置为偶校验  
                 options.c_cflag |= PARENB;       
                 options.c_cflag &= ~PARODD;       
                 options.c_iflag |= INPCK;      
                 break;
       case 's':
       case 'S': //设置为空格 
                 options.c_cflag &= ~PARENB;
                 options.c_cflag &= ~CSTOPB;
                 break; 
        default:  
                 fprintf(stderr,"Unsupported parity\n");    
                 return (FALSE); 
    } 
    // 设置停止位 
    switch (stopbits)
    {  
       case 1:   
                 options.c_cflag &= ~CSTOPB; break; 
       case 2:   
                 options.c_cflag |= CSTOPB; break;
       default:   
                       fprintf(stderr,"Unsupported stop bits\n"); 
                       return (FALSE);
    }
   
  //修改输出模式，原始数据输出
  options.c_oflag &= ~OPOST;
  
  options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);//我加的
//options.c_lflag &= ~(ISIG | ICANON);
   
    //设置等待时间和最小接收字符
    options.c_cc[VTIME] = 1; /* 读取一个字符等待1*(1/10)s */  
    options.c_cc[VMIN] = 1; /* 读取字符的最少个数为1 */
   
    //如果发生数据溢出，接收数据，但是不再读取 刷新收到的数据但是不读
    tcflush(fd,TCIFLUSH);
   
    //激活配置 (将修改后的termios数据设置到串口中）
    if (tcsetattr(fd,TCSANOW,&options) != 0)  
           {
               perror("com set error!\n");  
              return (FALSE); 
           }
    return (TRUE); 
}
int UART0_Init(int fd, int speed,int flow_ctrl,int databits,int stopbits,int parity)
{
    int err;
    //设置串口数据帧格式
    if (UART0_Set(fd,115200,0,8,1,'N') == FALSE)
       {                                                         
        return FALSE;
       }
    else
       {
               return  TRUE;
        }
}
int main(int argc,char *argv[])
{
	int fd,nwrite,nread,i =0;int flag= 0;
	int times;int fingernum;
	char *ptr = NULL;
	int msg[16] = {0xef,0x01,0xff,0xff,0xff,0xff,0x01,0x00,0x07,0x13,0x00,0x00,0x00,0x00,0x00,0x1b};
	//char Cmp_msg[12] = {"ef","1","ff","ff","ff","ff","1","0","7","
	UART0_Init(fd,115200,0,8,1,'N');
	fd = open("/dev/ttyS1",O_RDWR);
	if(-1 == fd){
		perror("open error\n");
		return -1;
		}

	for(i = 0;i<16;i++){
      		nwrite = write(fd,&msg[i],1);
      		if(nwrite < 0){
          		perror("error to write\n");
            		return -1;
                    	}nwrite = 0;//10.19-------------------
		}
	ptr=(char *)malloc(sizeof(char) *1);
		if(ptr==NULL){
		exit(1);
		}

 	nread = read(fd,ptr,12);
		if(nread<0){
		printf("error read\n");
		return -1;
		} nread =0;//10.19---------------------------		
	//printf("finger data\n");
	if(ptr !=NULL){
	//printf("%x\n",*(ptr));
		if(*(ptr+10)==0)
			{	
				printf("hand successful\n");
			}
		     }
	free(ptr);//10.19-------------------------------------
	// ************************begin*********************
	switch (*argv[1]){
	case '1':
	{
		//get image of picture
		int GIMG[12] = {0xef, 0X01 ,0Xff,0xff,0xff,0xff, 0x01, 0x00,0x03,0x01,0x00,0x05};
		//for(times = 0;times <40;times++) {
		   for(i =0;i<12;i++){
			nwrite = write(fd,&GIMG[i],1);
				   }
		ptr = (char *)malloc(sizeof(char)*1);
			if(ptr==NULL){
				exit(1); }
		
		nread =read(fd,ptr,12);
			if(*(ptr+9)==0){
				printf("in first******* get finger success \n");//----------------get image successful'''''1st
					
			//-----------生成特征文件---------------
			int GENT1[13] = {0xef,0X01 ,0Xff,0xff,0xff,0xff,0x01,0x00,0x04,0x02,0x01,0x00,0x08};
			for(i = 0;i<13;i++){
				nwrite = write(fd,&GENT1[i],1);
				}
			free(ptr);
			ptr = (char *)malloc(sizeof(char)*1);
			if(ptr==NULL){
                                exit(1); }
			nread = read(fd,ptr,12);
			if(ptr !=NULL){
				if(*(ptr+9)==0){
					printf("-in frist ********************生成特征文件-\n");//---------------1生成特征文件-successful---------
						}
					}
				}
		
                   for(i =0;i<12;i++){
                        nwrite = write(fd,&GIMG[i],1);      }
                ptr = (char *)malloc(sizeof(char)*1);
                        if(ptr==NULL){
                                exit(1); }
                nread =read(fd,ptr,12);
                        if(*(ptr+9)==0){
                                printf("in second *******get finger success \n");//----------------get image successful'''''2st
                                        
                        //----------2-生成特征文件---------------
                        int GENT2[13] = {0xef,0X01 ,0Xff,0xff,0xff,0xff,0x01,0x00,0x04,0x02,0x02,0x00,0x09};
                        for(i = 0;i<13;i++){
                                nwrite = write(fd,&GENT2[i],1);
                                }
                        free(ptr);
                        ptr = (char *)malloc(sizeof(char)*1);
                        if(ptr==NULL){
                                exit(1); }      
                        nread = read(fd,ptr,12);
                        if(ptr !=NULL){
                                if(*(ptr+9)==0){
                                        printf("in second*************-生成特征文件-\n");
					flag = 1;//--------------2-生成特征文件-successful---------
                                 }	}	}free(ptr);
		{
			flag=0;
			printf("get two finger\n");
			int MERG[12]={0xef,0X01 ,0Xff,0xff,0xff,0xff, 0x01,  0x00,0x03,0x05, 0x00,0x09};
			for(i=0;i<12;i++){
				write(fd,&MERG[i],1);
					}
			ptr = (char *)malloc(sizeof(char)*1);
			if(ptr==NULL){exit(1);
				}
		nread=	read(fd,ptr,12);
			if(ptr!=NULL){
				printf("%x",(*(ptr+9)));
				if(*(ptr+9)==0){
					printf("first finger and second finger 合并成功\n");flag=1;free(ptr);
					}
				}
			}
			//get num of finger 
			int GetNum[12] = {0xef,0x01,0xff,0xff,0xff,0xff,0x01,0x00,0x03,0x1d,0x00,0x21};
				for(i=0;i<12;i++){
                                write(fd,&GetNum[i],1);
                                        }
                        ptr = (char *)malloc(sizeof(char)*1);
                        if(ptr==NULL){exit(1);
                                }
                        read(fd,ptr,14);
                        if(ptr!=NULL){
                                if(*(ptr+9)==0){
                                        printf("获取有效指纹膜版数成功\n");
					if(*(ptr+11) !=NULL){
						fingernum = *(ptr+11);
						printf("%d",fingernum);
						
                                        }      } 	} 

			if(1==flag){
				int STOR[15] = {0xef,0X01,0Xff,0xff,0xff,0xff, 0x01,0x00,0x06,0x06,0x02,0x00,0x00,0x00,0x0f};
				for(i=0;i<15;i++){
				if(i==12){write(fd,&fingernum,1);}
				else if(i==14){	
						fingernum = 0x0f +fingernum;
						write(fd,&fingernum,1);
						}
					else{
  	                              write(fd,&STOR[i],1);
                                        }}
                        ptr = (char *)malloc(sizeof(char)*1);
                        if(ptr==NULL){exit(1);
                                }
                        read(fd,ptr,12);
                        if(ptr!=NULL){
                                if(*(ptr+9)==0){
                                        printf("指纹已成功录入\n");flag=1;free(ptr);
                                        }
			        	}	
				
		}
		fingernum=0;		}break;
	//search finger************************************************************************************************************************************************************************************
      case '2': {
	 for(i =0;i<12;i++){
      nwrite = write(fd,&GIMG[i],1);
           }
    ptr = (char *)malloc(sizeof(char)*1);
      if(ptr==NULL){
        exit(1); }
    
    nread =read(fd,ptr,12);
      if(*(ptr+9)==0){
        printf("get finger success \n");//----------------get image successful'''''1st
      for(i = 0;i<13;i++){
        nwrite = write(fd,&GENT1[i],1);
        }
      free(ptr);
      ptr = (char *)malloc(sizeof(char)*1);
      if(ptr==NULL){
                                exit(1); }
      nread = read(fd,ptr,12);
      if(ptr !=NULL){
        if(*(ptr+9)==0){
          printf("-生成特征文件-\n");//---------------1生成特征文件-successful---------
            }
          }
        }
    
                   for(i =0;i<12;i++){
                        nwrite = write(fd,&GIMG[i],1);      }
                ptr = (char *)malloc(sizeof(char)*1);
                        if(ptr==NULL){
                                exit(1); }
                nread =read(fd,ptr,12);
                        if(*(ptr+9)==0){
                                printf("get finger success \n");//----------------get image successful'''''1st
                                        
                        //----------2-生成特征文件---------------
                        for(i = 0;i<13;i++){
                                nwrite = write(fd,&GENT2[i],1);
                                }
                        free(ptr);
                        ptr = (char *)malloc(sizeof(char)*1);
                        if(ptr==NULL){
                                exit(1); }      
                        nread = read(fd,ptr,12);
                        if(ptr !=NULL){
                                if(*(ptr+9)==0){
                                        printf("-生成特征文件-\n");
          flag = 1;} } } //--------------2-生成特征文件-successful---------
	int SEAT[17]={0xef,0X01 ,0Xff,0xff,0xff,0xff, 0x01,0x00,0x08,0x04,0x01,0x00,0x00,0x00,0x65,0x00,0x73};
	if(1==flag){
		 for(i = 0;i<17;i++){
                        nwrite = write(fd,&SEAT[i],1);
                                }
                        free(ptr);
                        ptr = (char *)malloc(sizeof(char)*1);
                        if(ptr==NULL){
                                exit(1); }
                        nread = read(fd,ptr,16);
                        if(ptr !=NULL){
                                if(*(ptr+9)==0){
					printf("指纹比对ing...\n");
                                        printf("在指纹库中检测到您的指纹\n\n");
						}}}
}//arg2------------------------------
}	return 0;

}

