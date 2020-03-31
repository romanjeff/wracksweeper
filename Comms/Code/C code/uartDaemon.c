#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>
#include <serial.h>
#include <termios.h>


/* Declare function prototypes */

#define MODEM "/dev/tty04"
#define BAUDRATE B115200
#define FALSE 0
#define TRUE 1


int main(void){
				// PUT EVERYTHING BELOW HERE INSIDE int main(void){} with an exit(EXIT_SUCCESS) at the end

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
					exit(EXIT_FAILURE)
				}


				/* change current working dir to root */
				/* root will always be there, so we work from it */
				if ((chdir("/") < 0){
					exit(EXIT_FAILURE);
				}


				/* close standard file descriptors */
				close(STDIN_FILENO);
				close(STDOUT_FILENO);
				close(STDERR_FILENO);

				/* Loop to do work */

				while(1) {

				/* do tasks in here*/

				}
				
		/* always leave an exit path of some kind*/
		exit(EXIT_SUCCESS)

}
