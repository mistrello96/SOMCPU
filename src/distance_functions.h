#ifndef DISTANCE_FUNCTIONS_H
#define DISTANCE_FUNCTIONS_H

// function used to compute euclidean distance between each neuron and the selected sample
void compute_distance_euclidean(double* h_matrix, double* h_Samples, int currentIndex, double* h_distance, int nNeuron, int nElements);

// function used to compute sum of squares distance between each neuron and the selected sample
void compute_distance_sum_squares(double* h_matrix, double* h_Samples, int currentIndex, double* h_distance, int nNeuron, int nElements);

// function used to compute manhattan distance between each neuron and the selected sample
void compute_distance_manhattan(double* h_matrix, double* h_Samples, int currentIndex, double* h_distance, int nNeuron, int nElements);

// function used to compute tanimoto distance between each neuron and the selected sample
void compute_distance_tanimoto(double* h_matrix, double* h_Samples, int currentIndex, double* h_distance, int nNeuron, int nElements);

#endif