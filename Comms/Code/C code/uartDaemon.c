#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
//#include <syslog.h>
#include <string.h>
//#include <termios.h>


/* Declare function prototypes */

#define MODEM "/dev/tty04"
#define BAUDRATE B115200
#define FALSE 0
#define TRUE 1


int main(void){

				/* set pointer to output logfile */
				FILE *logFile = NULL;

				/* declare process id for parent, session id for child as type pid_t*/
				pid_t pid, sid;

				/* fork uartServ */
				pid = fork();

				/* if fork wasn't successful, child process = -1 */
				if (pid < 0) {
					exit(EXIT_FAILURE);
				}

				/* EXIT_SUCCESS exits parent process since fork worked */
				if (pid> 0){
					exit(EXIT_SUCCESS);
				}

				umask(0);

				/* set session id */
				sid = setsid();

				if (sid <0) {
					exit(EXIT_FAILURE);
				}


				/* change current working dir to root */
				/* root will always be there, so we work from it */
				if ((chdir("/")) < 0){
					exit(EXIT_FAILURE);
				}


				/* close standard file descriptors */
				close(STDIN_FILENO);
				close(STDOUT_FILENO);
				close(STDERR_FILENO);

				/* Loop to do work */

				while(1) {

					// open the port, or throw an error if you can't.
					int serial_port = open("/dev/ttyUO4",O_RDWR);
					if(serial_port < 0){
						printf("Error %i from open: %s\n", errno, strerror(errno));
					}
					// set up a struct for the termios tty instance
					struct termios tty;
					memset(&tty, 0, sizeof tty);

					// throw error if struct can't be used with termios::tcgetattr()
					if(tcgetattr(serial_port,&tty) !=0){
						printf("Error %i from tcgetattr: %s\n", errno, strerror(errno));
					}

					// set up the tty struct for termios

					tty.c_cflag &= ~PARENB; 				// clear parity bit, disable parity
																					// check
					tty.c_cflag &= ~CSTOPB; 				// one stop bit
					tty.c_cflag |= CS8; 						// 8 bits per byte
					tty.c_cflag &= ~CRTSCTS;				// disable hardware flow control
				  tty.c_cflag |= CREAD | CLOCAL		// disable modem specific lines, allow
																					// reads
					tty.c_lflag &= ~ICANON;					// Disable canonical input so newline
																					// doesn't cause input to process

				}

		/* always leave an exit path of some kind*/
		exit(EXIT_SUCCESS);

}
