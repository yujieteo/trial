/* We are going to solve the 1D Poisson Equation today and illustrate the use of the finite difference scheme. 
The simplest possible example is to take u'' = sin(pi * x).
Firstly, we will import some libraries.
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* Generates an array of N = n + 2 equally spaced points from 0 to 1.*/
double* linspace(int n) {
    /* You must consider the start and end points.*/
    int N = n + 2;

    /* We now allocate enough memory using malloc, also check if the memory allocation works.*/
    double* x = (double*)malloc(N * sizeof(double));
    if (!x) return NULL;
    
    /* Divide the unit interval equally for all these points. */
    double h = 1.0 / (N - 1);

    /* Generate the linear space using a for loop here.*/
    for (int i = 0; i < N; i++) {
        x[i] = i * h;
    }
    return x;
}

/*  Computes f(x) = sin(pi * x) for interior points of x (excluding first and last) */
double* compute_sin_pix(const double* x, int n) {
    /* Again, these are the total points including the boundaries.*/
    int N = n + 2;             
    
    /* We now allocate enough memory using malloc, also check if the memory allocation works.*/    
    double* f = (double*)malloc(n * sizeof(double));
    if (!f) return NULL;           

    /* Generate the linear space for sin(pi * x) using a for loop here.
    Note that M_PI is the value of pi in the math library.
    */
    for (int i = 0; i < n; i++) {
        f[i] = sin(M_PI * x[i + 1]); 
    }
    return f;
}

/* The main objective of this article is to introduce the finite difference scheme. Why a tridiagonal matrix? This comes from the basic concept of the second order finite differences. When you consider u''(x) for 1D equation, each interior point u_i is coupled with its neighbours. For a Poisson's equation, we have -u''(x_i) = -(u_{i-1} - 2u_i + u_{i+1})/h^2. This can be obtained if you perform the derivatives there. There are only nonzero entries for u_{i-1}, u_{i} and u_{i+1}. Therefore, the matrix A has nonzero elements only on the main diagonal (2) and the adjacent diagonals (-1). Hence, a tridiagonal matrix is used. To make it easy, we use Dirichlet boundary conditions to solve only the interior points.
*/

/* We will need to solve the tridiagonal system of A u = f.
The following <a href="https://en.wikipedia.org/wiki/Tridiagonal_matrix_algorithm">
Wikipedia page</a> is a good reference for this. Note that in practice, Thomas's algorithm used here is not stable.*/

void solve_tridiagonal(double* a, double* b, double* c, double* f, double* u, int n) {
    /* Here we remember to allocate the c_star and d_star as per Thomas's algorithm. */
    double* c_star = malloc(n * sizeof(double));
    double* d_star = malloc(n * sizeof(double));

    /* The initials are normalised by the b_i. */
    c_star[0] = c[0] / b[0];
    d_star[0] = f[0] / b[0];

    /* For the other cases this for loop is run from 2 to n - 1, recall that i < n means stop at n-1.*/
    for (int i = 1; i < n; i++) {
        /* m is the value in the denominator. */
        double m = b[i] - a[i] * c_star[i-1];
        /* The new c_star is obtained here. */
        c_star[i] = c[i] / m;
        /* The new d_star is obtained here. */
        d_star[i] = (f[i] - a[i] * d_star[i-1]) / m;
    }

    /* The back substitution is done here. Recall that we have already divided by b_i already, no need to divide again.*/
    u[n-1] = d_star[n-1];
    for (int i = n-2; i >= 0; i--) {
        u[i] = d_star[i] - c_star[i] * u[i+1];
    }
    
    /* Don't forget to free after using.*/
    free(c_star);
    free(d_star);
}

/* Recall that for a harmonic oscillator, when you integrate it, the sign flips, and you must divide by 1/pi^2 since the term inside the sine is pi * x, for sin(pi x).
*/
double* analytic_solution(const double* x, int N) {
    double* ua = malloc(N * sizeof(double));

    /* This is simply a routine computation using a for loop for the entire linear array. */
    for (int i = 0; i < N; i++) ua[i] = sin(M_PI * x[i]) / (M_PI*M_PI);
    return ua;
}

/* We have finally come to the main body of the function. */

int main() {
    
    /* We use n = 50 for the number of points, and generate the linear spaces. */

    int n = 50;
    double* x = linspace(n);
    double* f = compute_sin_pix(x, n);
    double h = x[1] - x[0];

    /* Setup tridiagonal matrix for -u'' with Dirichlet boundary conditions.
    This is the most important step.
    */

    /* Let a be the subdiagonal. */
    double* a = malloc(n * sizeof(double)); 

    /* Let b be the diagonal. */
    double* b = malloc(n * sizeof(double)); 

    /* Let c be the superdiagonal. */
    double* c = malloc(n * sizeof(double));

    /* We now enter the conditions for the finite difference scheme in this for loop. */
    for (int i = 0; i < n; i++) {
        a[i] = c[i] = -1.0 / (h*h);
        b[i] = 2.0 / (h*h);
    }

    /* These are the Dirichlet boundary condition. Both ends are zero. */
    a[0] = 0;   
    c[n-1] = 0;

    /* We now set up the empty matrix of u.*/
    double* u = malloc(n * sizeof(double));
    solve_tridiagonal(a, b, c, f, u, n);

    /* Compare this against the analytic solution by integrating.*/
    double* ua = analytic_solution(x, n+2);

    /* A comparison between the analytical solution of this equation is printed. */
    for (int i = 0; i < n; i++) {
        printf("x=%.3f numeric=%.6f analytic=%.6f error=%.2e\n", x[i+1], u[i], ua[i+1], fabs(u[i]-ua[i+1]));
    }

    
    /* Do not forget to free after use, and return freely.*/
    free(x); free(f); free(a); free(b); free(c); free(u); free(ua);
    return 0;
}

/*
The command I put in the terminal to compile this is as follows (I named my file 043-1d_poisson.c):
gcc -o 043-1d_poisson.o 043-1d_poisson.c
*/

/*
I have compiled this in gcc, here are the results I have obtained:
<pre><code>
x=0.020 numeric=0.006239 analytic=0.006237 error=1.97e-06
x=0.039 numeric=0.012455 analytic=0.012451 error=3.94e-06
x=0.059 numeric=0.018624 analytic=0.018618 error=5.89e-06
</code></pre>

The results are very close. The finite difference scheme is a basic concept in the development of finite element codes or computational fluid dynamic codes, where one may not have an analytical solution.
*/