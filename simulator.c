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

// extract a 2x2 square matrix at start_ind
float complex ** ExtractSqrtMx(float complex** mx,int start_x, int start_y){

    float complex ** tbtmx = malloc(sizeof(float complex*)*2);
    tbtmx[0] = calloc(2, sizeof(float complex));
    tbtmx[1] = calloc(2, sizeof(float complex));

    tbtmx[0][0] = mx[start_y][start_x];
    tbtmx[0][1] = mx[start_y][start_x+1];
    tbtmx[1][0] = mx[start_y+1][start_x];
    tbtmx[1][1] = mx[start_y+1][start_x+1];

    free(mx);
    return tbtmx;
}

float complex ** MX_ADD(float complex ** mx1, float complex ** mx2, int mx_size){

    for (int y=0; y<mx_size;y++){

        for (int x=0; x<mx_size;x++){
            mx1[y][x] = mx1[y][x] + mx2[y][x];
        }
    }

    free(mx2);
    return mx1;

}

// apply controlled gate, currently only 1-1 control supported
float complex* APPLY_C_gate(float complex* cur_state, int sv_len, Operation* op, int qbt_ind, int tot_qbt, float complex** identity){

    // PRINT_VECTOR(cur_state,sv_len);
    int impacted_qbt = op->impacted_qbts_num;
    int c_ind = op->impacted_qbts[0];
    int x_ind = op->impacted_qbts[1];

    float complex ** Xmx = ExtractSqrtMx(op->gate->mx, 2,2);

    // PRINT_MX(Xmx, 2);

    float complex ** Cs1 = malloc(sizeof(float complex * )*2);
    Cs1[0] = calloc(2,sizeof(float complex));
    Cs1[1] = calloc(2,sizeof(float complex));
    Cs1[1][1] = 1;
    float complex ** Cs0 = malloc(sizeof(float complex *)*2);
    Cs0[0] = calloc(2,sizeof(float complex));
    Cs0[1] = calloc(2,sizeof(float complex));
    Cs0[0][0] = 1;
    // PRINT_MX(Cs1, 2);
    // PRINT_MX(Cs0, 2);


    // printf("%d %d\n", c_ind, x_ind);

    // with n qubits, wheres n-1 tensor product
    // CX 3->1 with 5 qbt = I I I I [1,0,0,0] I + I X I I [0,0,0,1] I
    // with n qubits control
    float complex ** lgm0 = NULL;
    int lgm0_len = 0;
    float complex ** lgm1 = NULL;
    int lgm1_len = 0;
    
    // // get cont of |0><0|
    for (int i=tot_qbt-1; i>=0; i--){
        // printf("%d\n",i+1);

        // PRINT_MX(lgm0,lgm0_len);
        // printf("\n");
        if (i == c_ind){
            if (lgm0 == NULL){
                lgm0 = Cs0;
                lgm0_len += 2;
            }
            else{
                lgm0 = TS_MPD(lgm0,Cs0,lgm0_len,lgm0_len,2,2);
                lgm0_len *= 2;
            }
            continue;
        }
        if (i == x_ind) {
            if (lgm0 == NULL){
                lgm0 = Cs1;
                lgm0_len += 2;
            }
            else{
                lgm0 = TS_MPD(lgm0,identity,lgm0_len,lgm0_len,2,2);
                lgm0_len *= 2;
            }
            continue;
        }

        if (lgm0 == NULL){
            lgm0 = identity;
            lgm0_len += 2;
        }
        else{
            // PRINT_MX(lgm0, lgm0_len);
            // printf("\n");
            // PRINT_MX(identity,2);
            lgm0 = TS_MPD(lgm0,identity,lgm0_len,lgm0_len,2,2);
            lgm0_len *= 2;
        }
        
        // PRINT_MX(lgm0, lgm0_len);
    }

    // PRINT_MX(lgm0,lgm0_len);
    
    // // get cont of |1><1|
    for (int i=tot_qbt-1; i>=0; i--){

        if (i == c_ind){
            if (lgm1 == NULL){
                lgm1 = Cs1;
                lgm1_len += 2;
            }
            else{
                lgm1 = TS_MPD(lgm1,Cs1,lgm1_len,lgm1_len,2,2);
                lgm1_len *= 2;
            }
            continue;
        }
        if (i == x_ind) {
            if (lgm1 == NULL){
                lgm1 = Xmx;
                lgm1_len += 2;
            }
            else{
                lgm1 = TS_MPD(lgm1,Xmx,lgm1_len,lgm1_len,2,2);
                lgm1_len *= 2;
            }
            continue;
        }

        if (lgm1 == NULL){
            lgm1 = identity;
            lgm1_len += 2;
        }
        else{
            lgm1 = TS_MPD(lgm1,identity,lgm1_len,lgm1_len,2,2);
            lgm1_len *= 2;

        }
    }

    // PRINT_MX(lgm1, lgm1_len);
    float complex ** lgm = MX_ADD(lgm0,lgm1, lgm1_len);
    // printf("\n");
    // PRINT_MX(lgm,lgm0_len);

    cur_state = MX_MAP(cur_state,sv_len,lgm, lgm0_len);
    // printf("what?\n");
    return cur_state;
}

typedef struct struct_gate_queue{

};


void simulate(Circuit* circuit){


    int tot_qbt = circuit->size;
    int sv_size = pow(2,tot_qbt);
    printf("%ld\n", sv_size );
    printf("%zu\n", sizeof(float complex));
    printf("%zu\n", SIZE_MAX);
    float complex *statevector = calloc(sv_size, sizeof(float complex));

    float complex **Identity = initI()->mx;

    // initialize quantum state
    statevector[0] = 1; // causing seg fault

   // apply h gate on q0
    printf("1\n");

    statevector = APPLY_qbt_gate(statevector, sv_size, circuit->Q[0]->next,0,tot_qbt,Identity);
    // PRINT_VECTOR(statevector,sv_size);
    printf("2\n");

    circuit->Q[0]->next = circuit->Q[0]->next->next;
 
    // perform cx gate
    statevector = APPLY_C_gate(statevector,sv_size,circuit->Q[0]->next,0,tot_qbt,Identity);
    PRINT_VECTOR(statevector,sv_size);


    statevector = APPLY_qbt_gate(statevector, sv_size, circuit->Q[2]->next,2, tot_qbt,Identity);
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

