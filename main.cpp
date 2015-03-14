#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <iostream>

#define BAUDRATE B9600
using namespace std;


typedef struct 
{
  float latitude;
  float longitude;
}GPS;

GPS* splitgpgga(GPS*,unsigned char);

int t = 0;
int flag = 0;
float ido;
float keido;
char gpggacheck[6];
char gpgga[] = "GPGGA";
char latitudedo[3];
char latitudehun[8];
char longitudedo[4];
char longitudehun[8];

GPS* splitgpgga(GPS* here,unsigned char c)
{
  switch(flag)
    {
    case 0:
      {
	if( c == '$' ) flag=1;
	break;
      }
    case 1:
      {
	if( c == ','){
	  if(strncmp(gpggacheck,gpgga,5) == 0) flag = 2;
	  else flag = 0;
	  t = 0;		
	}else{
	  gpggacheck[t] = c;
	  t++;
	  if(t>5) t=0;	
	}
	break;
      }
    case 2:
      {
	if( c == ',')  flag = 3;
      }
      break;
    case 3:
      {
	latitudedo[t] = c;
	t++;
	if(t == 2){
	  latitudedo[t] = 0;
	  flag = 31;
	  t = 0;
	}
	break;
      }
    case 31:
      {
	if( c == ','){
	  latitudehun[t] = 0;
	  flag = 4;
	  t = 0;
	}else{
	  latitudehun[t] = c;
	  t++;
	  if(t > 9) t = 0;
	}
	break;
      }
    case 4:
      {
	if( c == ',')  flag = 5;
	break;
      }
    case 5:
      {
	longitudedo[t] = c;
	t++;
	if(t == 3){
	  longitudedo[t] = 0;
	  flag = 51;
	  t = 0;
	}
	break;
      }
      break;
    case 51:
      {
	if(c == ','){
	  longitudehun[t] = 0;
	  flag = 6;
	  t = 0;
	}else{
	  longitudehun[t] = c;
	  t++;
	  if(t > 9) t = 0;
	}
	break;
      }
    case 6:
      {
	ido = atof(latitudedo) + (atof(latitudehun)/60);
	keido = atof(longitudedo) + (atof(longitudehun)/60);

	here->latitude = ido;
	here->longitude = keido;
	
	flag = 0;
	break;
      }
      break;
    default: break;
    }
  return here;
}
 
int main(int argc,char** argv)
{
  struct termios tio;
  struct termios stdio;
  struct termios old_stdio;
  GPS Here;

  int tty_fd;
 
  unsigned char c='D';
  tcgetattr(STDOUT_FILENO,&old_stdio);
 
  cout << "Please start with  " << endl;
  cout <<  argv[0] << "  /dev/ttyACM0 (for example)\n" << endl;

  memset(&stdio,0,sizeof(stdio));
  stdio.c_iflag=0;
  stdio.c_oflag=0;
  stdio.c_cflag=0;
  stdio.c_lflag=0;
  stdio.c_cc[VMIN]=1;
  stdio.c_cc[VTIME]=0;
  tcsetattr(STDOUT_FILENO,TCSANOW,&stdio);
  tcsetattr(STDOUT_FILENO,TCSAFLUSH,&stdio);
  fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
 
  memset(&tio,0,sizeof(tio));
  tio.c_iflag=0;
  tio.c_oflag=0;
  tio.c_cflag=CS8|CREAD|CLOCAL;
  tio.c_lflag=0;
  tio.c_cc[VMIN]=1;
  tio.c_cc[VTIME]=5;
 
  tty_fd=open(argv[1], O_RDWR | O_NONBLOCK);      
  cfsetospeed(&tio,BAUDRATE);
  cfsetispeed(&tio,BAUDRATE);
 
  tcsetattr(tty_fd,TCSANOW,&tio);
  while (c!='q')
    {
      if (read(tty_fd,&c,1)>0) 	 //     write(STDOUT_FILENO,&c,1);
	splitgpgga(&Here,c);
      if (read(STDIN_FILENO,&c,1)>0)  write(tty_fd,&c,1); 

      cout.precision(4);
      cout << "N" << fixed << Here.latitude ;
      cout << "E" << fixed <<Here.longitude << "\r" ;
    }
 
  close(tty_fd);
  tcsetattr(STDOUT_FILENO,TCSANOW,&old_stdio);
  return EXIT_SUCCESS;
}
