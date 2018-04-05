/**
 * @file main.cxx
 * @author Behrooz Kamary Aliabadi
 * @date 19 Sep 2011
 * @brief The Sparse Associative Memory (SAM)
 *
 * The Sparse Associative Memory (SAM) is an associative memory
 * that resembles the human memory. For more details read the
 * references below.
 *
 * This simulation code reproduces figure 3 in article
 * "Storing Sparse Messages in Networks of Neural Cliques".
 * It is to demonstrate how the neural network which
 * has been presented in the article works.
 *
 * @see http://ieeexplore.ieee.org/document/6658945/
 * @see https://tel.archives-ouvertes.fr/tel-00962603/document
 * @see https://cordis.europa.eu/project/rcn/102141_en.html 
 */

#include <iostream>
#include <fstream>
#include <iomanip>
#include <ctime>

#include "sam.hpp"

int main (int argc, char** argv)
{
	std::srand(std::time(nullptr));

    // network parameters
    
    size_t nc	= 100;	    // The total number of clusters in the network 
    size_t nf	= 64;       // The number of fanals in each cluster
    size_t cmax = 12;       // The maximum message order
    size_t cmin = 12;       // The minimum message order


    // uniformely random generated messages' parameters

    size_t uint_min_num         = 0.5e5;  // The minimum number of learnd messages
    size_t uint_max_num         = 4.5e5;  // The maximum number of learnd messages
    size_t uint_num_steps       = 30;     // The number of simulation steps
    size_t uint_num_step        = (uint_max_num - uint_min_num)/uint_num_steps;
    size_t uint_num_unknowns    = 3;

    // algorithmic parameters

    size_t uint_num_it          = 4;
    size_t uint_num_mc          = 100;     // The observed number of errors for each simulation step



	sam memory(nc,nf);


	std::ofstream fs_results;
    fs_results.open("results.txt", std::ios::out);


    size_t uint_num_clusters;
    size_t uint_randint;

    std::vector <std::vector <size_t>> vec_clusters;
	std::vector <std::vector <size_t>> vec_partial_messages;
	std::vector <std::vector <size_t>> vec_partial_clusters;




    for (size_t uint_step = 0; uint_step < uint_num_steps + 1; uint_step++)
    {

        size_t uint_num_messages = uint_max_num - uint_num_step * uint_step;


        size_t uint_errors_guided   = 0;
        size_t uint_errors_blind    = 0;
        float float_err_guided      = 0;
        float float_err_blind       = 0;
        size_t uint_im              = 0;
        size_t uint_im_total        = 0;
        size_t uint_mc_trials       = 0;
        std::vector <std::vector <size_t>> vec_resp,vec_resp_sorted;

        std::cout << std::endl;

        while (uint_errors_guided < uint_num_mc)
        {

            uint_mc_trials++;
            memory.reset();

            // generate the random messages with random orders.
			std::vector <std::vector <size_t>> vec_messages (uint_num_messages, std::vector <size_t> (0));

            for (size_t uint_i = 0; uint_i < uint_num_messages; uint_i++)
            {
                uint_num_clusters = cmin + randint(cmax - cmin + 1) - 1;

                for (size_t uint_j = 0; uint_j < uint_num_clusters; uint_j++)
                    vec_messages[uint_i].push_back(randint(nf));
            }

            // learn the uniformly random messages
            vec_clusters = memory.learn(vec_messages);


            // This part generates the partial messages where some of the sub-messages are removed.
            // The number of unknown sub-messages is given by 'uint_num_unknowns'.
            vec_partial_messages = std::vector <std::vector <size_t>> (uint_num_messages, std::vector <size_t> (0));
            vec_partial_clusters = std::vector <std::vector <size_t>> (uint_num_messages, std::vector <size_t> (0));


            size_t uint_num_remainders;
            size_t uint_remainder_counter;

            for (size_t uint_i = 0; uint_i < uint_num_messages; uint_i++)
            {
                uint_num_clusters = vec_messages[uint_i].size(); // get number of clusters in each message
                uint_num_remainders = uint_num_clusters - uint_num_unknowns;
                while (uint_remainder_counter < uint_num_remainders)
                {
                    uint_randint = randint(uint_num_clusters) - 1;
                    if (!exist(vec_partial_clusters[uint_i],vec_clusters[uint_i][uint_randint]))
                    {
                        vec_partial_messages[uint_i].push_back(vec_messages[uint_i][uint_randint]);
                        vec_partial_clusters[uint_i].push_back(vec_clusters[uint_i][uint_randint]);
                        uint_remainder_counter++;
                    }
                }

                uint_remainder_counter = 0;
            }


            uint_im = 0;
            while (uint_errors_guided < uint_num_mc && uint_im < uint_num_messages)
            {

                vec_resp = memory.recall_guided(vec_partial_messages[uint_im],vec_partial_clusters[uint_im],vec_clusters[uint_im],uint_num_it);
                vec_resp_sorted = sort_clusters(vec_resp,vec_clusters[uint_im]);
                if (vec_resp_sorted[0] != vec_messages[uint_im]) uint_errors_guided++;

                vec_resp = memory.recall_blind(vec_partial_messages[uint_im],vec_partial_clusters[uint_im]);
                vec_resp_sorted = sort_clusters(vec_resp,vec_clusters[uint_im]);
                if (vec_resp_sorted[0] != vec_messages[uint_im]) uint_errors_blind++;

                uint_im++;
                uint_im_total++;

                // This part computes the error rate and send them to the output stream.
                float_err_guided    = (float)uint_errors_guided/uint_im_total;
                float_err_blind     = (float)uint_errors_blind/uint_im_total;
                std::cout << "\r num_trials : " << uint_mc_trials;
                std::cout << " num_msgs : " << uint_num_messages;
                std::cout << " pe_guided : " << float_err_guided;
                std::cout << " pe_blind : " << float_err_blind;
            }

            if (uint_mc_trials > 10 && uint_errors_blind < 1.0e-5) break;
        }


        // This part writes the error rates (blind and guided retrievals) in the file.
        fs_results << uint_num_messages;
        fs_results << " " <<  float_err_guided;
        fs_results << " " <<  float_err_blind << std::endl;
    }


    fs_results.close();  
    std::cout << std::endl;

	return EXIT_SUCCESS;
}
