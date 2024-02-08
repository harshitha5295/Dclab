#include <stdio.h>
#include "add.h"  // Include the header file containing the definition of the 'numbers' structure

int *
add_1_svc(numbers *argp, struct svc_req *rqstp)
{
    static int result;

    /* Server code to add two numbers */

    printf("Received numbers: %d and %d\n", argp->a, argp->b);

    result = argp->a + argp->b;

    return &result;
}

