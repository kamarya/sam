/**
 * @file sam.cpp
 * @author Behrooz Kamary Aliabadi
 * @date 19 Sep 2011
 * @brief Sparse Associative Memory (SAM)
 *
 * Sparse Associative Memory (SAM) is an associative memory
 * resembling the human memory. For more details read the
 * references below.
 *
 * @see http://ieeexplore.ieee.org/document/6658945/
 * @see https://tel.archives-ouvertes.fr/tel-00962603/document
 * @see https://cordis.europa.eu/project/rcn/102141_en.html 
 */

#include "sam.hpp"

sam::sam(size_t nc, size_t nf)
{
	nclusters = nc;
	nfanals   = nf;

    vec_weights = std::vector < std::vector < std::vector < std::vector <char> > > > (nclusters, 
                    std::vector < std::vector < std::vector <char> > > (nclusters,
                    std::vector < std::vector <char> > (nfanals,
                    std::vector <char> (nfanals,0))));

    ncores = std::thread::hardware_concurrency();
}

sam::~sam()
{
}

void sam::reset()
{
    for (size_t uint_cluster_i = 0; uint_cluster_i < nclusters; uint_cluster_i++)
    {
        for (size_t uint_cluster_j = 0; uint_cluster_j < nclusters; uint_cluster_j++)
        {
            for (size_t uint_fanal_i = 0; uint_fanal_i < nfanals; uint_fanal_i++)
            {
                for (size_t uint_fanal_j = 0; uint_fanal_j < nfanals; uint_fanal_j++)
                {
                    vec_weights[uint_cluster_i][uint_cluster_j][uint_fanal_i][uint_fanal_j] = 0;
                }
            }
        }
    }
}

// This routine learns the two dimensional set of
// messages given by 'vec_message'
std::vector<std::vector<size_t>> sam::learn(const std::vector<std::vector<size_t>>& vec_message)
{
    size_t uint_num_messages                = vec_message.size();
    size_t uint_cumulative_message_length   = 0;
    size_t uint_num_msg_clusters            = 0;
    size_t uint_random_cluster_counter      = 0;
    size_t uint_randint                     = 0;

    // This part choose random cluster to learn the messages.
    // In the manuscript it is assumed that the exploited clusters
    // for each clique are chosen uniformly random.
    std::vector<std::vector<size_t>> vec_random_clusters(uint_num_messages, std::vector<size_t>(0));
    for (size_t uint_msg_indx = 0; uint_msg_indx < uint_num_messages; uint_msg_indx++)
    {
        uint_num_msg_clusters = vec_message[uint_msg_indx].size();
        while (uint_random_cluster_counter < uint_num_msg_clusters)
        {
            uint_randint = randint(nclusters) - 1;
            if (!exist(vec_random_clusters[uint_msg_indx], uint_randint))
            {
                vec_random_clusters[uint_msg_indx].push_back(uint_randint);
                uint_random_cluster_counter++;
            }
        }
        uint_random_cluster_counter = 0;
    }

    // This part learns the input messages in 'vec_message' in cliques
    // by construing the connections in the way that is elaborated in
    // the manuscript.
    for (size_t uint_msg_indx = 0; uint_msg_indx < uint_num_messages; uint_msg_indx++)
    {
        uint_num_msg_clusters = vec_message[uint_msg_indx].size();
        uint_cumulative_message_length += uint_num_msg_clusters;
        for (size_t uint_cluster = 0; uint_cluster < uint_num_msg_clusters; uint_cluster++)
        {
            for (size_t uint_cluster_ = 0; uint_cluster_ < uint_num_msg_clusters; uint_cluster_++)
            {
                if (uint_cluster != uint_cluster_)
                    vec_weights
                        [vec_random_clusters[uint_msg_indx][uint_cluster]]
                        [vec_random_clusters[uint_msg_indx][uint_cluster_]]
                        [vec_message[uint_msg_indx][uint_cluster] - 1]
                        [vec_message[uint_msg_indx][uint_cluster_] - 1] = 1;
            }
        }
    }

    return vec_random_clusters;
}

// This routine performs the blind recovery. The input parameters are the known sub-messages
// given in 'vec_message' and their corresponding clusters given in 'vec_clusters'.
// The default number of iterations in this recovery mode is set to one since it does not help
// the error rate performance.
std::vector<std::vector<size_t>> sam::recall_blind(const std::vector<size_t>& vec_message, const std::vector<size_t>& vec_clusters)
{


    size_t uint_num_known_clusters = vec_message.size();

    // The decoder data containers have been defined and initialized here.

    // This two dimensional std::vector holds the computed scores of fanals in each iteration.
    std::vector<std::vector<size_t>> vec_network(nclusters, std::vector<size_t>(nfanals));
    // This two dimensional std::vector keep the list of active fanals in each cluster.
    std::vector<std::vector<size_t>> vec_network_list(nclusters, std::vector<size_t>(0));
    // This std::vector holds the list of clusters that have at least one active fanal.
    std::vector<size_t> vec_clusters_lag = vec_clusters;

    for (size_t uint_cluster = 0; uint_cluster < uint_num_known_clusters; uint_cluster++)
    {
        vec_network_list[vec_clusters[uint_cluster]].push_back(vec_message[uint_cluster]);
        vec_network[vec_clusters[uint_cluster]][vec_message[uint_cluster] - 1] = 1;
    }

    // This part computes the overall scores of all fanals that are connected to the
    // active fanals (for the first iteration step they correspond to the partial message)
    std::vector<std::thread> workers(nclusters);

    for (size_t uint_cluster = 0; uint_cluster < nclusters; uint_cluster++)
    {
        workers[uint_cluster] = std::thread([&, this, uint_cluster]() {

            for (size_t uint_fanal = 0; uint_fanal < nfanals; uint_fanal++)
            {
                for (std::vector<size_t>::iterator itc = vec_clusters_lag.begin(); itc != vec_clusters_lag.end(); itc++)
                {
                    for (std::vector<size_t>::iterator itf = vec_network_list[*itc].begin(); itf != vec_network_list[*itc].end(); itf++)
                    {

                        if (this->vec_weights[uint_cluster][*itc][uint_fanal][*itf - 1] > 0)
                        {
                            vec_network[uint_cluster][uint_fanal]++;
                            // 'break' is to assure a fanal receives only one signal unit from a cluster
                            // (that may have more than one active fanal)
                            break;
                        }
                    }
                }
            }
        });
    }

    std::for_each(workers.begin(), workers.end(), std::mem_fn(&std::thread::join));

    // This part performs a global winner-take-all.

    vec_network_list = std::vector<std::vector<size_t>>(nclusters, std::vector<size_t>(0));
    vec_clusters_lag = std::vector<size_t>(nclusters);

    // obtains the maximum activity level in each cluster
    for (size_t uint_cluster = 0; uint_cluster < nclusters; uint_cluster++)
    {
        vec_clusters_lag[uint_cluster] = max(vec_network[uint_cluster]);
    }

    vec_clusters_lag        = max_indices(vec_clusters_lag);
    size_t max_value_fanal  = max(vec_network[vec_clusters_lag[0]]);

    for (size_t uint_cluster = 0; uint_cluster < nclusters; uint_cluster++)
    {
        for (size_t uint_indx = 0; uint_indx < nfanals; uint_indx++)
        {
            // find fanals that have a score equal to the maximum score.
            if (vec_network[uint_cluster][uint_indx] == max_value_fanal && exist(vec_clusters_lag, uint_cluster))
            {
                vec_network[uint_cluster][uint_indx] = 1;
                vec_network_list[uint_cluster].push_back(uint_indx + 1);
            }
            else
                vec_network[uint_cluster][uint_indx] = 0;
        }
    }

    // message retrieval

    std::vector<std::vector<size_t>> vec_retrieved(2, std::vector<size_t>(vec_clusters_lag.size()));

    size_t uint_amb_counter         = 0;
    size_t uint_cluster_counter     = 0;

    for (std::vector<size_t>::iterator itc = vec_clusters_lag.begin(); itc != vec_clusters_lag.end(); itc++)
    {

        vec_retrieved[1][uint_cluster_counter] = *itc;

        for (size_t uint_indx = 0; uint_indx < nfanals; uint_indx++)
        {
            if (vec_network[*itc][uint_indx] == 1)
            {
                vec_retrieved[0][uint_cluster_counter] = uint_indx + 1;
                uint_amb_counter++;
            }
        }

        // Fanal ambiguity detection:
        // This part checks whether there is more than one active fanal in a cluster.
        // In that case it returns an empty vector (see the references for more info.).
        if (uint_amb_counter > 1)
        {
            return (std::vector<std::vector<size_t>>(2, std::vector<size_t>(0)));
        }

        uint_amb_counter = 0;
        uint_cluster_counter++;
    }

    // It returns a two dimensional matrix
    // Row 0 holds the sub-messages
    // Row 1 holds the corresponding clusters
    return vec_retrieved;
}

std::vector<std::vector<size_t>> sam::recall_guided(const std::vector<size_t>& vec_message,
                                                    const std::vector<size_t>& vec_clusters,
                                                    const std::vector<size_t>& vec_clusters_all,
                                                    size_t uint_max_it)
{

    size_t uint_num_known_clusters = vec_message.size();
    size_t nall = vec_clusters_all.size();

    // classical decoder data containers
    std::vector<std::vector<size_t>>    vec_network(nclusters, std::vector<size_t>(nfanals));
    std::vector<std::vector<size_t>>    vec_network_list(nclusters, std::vector<size_t>(0));
    std::vector<size_t>                 vec_clusters_lag = vec_clusters;

    for (size_t uint_cluster = 0; uint_cluster < uint_num_known_clusters; uint_cluster++)
    {
        vec_network_list[vec_clusters[uint_cluster]].push_back(vec_message[uint_cluster]);
        vec_network[vec_clusters[uint_cluster]][vec_message[uint_cluster] - 1] = 1;
    }

    for (size_t uint_it = 0; uint_it < uint_max_it; uint_it++)
    {
        std::vector<std::thread> workers(nall);

        for (size_t uint_cluster = 0; uint_cluster < nall; uint_cluster++)
        {
            workers[uint_cluster] = std::thread([&, this, uint_cluster]() {

                for (size_t uint_fanal = 0; uint_fanal < nfanals; uint_fanal++)
                {
                    for (std::vector<size_t>::iterator itc = vec_clusters_lag.begin(); itc != vec_clusters_lag.end(); itc++)
                    {
                        for (std::vector<size_t>::iterator itf = vec_network_list[*itc].begin(); itf != vec_network_list[*itc].end(); itf++)
                        {
                            if (this->vec_weights[vec_clusters_all[uint_cluster]][*itc][uint_fanal][*itf - 1] > 0)
                            {
                                vec_network[vec_clusters_all[uint_cluster]][uint_fanal]++;
                                break;
                            }
                        }
                    }
                }
            });
        }

        std::for_each(workers.begin(), workers.end(), std::mem_fn(&std::thread::join));

        // Winner-take-all

        vec_network_list = std::vector<std::vector<size_t>>(nclusters, std::vector<size_t>(0));
        vec_clusters_lag = std::vector<size_t>(nclusters, 0);
        size_t uint_max_value_fanal;

        // obtains the maximum activity level in each cluster
        for (size_t uint_cluster = 0; uint_cluster < nall; uint_cluster++)
        {
            vec_clusters_lag[vec_clusters_all[uint_cluster]] = max(vec_network[vec_clusters_all[uint_cluster]]);
        }

        vec_clusters_lag        = max_indices(vec_clusters_lag);
        uint_max_value_fanal    = max(vec_network[vec_clusters_lag[0]]);

        for (size_t uint_cluster = 0; uint_cluster < nall; uint_cluster++)
        {
            if (uint_max_value_fanal > 0)
            {
                for (size_t uint_indx = 0; uint_indx < nfanals; uint_indx++)
                {
                    // find fanals that have a score equal to the maximum score.
                    if (vec_network[vec_clusters_all[uint_cluster]][uint_indx] == uint_max_value_fanal)
                    {
                        vec_network[vec_clusters_all[uint_cluster]][uint_indx] = 1;
                        vec_network_list[vec_clusters_all[uint_cluster]].push_back(uint_indx + 1);
                    }
                    else
                        vec_network[vec_clusters_all[uint_cluster]][uint_indx] = 0;
                }
            }
        }

    } // end of iteration

    // message retrieval

    std::vector<std::vector<size_t>> vec_retrieved(2, std::vector<size_t>(nall));

    size_t uint_amb_counter     = 0;
    size_t uint_cluster_counter = 0;

    for (size_t uint_cluster = 0; uint_cluster < nall; uint_cluster++)
    {

        vec_retrieved[1][uint_cluster_counter] = vec_clusters_all[uint_cluster];

        for (size_t uint_indx = 0; uint_indx < nfanals; uint_indx++)
        {
            if (vec_network[vec_clusters_all[uint_cluster]][uint_indx] == 1)
            {
                vec_retrieved[0][uint_cluster_counter] = uint_indx + 1;
                uint_amb_counter++;
            }
        }

        // fanal ambiguity detection
        if (uint_amb_counter > 1)
            return (std::vector<std::vector<size_t>>(2, std::vector<size_t>(0)));

        uint_amb_counter = 0;
        uint_cluster_counter++;
    }

    // It returns a two dimensional std::vector
    // row 0 holds the sub-messages
    // row 1 holds the corresponding clusters
    return vec_retrieved;
}