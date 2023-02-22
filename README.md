# CVRPTW_tabu_search
My approach to CVRPTW problem, solved using tabu search algorithm.

Program is using data stored in .txt file in Solomon's format:

problemname\n
VEHICLE\n
NUMBER CAPACITY\n
K Q\n
CUSTOMER\n
CUST NO. XCOORD. YCOORD. DEMAND READY TIME DUE DATE SERVICE TIME\n
i_0 x_0 y_0 q_0 e_0 l_0 d_0\n
...
i_n x_n y_n q_n e_n l_n d_n\n

where "\n" is new line's sign. K means vehicles quantity. All the numbers are in int format. Blank lines must be ignored.

Program's call example:
\tabu_search.exe data.txt

result is stored in .txt file called "wynik.txt". Data need to be interpreted as follows:

Route'sQuantity AllRoute'sLength\n
nr_1._vertex_in_1.route nr_2._vertex_in_1.route ...\n,
...
nr_1._vertex_in_last.route nr_2._vertex_in_last.route ...\n
where "\n" is new line's sign.

If there is no valid solution to the problem route's Quantity in the result file will be equal -1.

