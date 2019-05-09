# JobAlloc

Shift scheduling problems as a well-known topic in
operation research has already been researched for many
years [5]. It is the process of assigning shifts to staff in
order to obtain lowest cost [6]. However, in many real-world
cases, the number of shifts and staff can be numerous, and
with the consideration of different constraints such as time
windows, the entire problem can be much more arduous.
This study extends classical shift scheduling problem in
which the transportation cost (or moving cost) is involved.
Taking advantage of the extension of a classical problem,
it shows that our model can be much more adaptive to
be utilized in a variety of real-world cases. Furthermore,
we will formulate our research as a integer programming
(IP) problem and provide the model, some moderate size
of instances can be solved by mixed integer programming
(MIP) solvers such as CPLEX and GUROBI. However, it is
time-consuming; and it is not able for MIP solvers to solve
instances with large numbers of staff and shifts. As a result,
we also propose an iterated local search algorithm which
is efficient and effective to find approximately optimal
solutions.
