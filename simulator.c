#include <stdio.h>
#include <complex.h>
#include <stdlib.h>
#include <math.h>

#include "simulator.h"
#include "qubit.h"
#include "gate.h"
#include "circuit.h"


/*
    general idea:
        read the circuit by circuit depth
        apply gate to each qubit
        pass resulting circuit onto measurement 

    how to deal with multi-qubit gate:
        since circuit have placeholder gates, all qubits affected by the
        gate will be in the same depth. 
        
        In this case we check the param_id and take tensor product
            should be a vector of [2xn] x 1 where n = num of qubits

        then map the vector with given gate to get final state vector V

        (need more research on the topic and proof it)
        resulting vector should give amplitude for each qubit in the form
        (assuming i start on 1):
            param_qubit[i].a = V[(n/(2i))-1] 
            param_qubit[i].b = V[(n/(2i))] 

        
    
    check this site:
        https://learn.microsoft.com/en-us/azure/quantum/concepts-multiple-qubits


*/

/*
    Damn ngl i think its kinda fucked lmao ...

    path1: read them all together, create a big array for all states and modifi them by steps
        requires restructre the simulator
        requires some math for details of implementaion

    path2: keep the same structure, but when entanglement happens, make it dependent while keep
    the original state. modify it when state ended
        requires extra process for entanglement and keeping the state
        requires some tha for details

    path3: Stop acting like you know the way like you know the rules! 
        there are no rules man we are lost :(  
    

*/

void PRINT_VECTOR(float complex *vector, int size){

    for (int i=0;i<size;i++){
        printf("\n%d\t", i);

        PRINT_COMPLEX(vector[i]);
    }
    printf("\n");

}

float complex** TS_MPD(float complex ** mx1, float complex ** mx2, int mx1w, int mx1h, int mx2w,
    int mx2h){

    float complex ** ftg = malloc(sizeof(float complex*)* mx1h*mx2h);
    // printf("%d\n", mx1h*mx2h);
    for (int mx1y=0; mx1y<mx1h; mx1y++){ 
        
        // init for row 1 tp
        for (int i=0; i<(mx2h);i++){
            ftg[mx2h*mx1y+i] = malloc(sizeof(float complex)* mx1w*mx2w);
            // printf("%d %d\n", mx2h*mx1y+i, mx1w*mx2w);
        }

        for (int mx1x=0; mx1x<mx1w; mx1x++){

            // for every item in mx 1, perform scalar trans on mx2
            // printf("%d %d\n", mx1y, mx1x);
            float complex scalar = mx1[mx1y][mx1x];

            for (int mx2x=0; mx2x<mx2w; mx2x++){
                for (int mx2y=0; mx2y<mx2h; mx2y++){
                    printf("%d %d\n", mx1y*mx2h+mx2y, mx1x*mx2w+mx2x);
                    ftg[mx1y*mx2h+mx2y][mx1x*mx2w+mx2x] = scalar * mx2[mx2y][mx2x];
                    // ftg[mx1y*mx2h+mx2y][mx1x*mx2w+mx2x] = 5;

                }
            }
            
        }
    }
    return ftg;


}

float complex** TS_MPDL(float complex ** mx1, float complex ** mx2, int mx1w, int mx1h, int mx2w,
    int mx2h){

    float complex ** ftg = malloc(sizeof(float complex*)* mx1h*mx2h);
    // printf("%d\n", mx1h*mx2h);
    for (int mx1y=0; mx1y<mx1h; mx1y++){ 
        
        // init for row 1 tp
        for (int i=0; i<(mx2h);i++){
            ftg[mx2h*mx1y+i] = malloc(sizeof(float complex)* mx1w*mx2w);
            // printf("%d %d\n", mx2h*mx1y+i, mx1w*mx2w);
        }

        for (int mx1x=0; mx1x<mx1w; mx1x++){

            // for every item in mx 1, perform scalar trans on mx2
            // printf("%d %d\n", mx1y, mx1x);
            float complex scalar = mx1[mx1y][mx1x];

            for (int mx2x=0; mx2x<mx2w; mx2x++){
                for (int mx2y=0; mx2y<mx2h; mx2y++){
                    printf("%d %d\n", mx1y*mx2h+mx2y, mx1x*mx2w+mx2x);
                    ftg[mx1y*mx2h+mx2y][mx1x*mx2w+mx2x] = scalar * mx2[mx2y][mx2x];
                    // ftg[mx1y*mx2h+mx2y][mx1x*mx2w+mx2x] = 5;

                }
            }
            
        }
    }
    
    free(mx1);
    free(mx2);
    return ftg;


}


float complex* MX_MAPL(float complex * vector, int vec_dim, float complex ** mx, int mx_dim){

    float complex *new = (float complex*) calloc(vec_dim, sizeof(float complex));

    for (int i=0;i<mx_dim;i++){

        for (int j=0;j<mx_dim;j++){
            // PRINT_COMPLEX(vector[j]);
            // printf(" * ");
            // PRINT_COMPLEX(mx[i][j]);
            // printf("\n");
            new[i] += mx[i][j] * vector[j];
        }
    }

    free(vector);
    free(mx);
    return new;

    // printf("new state: ");
    // PRINT_COMPLEX(vector[0]);
    // PRINT_COMPLEX(vector[1]);
    // printf("\n");

    // return new;

}



void simulate(Circuit* circuit){

    int tot_qbt = circuit->size;
    int sv_size = pow(2,tot_qbt);
    float complex *statevector = calloc(pow(2,sv_size),sizeof(float complex));
    
    statevector[0] = 1;

    // process qubit index
    int ind = 0;
    Operation* op = circuit->Q[0]->next;

    PRINT_MX(op->gate->mx,op->gate->dimension);

    // init identity, it should be made avaliable for everybody
    Gate* gate = initPX();
    float complex ** Identity = malloc(sizeof(float complex*) * 2);
    Identity[0] = malloc(sizeof(float complex) * 2);
    Identity[1] = malloc(sizeof(float complex) * 2);
    Identity[0][0] = 1;
    Identity[1][1] = 1;
    Identity[0][1] = 0;
    Identity[1][0] = 0;


    // following process should be done iteratively
    // let i = acting qbt ind, n = tot qbts
    // I i times, MPD Gate->mx, TP I n-i-1 times

    // get ftg for this operation (x * I)
    float complex ** ftg = TS_MPD(Identity,gate->mx,2,2,gate->dimension, gate->dimension);
    ftg = TS_MPD(ftg, Identity, 4,4,2,2);

    PRINT_MX(ftg, 4 * gate->dimension); 

    // map statevector
    statevector = MX_MAPL(statevector,sv_size, ftg, 4*gate->dimension);
    PRINT_VECTOR(statevector,sv_size);


}

void process_qbt(Circuit*, int qbt_ind){
    
}


float complex* MX_MAP(float complex * vector, int vec_dim, float complex ** mx, int mx_dim){

    float complex *new = (float complex*) calloc(vec_dim, sizeof(float complex));

    for (int i=0;i<mx_dim;i++){

        for (int j=0;j<mx_dim;j++){
            new[i] += mx[i][j] * vector[j];
        }
    }

    return new;

}

float complex* TS_PD(float complex *vector1, int vec1_dim, float complex *vector2, int vec2_dim){

    float complex* new = (float complex*) calloc(vec1_dim * vec2_dim, sizeof(float complex));

    for (int i=0; i<vec1_dim;i++){

        for (int j=0; i<vec2_dim; j++){

            new[i*vec2_dim+vec2_dim] = vector1[i] * vector2[j];

        }
    }

    return new;

}

void Transform(int * qbts, Circuit *qc, int qbt_num){

    

}

float complex* Init_QS(int qbts){
    float complex* a = calloc(2*qbts, sizeof(float complex));
    return a;
}





/*

a = [1/sqrt2 1/sqrt2] b = [1/sqrt2 1/sqrt2]
a ox b = [ 1/2 1/2 1/2 1/2]

[1,0,0,0     [1/2    [1/2   [00           [1/4
 0,1,0,0  ox  1/2  =  1/2 =  01   => pr =  1/4
 0,0,0,1      1/2     1/2    10            1/4
 0,0,1,0]     1/2]    1/2    11]           1/4]

*/

