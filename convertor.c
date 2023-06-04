#include <stdio.h>
#include <complex.h>
#include <stdlib.h>
#include <math.h>

#include "convertor.h"

// typedef struct struct_convertor {

//     int char_len;
//     enum unit {prob, state};
//     union union_result{
//         float prob;
//         char* state;
//     };
// } Conv_Unit;



char* toBin(int i, int str_len){

    char * bin = malloc(str_len*sizeof(char));
    
    int r = i;
    for (int j=0; j<str_len;j++){
        r = (i/pow(2,str_len-j-1));
        i -= (r*pow(2,str_len-j-1));
        // printf("%d\n",r);    
        if (r != 0){
            bin[j] = '1';
        }
        else{
            bin[j] = '0';
        }
        // bin[j]
    }
    // printf("%s\n",bin);

    return bin;
}

float toProb(float complex n){

    
    // printf("%f\n", pow(creal(n),2) + pow(cimag(n),2));
    return pow(creal(n),2) + pow(cimag(n),2);
}

ResUnit** to_prob(float complex* statevector, int sv_len){

    int str_len = (int) (log(sv_len)/log(2));
    ResUnit ** result = malloc(sizeof(ResUnit*) * sv_len);

    char *bin;
    for (int i=0;i<sv_len;i++){
        ResUnit* info = malloc(sizeof(ResUnit));
        info->prob = toProb(statevector[i]);
        info->state = toBin(i, str_len);
        // printf("%d\t%d\t%s\n", i, str_len, toBin(i,str_len));
        result[i] = info;
    }
    return result;

}

void PRINT_RESULT(ResUnit** result, int res_len){
    printf("\nResult:\n\tstate\t\tprobability\n");

    for (int i=0;i<res_len;i++){
        printf(" %d\t%s\t\t%.4f\t%%\n", i, result[i]->state, result[i]->prob*100);
    }
} 
