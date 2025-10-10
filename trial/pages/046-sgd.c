/* This blog post is a discussion on the stochastic gradient descent algorithm. The following is adapted from pytorch's optim library. I am just present it here. We first start with some libraries. The original implementation is by Clement Farabet, as of 2012. I tried to find the primary literature for this implementation but I could not find it.
*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
/* The hyperparameters are as follows - (1) the learning rate; (2) the learning rate decay; (3) the weight decay; (4) the momentum decay; (5) the dampening for the momentum; (6) nesterov momentum.
*/
typedef struct {
/*
Recall that in the Taylor's expansion of f(x - eta g)
it is approximately f(x) - eta ||g||^2 when eta is small.
The learning rate, eta must be controlled to guarantee descent.
*/
    double learningRate;
/*
The step size must shrink as it approaches the local minima,
we have eta_t = eta_0 / (1 + td), where d is the decay factor.
*/
    double learningRateDecay;
/*
Regularisation is done by adding lambda ||x||^2 to the objective to encourage small way, the gradient is modified from grad f(x) to grad(fx) + lambda x
*/
    double weightDecay;
/*
As the gradient step accumulates velocity, oscillations are smoothened so that convergence is accelerated in valleyes. Consider the "convex" combination of v_{t+1} = mu v_t + (1 - damp) grad f(x_t), where x_{t+1} = x_t - eta v_{t+1}, we have damp = dampening variable.
*/
    double momentum;
    double dampening;
/*
This is a more subtle point, this is a look ahead correction that evaluates the gradient at the next anticipated position to improve the convergence rate.
x_{t+1} = x_{t} - eta(mu v_t + grad(f(x_t + mu v_t)))
*/
    int nesterov;
} SGDConfig;
/* This is an evaluation counter, it is optional but you can evaluate the velocity with this.*/
typedef struct {
    int evalCounter;
    double *velocity;
} SGDState;
/* This is the function to optimise, it is y = (x - 3)^2 = x^2 - 6x + 9.
The minimum is clearly at x = 3, with a minimum of 0.
The objective function is evaluated in the SGD and is called from there.
*/
double opfunc(double *x, double *grad, int n) {
    double fx = 0.0;
    /* The four loop starts with the addition (x-3)*(x-3) twice.*/
    for (int i = 0; i < n; i++) {
        double diff = x[i] - 3.0;
        fx += diff * diff;
        grad[i] = 2.0 * diff;
    }
    return fx;
}
/* Now this is the main implementation of the stochastic gradient descent. We put in the function we want, as well as the x values.
SGDConfig holds the hyperparameters.
SGDState tracks iteration count and momentum buffer.
*/
void sgd(
    double (*opfunc)(double*, double*, int),
    double *x, int n,
    SGDConfig *config,
    SGDState *state
) {
    /*
    We allocate memory for the gradient pointer, and we get the value of fx.
    */
    double *grad = (double*)malloc(n * sizeof(double));

    /* Evaluates the objective and the gradient at the current parameters.*/
    double fx = opfunc(x, grad, n);

    /* Apply weight decay. This adds lambda x to the gradient. 
    If the weight decay is known zero, then add lambda * x to the gradient. Recall that we want to minimise L(x) = f(x) + lambda/2 ||x||^2 for second order term, taking derivative this must mean we add lambda x.
    */
    if (config->weightDecay != 0) {
        for (int i = 0; i < n; i++)
            grad[i] += config->weightDecay * x[i];
    }

    /* If velocity is used as a hyperparameter, and momentum is nonzero, then initialise the velocity.
    */
    if (config->momentum != 0 && state->velocity == NULL) {
        state->velocity = (double*)calloc(n, sizeof(double));
    }

    /* This is simply the equation:
    v_{t+1} = mu v_t + (1 - damp) grad f(x_t), where x_{t+1} = x_t - eta v_{t+1}, you can parse and check that this is the case. The momentum, mu, is a sort of memory term that controls how much of the previous direction is kept, this follows the structure of a low pass filter.
    You can parse the equation.
    */
    if (config->momentum != 0) {
        for (int i = 0; i < n; i++) {
            state->velocity[i] =
                config->momentum * state->velocity[i]
                + (1.0 - config->dampening) * grad[i];

            /* This is simply the Nesterov correction: you multiply the momentum with the velocity, x_{t+1} = x_{t} - eta(mu v_t + grad(f(x_t + mu v_t))), by making a guess of where the momentum takes you and correcting the gradient, convergence speed increases.*/
            if (config->nesterov)
                grad[i] += config->momentum * state->velocity[i];
            else
                grad[i] = state->velocity[i];
        }
    }

    /* The learning rate decay follows: eta_t = eta_0 / (1 + td), where d is the decay factor. Note that this prevents oscillations around the minimum, so you want to decrease eta_t over time. Note that this looks like a damping equation if you took a course on structural vibration.*/
    double clr = config->learningRate /
                 (1.0 + state->evalCounter * config->learningRateDecay);

    /* The parameters are updated for x as it gets closer. This is the key step, the gradient is the steepest increase, so stepping in the negative direction decreases the function faster.*/
    for (int i = 0; i < n; i++)
        x[i] -= clr * grad[i];

    /* The evaluation counter is incremented, and the step is printed out for your convenience. This holds the history to make the momentum recurrence and the evaluation counter to compute the learning decay rate correctly.*/
    state->evalCounter++;
    printf("Step %d: f(x)=%.6f, x=%.6f\n", state->evalCounter, fx, x[0]);

    /* Do not forget to free the gradient.*/
    free(grad);
}

/* The main body of the function is taken.*/
int main() {
    int n = 1;
    /* This is merely an inital guess.*/
    double x[1] = { 10.0 }; 

    /* These are initialising the hyperparameters.*/
    SGDConfig config = {
        .learningRate = 0.1,
        .learningRateDecay = 0.0,
        .weightDecay = 0.0,
        .momentum = 0.9,
        .dampening = 0.0,
        .nesterov = 1
    };
    SGDState state = { .evalCounter = 0, .velocity = NULL };

    /* Now we run this operation for 50 loops, see how close we get.*/
    for (int i = 0; i < 50; i++) {
        sgd(opfunc, x, n, &config, &state);
    }

    /* Do not forget to free the state.velocity, and exit out gracefully.*/
    free(state.velocity);
    return 0;
}

/* My results with show: Step 50: f(x)=0.000002, x=2.999670 for the very last step.
*/