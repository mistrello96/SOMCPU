#include <unistd.h>
#include <fstream>
#include <algorithm>
#include <iostream>
#include <random>
#include <float.h>
#include <assert.h>
#include <omp.h>

#include "utility_functions.h"
#include "distance_functions.h"
#include "update_functions.h"
#include "cmdline.h"

int main(int argc, char **argv)
{
	// reading passed parameters
	gengetopt_args_info ai;
    if (cmdline_parser (argc, argv, &ai) != 0)
    {
        exit(1);
    }

	// INIZIALIZING VARIABLES WITH DEFAULT VALUES
	// path of the input file
	std::string filePath = ai.inputfile_arg;
	// verbose
    bool verbose = ai.verbose_flag;
    // advanced debug
    bool debug = ai.debug_flag;
    // test flag
    bool test = ai.test_flag;
    // save SOM to file
    bool saveall = ai.saveall_flag;
    // save distances to file
    bool savedistances = ai.savedistances_flag;
    // PATH to the folder where saved files will be placed
    std::string savePath = ai.savepath_arg;
    // number of rows in the matrix
    int nRows = ai.nRows_arg;
    // number of columns in the matrix
    int nColumns = ai.nColumns_arg;
    // initial learning rate
    double ilr = ai.initial_learning_rate_arg;
    // final learning rate
    double flr = ai.final_learning_rate_arg;
    // number of iterations(epochs)
    int maxnIter = ai.iteration_arg;
    // initial radius for the update function
    double initialRadius = ai.radius_arg;
    // type of distance
    char distanceType = ai.distance_arg[0];
    // type of neighbour function
    char neighborsType = ai.neighbors_arg[0];
    // type of initialization
    char initializationType = ai.initialization_arg[0];
    // type of lattice
    char lattice = ai.lattice_arg[0];
    // exponential decay for radius and/or learning rate
    char exponential = ai.exponential_arg[0];
    // dataset presentation method
    bool randomizeDataset = ai.randomize_flag;
    // normalization of the average distance of each iteration (epoch)
    bool normalizedistance = ai.normalizedistance_flag;
    // toroidal topology
    bool toroidal = ai.toroidal_flag;
    // counter of epochs/iterations
    int nIter = 0;
    
    if (maxnIter <= 0){
    	std::cout << "The iteration number must be greater than 0" << std::endl;
        exit(0);
    }

        if (ilr < 0){
    	std::cout << "The iteration number must be greater than 0" << std::endl;
        exit(0);
    }

    assert(maxnIter > 0);
    assert(ilr >= 0);

    // DECLARATION OF USEFULL VARIABLES
    // min and max values of neurons (used for random initialization)
    double min_neuronValue, max_neuronValue;
    // number of lines/samples in the input file
    int nSamples;
    // total number of neurons in the SOM
    int nNeurons;
    // total length of the matrix vector (nNeurons*nElements)
    int totalLength;
    // number of features in each sample
    int nElements;
    // learning rate of the ongoing iteration
    double lr;
    // radius of the ongoing iteration
    double radius;
    // accuracy of the ongoing iteration
	double accuracy;
	// index of the sample picked for the iteration
    int currentIndex;
    // BMU distance to the sample
    double BMU_distance;
    // BMU index
    unsigned int BMU_index;
    // file used to save distances of samples to their BMU on the last epoch
    std::ofstream distancesfile;

    omp_set_num_threads(ai.threads_arg);

    //checking the required params
    if(ilr == -1 || maxnIter == -1)
    {
        std::cout << "./a.out: '--initial_learning_rate' ('-s') option required" << std::endl;
        std::cout << "./a.out: '--iteration' ('-n') option required " << std::endl;
        exit(0);
    }

    // READ THE INPUT FILE
    // vector of samples used to train the SOM
    std::vector <double> samples;
    // retrive the number of features in each sample
    nElements = readSamplesfromFile(samples, filePath);

    // COMPUTE VALUES FOR THE SOM INITIALIZATION
    // retrive the number of samples
    nSamples = samples.size() / nElements;
    double *h_Samples = (double *)malloc(sizeof(double) * samples.size());
    // copy from vector to array
    # pragma omp parallel for schedule(guided)
    for (int i = 0; i < samples.size(); i++)
    {
        h_Samples[i] = samples[i];
    }

    // EXTRACTING THE MIN/MAX FROM SAMPLES(only used for random initialization)
    if (initializationType == 'r')
    {
        min_neuronValue = *std::min_element(h_Samples, h_Samples + samples.size());
        max_neuronValue = *std::max_element(h_Samples, h_Samples + samples.size());
	}

	if(nRows < 0 | nColumns < 0)
    {
        std::cout << "Negative number of neuron is not permitted" << std::endl;
        exit(0);
    }

    // estimate the neuron number if not given(using heuristic)
    if (nRows ==0 | nColumns == 0)
    {
    	int tmp = 5*(pow(nSamples, 0.54321));
    	nRows = sqrt(tmp);
    	nColumns = sqrt(tmp);
    }
    if (test)
		assert(nRows > 0 && nColumns > 0);

    
    // estimate the radius if not given (covering 2/3 of the matrix)
    if (initialRadius == 0)
    {
    	initialRadius = 1 + (std::max(nRows, nColumns)/2) * 2 / 3;
    }
    
    if (test)
    	assert(initialRadius != 0);

    // total number of neurons in the SOM
    nNeurons = nRows * nColumns;
    // total length of the serialized matrix
    totalLength = nRows * nColumns * nElements;

    if (test)
	    assert(nNeurons != 0 && totalLength != 0);
    

    // CHECKING PARAMS COMPATIBILITY
    if(normalizedistance && (distanceType=='t'))
    {
        std::cout << "NormalizeDistance option not avaiable with Tanimoto Distance" << std::endl;
        exit(0);
    }
    assert(!((normalizedistance) && (distanceType=='t')));

    // ALLOCATION OF THE STRUCTURES
    // host SOM representation(linearized matrix)
    double *h_Matrix = (double *)malloc(sizeof(double) * totalLength);
    // host array of the distances, it stores each neuron distance from the sample, used to search BMU
    double *h_Distance = (double *) malloc(sizeof(double) * nNeurons);
    // host distance history array, it stores BMU distance for each sample of the iteration, used to compute accuracy
    double h_DistanceHistory = 0;


    // SOM INIZIALIZATION
    // generating random seed
    std::random_device rd;
    std::mt19937 e2(rd());
    // random values initialization between min and max values included in samples
    if (initializationType == 'r')
    {
        // uniform distribution of values
    	std::uniform_real_distribution<> dist(min_neuronValue, max_neuronValue);
        # pragma omp parallel for schedule(guided)
	    for(int i = 0; i < totalLength; i++)
	    {
	    	h_Matrix[i] = dist(e2); 
	    }
    }
    // initialization with random samples choosen from the input file
    else
    {   
        // uniform distribution of indexes
    	std::uniform_int_distribution<> dist(0, nSamples);
        # pragma omp parallel for schedule(guided)
	    for (int i = 0; i < nNeurons; i++)
	    {
	        int r = dist(e2);
	        for (int k = i * nElements, j = 0; j < nElements; k++, j++)
	        {
	             h_Matrix[k] = samples[r*nElements + j];
	        }
	    }
	}

    // save the initial SOM to file
    if (debug | saveall)
    {
        saveSOMtoFile(savePath + "/initialSOM.out", h_Matrix, nRows, nColumns, nElements);
    }
    
	// inizializing values of lr, radius and accuracy for the first iteration
    lr = ilr;
    radius = initialRadius;
	accuracy = DBL_MAX;

    // debug print
    if(verbose | debug)
    {
        std::cout << "Running the program with " << nRows  << " rows, " << nColumns << " columns, " << nNeurons << " neurons, " << radius << " initial radius, "<< std::endl;
    }

    // initializing index array, used to shuffle the sample vector at each new iteration
    int* randIndexes = new int[nSamples];
    # pragma omp parallel for schedule(guided)
    for (int i = 0; i < nSamples; i++)
    {
    	if (test)
    		assert (i <= nSamples);
    	randIndexes[i] = i;
    }

    if (test)
    	assert (nIter < maxnIter);
    // ITERATE UNTILL MAXNITER IS REACHED
    while(nIter < maxnIter)
    {
    	// randomize sample indexes if required
    	if(randomizeDataset)
        {
    		std::random_shuffle(&randIndexes[0], &randIndexes[nSamples-1]);
        }

        // debug print
        if (debug | verbose)
        {
            std::cout << "Learning rate of this iteration is " << lr << std::endl;
            std::cout << "Radius of this iteration is " << radius << std::endl;
        }

        // open the file used for saving distances of samples during the last iteration
        if(nIter == (maxnIter-1) && (savedistances || saveall))
        {   
            distancesfile.open(savePath + "/distances.out");
        }
            

        // ITERATE ON EACH SAMPLE TO FIND CORRESPONDING BMU
	    for(int s=0; s < nSamples ; s++)
        {
            // computing the sample index for ongoing iteration
            currentIndex = randIndexes[s]*nElements;
    		
    		// computation of distance between neurons and sample
		    switch(distanceType)
            {
		    	case 'e' : compute_distance_euclidean(h_Matrix, h_Samples, currentIndex, h_Distance, nNeurons, nElements); break;
		    	case 's' : compute_distance_euclidean(h_Matrix, h_Samples, currentIndex, h_Distance, nNeurons, nElements); break;
		    	case 'm' : compute_distance_euclidean(h_Matrix, h_Samples, currentIndex, h_Distance, nNeurons, nElements); break;
				case 't' : compute_distance_euclidean(h_Matrix, h_Samples, currentIndex, h_Distance, nNeurons, nElements); break;
		    }
 
            // BMU search
            BMU_distance = h_Distance[0];
            BMU_index = 0;
            #pragma omp parallel
            {
                int index_local = BMU_index;
                double min_local = BMU_distance;  
                #pragma omp for nowait
                for (int i = 1; i < nNeurons; i++) {        
                    if (h_Distance[i] < min_local) {
                        min_local = h_Distance[i];
                        index_local = i;
                    }
                }
                #pragma omp critical 
                {
                    if (min_local < BMU_distance) {
                        BMU_distance = min_local;
                        BMU_index = index_local;
                    }
                }
            }

            // debug
		    if(debug)
            {
			   std::cout << "The minimum distance is " << BMU_distance << " at position " << BMU_index << std::endl;
            }

            // if  required, during the last iteration , save distance to file
            if (nIter == (maxnIter-1) && (savedistances || saveall))
            {
                distancesfile << "The minimum distance of the "<< currentIndex << " sample is " << BMU_distance << " at position " << BMU_index << "\n";
            }

            // compute BMU distance history as required and save it in the history array
            if(!normalizedistance)
            {
                h_DistanceHistory += BMU_distance;
            }
            else
            {
                if(distanceType=='s')
                {
                    h_DistanceHistory += BMU_distance;
                }
                else if (distanceType=='e' | distanceType=='m')
                {
                    BMU_distance = (BMU_distance) * (BMU_distance);
                    h_DistanceHistory += BMU_distance;
                }
            }


            // UPDATE THE NEIGHBORS
            // call the function to update the SOM
            if(radius == 0)
            {
                update_BMU(h_Matrix, h_Samples, lr, currentIndex, nElements, BMU_index);
            }
            else
            {
                if (toroidal)
                {
                    if(lattice == 's')
                        update_SOM_toroidal(h_Matrix, h_Samples, lr, currentIndex, nElements, BMU_index, nRows, nColumns, radius, nNeurons, neighborsType);
                    else
                        update_SOM_exagonal_toroidal(h_Matrix, h_Samples, lr, currentIndex, nElements, BMU_index, nRows, nColumns, radius, nNeurons, neighborsType);
                }
                else
                {
                    if(lattice == 's')
                        update_SOM(h_Matrix, h_Samples, lr, currentIndex, nElements, BMU_index, nColumns, radius, nNeurons, neighborsType);
                    else
                        update_SOM_exagonal(h_Matrix, h_Samples, lr, currentIndex, nElements, BMU_index, nColumns, radius, nNeurons, neighborsType);
                }
            }      
        }

        if (test)
			assert(h_DistanceHistory != 0);
        // END OF EPOCH. UPDATING VALUES
        // computing accuracy as required
        if(!normalizedistance)
        {
            accuracy = h_DistanceHistory;
            h_DistanceHistory = 0;            
        }
        else
        {
            accuracy = sqrt(h_DistanceHistory / nElements) / nSamples;
            h_DistanceHistory = 0;  
        }

        // debug print
        if (verbose | debug)
        {
            std::cout << "Mean distance of this iteration is " << accuracy << std::endl;
        }
        
		// updating the counter of iterations
		nIter ++;

        // updating radius and learning rate
        if (exponential== 'r' | exponential == 'b')
        	radius = (int) (initialRadius * exp(-(double)nIter/(sqrt(maxnIter))));
        else
            radius = (int) (initialRadius - (initialRadius) * ((double)nIter/maxnIter));

        if (exponential== 'l' | exponential == 'b')
        	lr = ilr * exp(- (double)nIter/sqrt(maxnIter)) + flr;
        else 
            lr = ilr - (ilr - flr) * ((double)nIter/maxnIter);
}

	std::cout << "\n\n TRAINING RESULTS" << std::endl;
	std::cout << "\nMean distance of the sample to the trained SOM is " << accuracy << std::endl;

    // save trainde SOM to file
    if (debug | saveall)
    {
        saveSOMtoFile(savePath + "/outputSOM.out", h_Matrix, nRows, nColumns, nElements);
    }

	//freeing all allocated memory
    free(h_Matrix);
    free(h_Distance);
    free(randIndexes);

}