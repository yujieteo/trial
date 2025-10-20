/*Search, A* optimality condition: evaluate all possible outcomes (large memory) without overestimating cost (admissible) and monotone (always move closer) to objective. Calculate cost now and add heuristic cost.*/
/*Search, A* fixed time: update dynamics in real time, or look at fixed depth and repeat A* fixed depth search one step at a time.*/
/*Search, pruning and beam search: check bounded many outcomes prune the worst proportion. Or use A*, prune the worst when memory is full. */
/*Search, tabu list: forbit returning to visited states unless it is really good.*/
/*Search, simulated annealing: use high randomness at the beginning to escape local optima, lower randomness later.*/
/*Search, bisection: either start from where you are and the goal to meet in the middle, or do binary search and cut half each time (Shannon entropy)*/
/*Search, repairable improvement and constraint propagation: find a feasible solution first, repair later. Related, find a solution meeting the constraints, backtrack when violated. Key word: define objectives and constraints violations*/
/*Search, lookahead: look ahead when you can to avoid problems.*/
/*Search, memory and time tradeoffs: think about tradeoffs in memory (evalauting all possible outcomes), and time (opportunity cost of bad search, time needed).*/
/*Search, multiple objective Pareto optimality and scalarisation: first step is always to identify outcomes, scalarisation means weigh your different outcomes to one outcome, Pareto optimality means I must make adjacent outcomes worse.*/
/*Search, bitter lesson in scalability: make sure that all of the experience possible (not just data, scale to the human experience or general experience) at all scales (atomic, molecular, physical, metaphysical) is possible.*/