#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/prctl.h>

int main(int argc, char *argv[]) {
	printf("%d\n", getpid());

	char name[32];
	prctl(PR_SET_NAME, "xx", NULL, NULL, NULL);
	prctl(PR_GET_NAME, name, NULL, NULL, NULL);
	printf("rename %s\n", name);

	int i;
	for (i= 0; i < argc; i++) {
		printf("args=%s\n", argv[i]);
	}

	sleep(30);
	return 0;
}
