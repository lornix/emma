#include <stdio.h>
#include <stdlib.h>


int main(__attribute__((unused)) int argc,__attribute__((unused)) char *argv[],char **environ)
{
    char **ep=environ;
    char *p;
    while ((p=*ep++))
    {
	printf("%s\n",p);
    }
    return 0;
}
