/*                                                                         *
 *  Ground ZERO improvements copyright pending 1994, 1995 by James Hilke   */

#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include "status.h"

main ()
{
  int pid;
  char filename[80];
  const char *argv[] =
    {"/usr/bin/nice", "-20", "ground0", "-p", "4000", NULL};
  const char *envp[] =
    {NULL};
  int found, count = 1000;
  int status, newd;

  newd = -1;
  while (1)
    {
      /* yes, we could just look for the files at the beginning and count
	 up from there, but then we wouldn't be able to adjust for deleted
	 or added logs while running */
      while (newd == -1)
	{
	  sprintf (filename, "../log/%d.log", count);
	  newd = open (filename, O_WRONLY|O_CREAT|O_EXCL, S_IWUSR|S_IRUSR);
	  count++;
	}

      dup2 (newd, 2);
      close (newd);
      newd = -1;
  
      if ((pid = fork()) == NULL)
        execve ("/usr/bin/nice", argv, envp);
      else
        wait (&status);

      if (WIFEXITED (status))
	{
	  unsigned char num;

	  num = WEXITSTATUS(status);

	  if (num != STATUS_REBOOT)
	    exit (num);
	}
      else
	system ("mv -f  ../boot/fullsave.txt ../boot/startfile.txt");
    }
}
