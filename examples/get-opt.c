#include <stdio.h>
#include <string.h>
#include <getopt.h>
int main(int argc, char *argv[]) {
	int opt = 0;
	while ((opt = getopt(argc, argv, "ab:c::")) != -1) {
		switch (opt) {
			case 'a':
			printf("Option a sans parametre\n");
			break;
			case 'b':
			printf("Option b avec parametre %s\n", optarg);
			break;
			case 'c':
			printf("Option c ");
			if (optarg) {
				printf("avec parametre %s\n", optarg);
			} else {
				printf("sans parametre\n");
			}
			break;
		}
	}
	return 0;
}