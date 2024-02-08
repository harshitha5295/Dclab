#include <stdio.h>
#include <stdlib.h>
#include "add.h"  // Include the header file containing RPC function declarations

void add_p_1(char *host, int x, int y) {
    CLIENT *clnt;
    int *result_1;
    numbers add_1_arg;

#ifndef DEBUG
    clnt = clnt_create(host, ADD_p, ADD_v, "udp");
    if (clnt == NULL) {
        clnt_pcreateerror(host);
        exit(1);
    }
#endif /* DEBUG */

    add_1_arg.a = x;
    add_1_arg.b = y;

    result_1 = add_1(&add_1_arg, clnt);
    if (result_1 == (int *) NULL) {
        clnt_perror(clnt, "call failed");
    } else {
        printf("Result is %d\n", *result_1);  // Print the result with a newline
    }

#ifndef DEBUG
    clnt_destroy(clnt);
#endif /* DEBUG */
}

int main(int argc, char *argv[]) {
    char *host;

    if (argc != 4) {  // Check if the user provides correct number of arguments
        printf("usage: %s server_host num1 num2\n", argv[0]);
        exit(1);
    }

    host = argv[1];
    add_p_1(host, atoi(argv[2]), atoi(argv[3]));
    exit(0);
}

