#include<stdio.h>
#include<sys/types.h>

int main(int argc, char **argv)
{
//	execlp("ls","ls","-l",NULL);
//	execlp("echo", "echo", "hello", "world", NULL);
//	execlp("man", "man", "execlp", NULL);
	execlp("/usr/bin/swupdate","swupdate", "-i", update_file, "-v", "-k", key_file, NULL);

}
