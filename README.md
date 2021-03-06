# SOMCPU
A parallel C++ implementation of Self-Organizing Maps for unsupervised learning.
The learning process is done by competitive learning between neurons of the SOM.
The closest unit to the input vector is called Best Machine Unit (BMU) and its weigth vector is moved in the direction of the input vector.
Also the neighbourhood of the BMU is moved in the same direction, but with lower magnitude, according to the distance from the BMU.
This tool allows to specify the following parameters used in the learning process:
- Number of rows of the SOM
- Number of columns of the SOM
- Initial and final learning rate
- Number of iterations in the learning process
- Radius of the updating function
- Various type of distances for the BMU search (Euclidean, Sum of Squares, Manhattan, Tanimoto)
- Various neighbour function (gaussian, bubble, mexican hat)
- Two types of lattice for the neurons of the SOM (square or exagonal)
- Possibility to use a toroidal topology
- Possibility to decay the learning rate and/or the radius exponentially
- Number of threads

The input samples must be provided in tab-separated format.

For all the other functions, please refer to the help menu included in the tool
--------------------------------------------------------

Here you can find some examples of the tool uses:
1) The tool will use the file provided as input, will train for 1000 iterations with a learning rate that 
will linearly decay from 0.1 to 0.001. The distance between neurons and the input vector will be normalized. 
The size of the SOM and the radius of the updating function will be estimated runtime with heuristcs. Neurons will be represented by exagones.

./CUDA-SOM.out -i /folder/folder/inputfile.txt -n 1000 -s 0.1 -f 0.001 --normalizedistance

2) The tool will use the file provided as input, will train for 5000 iterations with a learning rate that 
will linearly decay from 0.3 to 0.001. The size of the SOM is set to 200x200, the radius of the updating finction (bubble)
is set to 50, and will decrease exponentially. The distance between neurons and the input vector is computed 
using the sum of squares distance. Once the SOM is trained, the distances between the input file and the SOM will be
saved to a file.

./CUDA-SOM.out -i /folder/folder/inputfile.txt -n 5000 -s 0.3 -f 0.001 -x 200 -y 200 --savedistances -r 50 --distance=s --neighbors=b --toroidal --exponential=r

a