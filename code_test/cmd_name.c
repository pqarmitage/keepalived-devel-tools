#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <sys/user.h>

extern char **environ;

int main(int argc, char **argv)
{   
    unsigned int pid = getpid();
    char proc_pid_cmdline_path[PATH_MAX];
    char cmdline[PAGE_SIZE];
    
    sprintf(proc_pid_cmdline_path, "/proc/%d/cmdline", pid);
    
    FILE *proc_pid_cmdline =  fopen(proc_pid_cmdline_path, "r");
    fgets(cmdline, PAGE_SIZE, proc_pid_cmdline);
    fclose(proc_pid_cmdline);
    
    printf("%s : %s\nenvironment variable HOME = %s\n", proc_pid_cmdline_path, cmdline, getenv("HOME"));
    
    int env_len = -1;
    if (environ) 
        while (environ[++env_len])
            ;
    
    unsigned int size;
    if (env_len > 0)
        size = environ[env_len-1] + strlen(environ[env_len-1]) - argv[0];
    else
        size = argv[argc-1] + strlen(argv[argc-1]) - argv[0];
    
    if (environ)
    {
        
        char **new_environ = malloc((env_len+1)*sizeof(char *));
        environ[env_len] = NULL;

        unsigned int i = -1;
        while (environ[++i])
            new_environ[i] = strdup(environ[i]);
        
        environ = new_environ;
    }   
        
    // prctl(PR_SET_NAME, "newname");
    
    char *args = argv[0];
    memset(args, '\0', size);
    snprintf(args, size - 1, "This is a new title and it should be definetely longer than initial one. Actually we can write %d bytes long string to the title. Well, it works!", size);
    
    proc_pid_cmdline =  fopen(proc_pid_cmdline_path, "r");
    fgets(cmdline, PAGE_SIZE, proc_pid_cmdline);
    fclose(proc_pid_cmdline);
    
    printf("%s : %s\nenvironment variable HOME = %s\n", proc_pid_cmdline_path, cmdline, getenv("HOME"));
    
    return 0; 
}
