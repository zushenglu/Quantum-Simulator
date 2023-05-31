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

    float complex ** lgm = malloc(sizeof(float complex*)* mx1h*mx2h);
    // printf("%d\n", mx1h*mx2h);
    for (int mx1y=0; mx1y<mx1h; mx1y++){ 
        
        // init for row 1 tp
        for (int i=0; i<(mx2h);i++){
            lgm[mx2h*mx1y+i] = malloc(sizeof(float complex)* mx1w*mx2w);
            // printf("%d %d\n", mx2h*mx1y+i, mx1w*mx2w);
        }

        for (int mx1x=0; mx1x<mx1w; mx1x++){

            // for every item in mx 1, perform scalar trans on mx2
            // printf("%d %d\n", mx1y, mx1x);
            float complex scalar = mx1[mx1y][mx1x];

            for (int mx2x=0; mx2x<mx2w; mx2x++){
                for (int mx2y=0; mx2y<mx2h; mx2y++){
                    // printf("%d %d\n", mx1y*mx2h+mx2y, mx1x*mx2w+mx2x);
                    lgm[mx1y*mx2h+mx2y][mx1x*mx2w+mx2x] = scalar * mx2[mx2y][mx2x];
                    // lgm[mx1y*mx2h+mx2y][mx1x*mx2w+mx2x] = 5;

                }
            }
            
        }
    }
    return lgm;


}

float complex** TS_MPDL(float complex ** mx1, float complex ** mx2, int mx1w, int mx1h, int mx2w,
    int mx2h){

    float complex ** lgm = malloc(sizeof(float complex*)* mx1h*mx2h);
    // printf("%d\n", mx1h*mx2h);
    for (int mx1y=0; mx1y<mx1h; mx1y++){ 
        
        // init for row 1 tp
        for (int i=0; i<(mx2h);i++){
            lgm[mx2h*mx1y+i] = malloc(sizeof(float complex)* mx1w*mx2w);
            // printf("%d %d\n", mx2h*mx1y+i, mx1w*mx2w);
        }

        for (int mx1x=0; mx1x<mx1w; mx1x++){

            // for every item in mx 1, perform scalar trans on mx2
            // printf("%d %d\n", mx1y, mx1x);
            float complex scalar = mx1[mx1y][mx1x];

            for (int mx2x=0; mx2x<mx2w; mx2x++){
                for (int mx2y=0; mx2y<mx2h; mx2y++){
                    // printf("%d %d\n", mx1y*mx2h+mx2y, mx1x*mx2w+mx2x);
                    lgm[mx1y*mx2h+mx2y][mx1x*mx2w+mx2x] = scalar * mx2[mx2y][mx2x];
                    // lgm[mx1y*mx2h+mx2y][mx1x*mx2w+mx2x] = 5;

                }
            }
            
        }
    }
    
    free(mx1);
    free(mx2);
    return lgm;


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

// apply given gate, return statevector after complete the transformation
float complex* APPLY_qbt_gate(float complex* cur_state, int sv_len, Operation* op, int qbt_ind, int tot_qbt, float complex** identity){

    float complex** gmx = op->gate->mx;

    if (op == NULL){
        return cur_state;
    }

    // PRINT_VECTOR(cur_state, sv_len);
    // PRINT_MX(identity,2);
    float complex** lgm;
    if (qbt_ind == tot_qbt-1){ // multiply special first
        lgm = TS_MPD(gmx,identity,2,2,2,2);
    }
    else {
        if (qbt_ind == tot_qbt-2){ // multiply special second
            lgm = TS_MPD(identity,gmx,2,2,2,2);
        }
        else{
            lgm = TS_MPD(identity, identity,2,2,2,2);
        }
    }
    // PRINT_MX(lgm,4);
    int lgm_size = 4;
    for (int i=tot_qbt-3;i>=0;i--){
        
        if (i == qbt_ind){
            lgm = TS_MPD(lgm,gmx,lgm_size,lgm_size,2,2);
        }
        else{
            lgm = TS_MPD(lgm,identity,lgm_size,lgm_size,2,2);
        }

        lgm_size *= 2;
    }

    // PRINT_MX(lgm, lgm_size);

    cur_state = MX_MAPL(cur_state,pow(2,tot_qbt),lgm,pow(2,tot_qbt));

    return cur_state;
}

float complex* APPLY_qbts_gate(float complex* cur_state, int sv_len, Qubit *qbt, int qbt_ind, int tot_qbt, float complex** identity){

    return cur_state;
}

void simulate(Circuit* circuit){

    int tot_qbt = circuit->size;
    int sv_size = pow(2,tot_qbt);
    float complex *statevector = calloc(pow(2,sv_size),sizeof(float complex));
    float complex **Identity = initI()->mx;
    
    // initialize state
    statevector[0] = 1;

   // apply h gate on q0

    statevector = APPLY_qbt_gate(statevector, sv_size, circuit->Q[0]->next,0,tot_qbt,Identity);
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

