#include <stdio.h>
#include <stdlib.h>

int main(){
    char *t;
    while(1){
        t=malloc(10000* sizeof(char));
        free(t);
    }
}