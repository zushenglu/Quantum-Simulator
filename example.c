#include <stdio.h>
#include <complex.h>

struct Qubit {
    int index;
    // float imaginary x;
    // float imaginary y;
} ;

void complexEX(){
    double complex x = 4.0 * I; 

    printf("z = %.1f%+.1fi\n", creal(x), cimag(x));

    x += 5;
    printf("z = %.1f%+.1fi\n", creal(x), cimag(x));

    x *= 4;
    printf("z = %.1f%+.1fi\n", creal(x), cimag(x));


}

typedef struct S {  // S goes to struct namespace
    int x; 
} S1; // S1 goes to global name space

struct S2 { 
    int x; 
} S2_default={2};

typedef struct S2 SS2;

struct s {
    int x;
} ss1, ss2, sss[20];

void f1( S1 argument ){printf("hi there\n");};
// void ff1( struct S1 argument ){printf("hi there\n");}; bad cus S1 not in struct namespace

void f2( struct S2 argument ){printf("hi there\n");};
// void ff2( S2 argument ){printf("hi there\n");}; bad cus S2 not in global namespace

void structEX(){

    // note that deftype variables cannot be overwritten by function while struct can

    // ex with deftype
        // call it using global namespace
        S1 s;
        s.x = 1;
        printf("%d\n",s.x);
        // f1(s);

        // call it using struct namespace
        struct S s2;
        s2.x = 100;
        printf("%d\n",s2.x);
        // f1(s2);
    
    // ex without deftype;

        // call it with struct namespace
        // S2 ss2; // gives error b/c it is in struct namespace
        struct S2 ss2;
        ss2.x = 2;
        printf("%d\n", ss2.x);
        // f1()

        // call it with alias in the global space
        SS2 sss2;
        sss2.x= 200;
        printf("%d\n", sss2.x);

        // ex as parameter declared in struct namespace
        f2(ss2); 

        // ex as parameter using alias in global namespace
        f2 (sss2);

        struct S2 s2_default_test;
        printf("%d\n",s2_default_test.x);

    // struct s;
    


}


int main(int argnum, char** arg){



    structEX();

    return 0;
}