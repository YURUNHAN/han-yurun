/************** lab3base.c file **************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#define MAX 128

char gdir[MAX]; // gdir[ ] stores dir strings
char *dir[64];
int ndir;

char gpath[MAX]; // gpath[ ] stores token strings
char *name[64];
int ntoken;
int fd[2];
char *s, line[MAX];
int i;
int pid, status;
char *head, *tail;
int pd[2];
int lpd[2];



int main(int argc, char *argv[], char *env[])
{
  fd[0] = dup(stdout);
  fd[1] = dup(stdin);



  printf("************* Welcome to yhsh **************\n");
  i = 0;
  while (env[i])
  {
    printf("env[%d] = %s\n", i, env[i]);

    // Looking for PATH=
    if (strncmp(env[i], "PATH=", 5) == 0)
    {
      printf("show PATH: %s\n", env[i]);

      printf("decompose PATH into dir strings in gdir[ ]\n");
      strcpy(gdir, &env[i][5]);

      
      printf("%s", gdir);
      ndir = 0;
      char *p = strtok(gdir, ":");

      while (p != NULL)
      {
        dir[ndir++] = p;
        p = strtok(NULL, ":");
      }

      for (int i = 0; i < ndir; i++)
      {
        printf("%d. %s\n", i, dir[i]);
      }

      break;
    }
    i++;
  }

  printf("*********** yhsh processing loop **********\n");

  while (1)
  {
    printf("\yhsh $: ");
    fgets(line, 128, stdin);
    line[strlen(line) - 1] = 0; // fgets() has \n at end


    if (line[0] == 0)
      continue;
    printf("line = %s\n", line); // print line to see what you got

    
    strcpy(gpath, line);
    tokenizeLine(&line);

    // 3. Handle name[0] == "cd" or "exit" case

    if (strcmp(name[0], "cd") == 0)
    {
      if (name[1] == NULL)
      {
        printf("%s\n", &env[28][5]);
        chdir(&env[28][5]);
      }
      else
      {
        chdir(name[1]);
      }
      continue;
    }

    if (!strcmp(name[0], "exit"))
    {
      exit(1);
    }

    // 4. name[0] is not cd or exit:

    pid = fork(); // fork a child sh

    if (pid)
    {
      printf("parent sh %d waits\n", getpid());
      pid = wait(&status);
      printf("child sh %d died : exit status = %04x\n", pid, status);
      continue;
    }
    else
    {
      // doPipe(line, 0);
      printf("child sh %d begins\n", getpid());
      doPipe(gpath, 0);

    }
  }
}

int doPipe(char *cmdLine, int *pd)
{
  if (pd)
  { // pipe passed in, as writer on pipe
    close(pd[0]);
    dup2(pd[1], 1);
    close(pd[1]);
  }
  int hasPipe = scan(cmdLine);
  if(hasPipe){
    pipe(lpd);
    int pid = fork();
    if(pid){
      close(lpd[1]);
      dup2(lpd[0], 0);
      close(lpd[0]);
      doCommand(tail);
    }
    else{
      doPipe(head, lpd);
    }
  }
  else{
    doCommand(cmdLine);
  }
}

int scan(char *cmdLine){
  for(int i = strlen(cmdLine)-1; i > 0 ; i--){
    if(cmdLine[i] == '|' ){
      cmdLine[i] = 0;
      tail = cmdLine + i + 1;
      head = cmdLine;
      return 1;
    }
  }
  head = cmdLine;
  tail = NULL;
  return 0;

}



int doCommand(char *cmdLine)
{
  memset(name, 0, sizeof(name));
  //resetStreams();
  tokenizeLine(cmdLine);
  ioRedirection();
  char cmd[64];
  if(name[0][0] == '.' && name[0][1] =='/'){
    char temp[MAX];
    getcwd(temp, MAX);
    strcat(temp, "/");
    strcat(temp, name[0]);

    execve(temp,name, __environ);
  }
  for (int i = 0; i < ndir; i++)
  {

    strcpy(cmd, dir[i]);
    strcat(cmd, "/");
    strcat(cmd, name[0]);
    //printf("i=%d cmd=%s\n", i, cmd);

    execve(cmd, name, __environ);
  }
  printf("cmd %s not found, child sh exit\n", name[0]);
  exit(123); // die with value 123
}

void tokenizeLine(char *line)
{
  ntoken = 0;
  char *p = strtok(line, " ");
  while (p != NULL)
  {
    name[ntoken++] = p;
    p = strtok(NULL, " ");
  }

  for (int i = 0; i < ntoken; i++)
  {
    //printf("%s\n", name[i]);
  }
}


void ioRedirection()
{
  for (int i = 0; i < ntoken; i++)
  {
    if (strcmp(name[i], ">") == 0)
    {
      name[i] = 0;
      close(1);
      int fd = open(name[i + 1], O_WRONLY | O_CREAT);
      dup2(fd, 1);
    }
    else if (strcmp(name[i], ">>") == 0)
    {
      name[i] = 0;
      close(1);
      int fd = open(name[i + 1], O_WRONLY | O_CREAT | O_APPEND);
      dup2(fd, 1);
    }
    else if (strcmp(name[i], "<") == 0)
    {
      name[i] = 0;
      close(0);
      int fd = open(name[i + 1], O_RDONLY);
      dup2(fd, 0);
    }
  }
}

void resetStreams()
{
  dup2(fd[0], stdout);
  dup2(fd[1], stdin);
}
