/**
 * @file sam.hpp
 * @author Behrooz Kamary Aliabadi
 * @date 19 Sep 2011
 * @brief Sparse Associative Memory (SAM)
 *
 * Sparse Associative Memory (SAM) is an associative memory
 * resembling with the aim of resembling the human memory.
 * For more details read the references below.
 *
 * @see http://ieeexplore.ieee.org/document/6658945/
 * @see https://tel.archives-ouvertes.fr/tel-00962603/document
 * @see https://cordis.europa.eu/project/rcn/102141_en.html 
 */

#ifndef __SAM_HPP__
#define __SAM_HPP__

#include <iostream>
#include <fstream>
#include <iomanip>
#include <ctime>
#include <thread>
#include <algorithm>

#include "utility.hpp"

/**
 * @class sam
 *
 * @brief Sparse Associative Memory (SAM) abstraction class
 *
 * The class implements the Sparse Associative Memory (SAM)
 * described in the given references.
 *
 */
class sam
{

  public:
   /**
    * @brief constructor
    * @param nc the total number of clusters in the network.
    * @param nf the total number of fanals in each cluster.
    *
    * The number of none zero elements in each message is limited
    * by the total number of clusters. An element of a message
    * is a number (index of the element in an alphabet) limited
    * by the total number of fanals.
    */
    sam(size_t nc, size_t nf);

    //! destructor
    ~sam();

    /**
     * @brief learn a single message i.e. word.
     * @param vec_message the vector of message elements.
     * @return the vector of message elements along with their corresponding cluster indices
     *
     * A message element is a number between zero and the number of fanals in each cluster.
     */
    std::vector<std::vector<size_t>> learn(std::vector<std::vector<size_t>> vec_message);

    /**
     * @brief recall the entire message given a few of its elements (a partially known message)
     *
     * This method implements an algorithm, namely, blind recall since the network knows neither
     * the entire message nor their corresponding clusters.
     */
    std::vector<std::vector<size_t>> recall_blind(const std::vector<size_t>& vec_message, const std::vector<size_t>& vec_clusters);

    /**
     * @brief recall the entire message given a few of its elements (a partially known message)
     */
    std::vector<std::vector<size_t>> recall_guided(const std::vector<size_t>& vec_message,
                                                   const std::vector<size_t>& vec_clusters,
                                                   const std::vector<size_t>& vec_clusters_all,
                                                   size_t uint_max_it);

    /**
     * @brief reset the associative memory to the initial state (erase learned messages).
     */
    void reset();

  private:
    std::vector<std::vector<std::vector<std::vector<char>>>> vec_weights;

    size_t nclusters; // The total number of clusters in the network
    size_t nfanals;   // The number of fanals in each cluster
};

#endif
