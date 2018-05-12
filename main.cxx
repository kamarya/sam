/**
 * @mainpage Sparse Associative Memory (SAM)
 * @author Behrooz Kamary Aliabadi
 * @date 19 Sep 2011
 *
 * Sparse Associative Memory (SAM) is an associative memory
 * resembling the human memory. For more details read the
 * references below.
 *
 * This code reproduces figure 3 in article
 * "Storing Sparse Messages in Networks of Neural Cliques".
 * It is to demonstrate how the neural network which
 * has been presented in the article work.
 *
 * @see http://ieeexplore.ieee.org/document/6658945
 * @see https://tel.archives-ouvertes.fr/tel-00962603/document
 * @see https://cordis.europa.eu/project/rcn/102141_en.html
 */

#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <sys/resource.h>

#include <stdexcept>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <ctime>
#include <cstring>

#include "sam.hpp"

#define CWIDTH          15
#define USAGE_STDERR    std::cerr << std::left << std::setw(CWIDTH)

// network parameters

size_t nc               = 100; // The total number of clusters in the network
size_t nf               = 64;   // The number of fanals in each cluster
size_t cmax             = 12; // The maximum message order
size_t cmin             = 12; // The minimum message order

// uniformly random generated messages' parameters

size_t min_num          = 0.5e5; // The minimum number of learnd messages
size_t max_num          = 4.5e5; // The maximum number of learnd messages
size_t num_steps        = 30;  // The number of simulation steps
size_t num_unknowns     = 3;

// algorithmic parameters

size_t num_it           = 4;   // number of iterations
size_t num_mc           = 500; // The observed number of errors

const char* filename    = nullptr;
int         prio        = 0;

int  run(void);
int  setprio(int);
void usage(const char* progname);

int main(int argc, char **argv)
{
    static struct option long_options[] =
        {
            {"nmin", required_argument, 0, 'm'},
            {"nmax", required_argument, 0, 'x'},
            {"nit", required_argument, 0, 'i'},
            {"nf", required_argument, 0, 'f'},
            {"nc", required_argument, 0, 'c'},
            {"ne", required_argument, 0, 'e'},
            {"nmc", required_argument, 0, 'o'},
            {"csv", required_argument, 0, 'r'},
            {"prio", required_argument, 0, 'p'},
            {"help", no_argument, 0, 'h'},
            {0, 0, 0, 0},
        };

    const char *const short_opts = "hm:x:i:f:c:e:o:r:p:";

    while (true)
    {
        const auto opt = getopt_long(argc, argv, short_opts, long_options, nullptr);

        if (-1 == opt)
            break;

        switch (opt)
        {
        case 'm':
            try { min_num = std::stoi(optarg);} catch (...) {/*don't care*/}
            break;
        case 'x':
            try { max_num = std::stoi(optarg);} catch (...) {/*don't care*/}
            break;
        case 'o':
            try { num_mc = std::stoi(optarg);} catch (...) {/*don't care*/}
            break;
        case 'c':
            try { nc     = std::stoi(optarg);} catch (...) {/*don't care*/}
            break;
        case 'f':
            try { nf     = std::stoi(optarg);} catch (...) {/*don't care*/}
            break;
        case 'r':
            filename     = optarg;
            break;
        case 'p':
            prio         = std::stoi(optarg);
            break;
        case 'h': // -h or --help
            usage(argv[0]);
            return EXIT_SUCCESS;
        case '?': // unrecognized option
        default:
            usage(argv[0]);
            return EXIT_FAILURE;
        }
    }


    if (filename == nullptr)
    {
        usage(argv[0]);

        std::cerr << std::endl << "error: you must specify the results filename." << std::endl;

        return EXIT_FAILURE;
    }

    if (max_num < min_num)
    {
        usage(argv[0]);

        std::cerr << std::endl << "error: nmax < nmin is invalid." << std::endl;

        return EXIT_FAILURE;
    }

    if (prio != 0)
    {
        if (setprio(prio) != 0)
        {
            std::cerr << "error: failed to set the process priority." << std::endl;
            std::cerr << std::strerror(errno) << std::endl;
            return EXIT_FAILURE;
        }
    }

    return run();
}

void usage(const char* progname)
{
    std::cerr << "Usage : " << progname << "  [options]" << std::endl;
    USAGE_STDERR << "-h | --help " << "this help message." << std::endl;
    USAGE_STDERR << "-m | --nmin " << "minimum number of stored messages." << std::endl;
    USAGE_STDERR << "-x | --nmax " << "maximum number of stored messages." << std::endl;
    USAGE_STDERR << "-o | --nmc "  << "Monte-Carlo error count." << std::endl;
    USAGE_STDERR << "-c | --nc "   << "total number of clusters." << std::endl;
    USAGE_STDERR << "-f | --nf "   << "number of fanals in each cluster." << std::endl;
    USAGE_STDERR << "-p | --prio " << "set process priority (-20 is the highest and 0 is the lowest)." << std::endl;
    USAGE_STDERR << "-r | --csv "  << "the results' file name in CSV format." << std::endl;
}

int setprio(int prio)
{
    id_t pid = getpid();
    int ret = setpriority(PRIO_PROCESS, pid, prio);

    if (ret == 0 && getuid() == 0)
    {
        const char *realuid = getenv("SUDO_UID");
        const char *realgid = getenv("SUDO_GID");

        if (realgid != nullptr && realuid != nullptr)
        {

            try
            {
                ret = std::stoi(realgid);
            }
            catch (...)
            {
                return -1;
            }

            ret = setgid(ret);

            if (ret != 0)
                return -1;

            try
            {
                ret = std::stoi(realuid);
            }
            catch (...)
            {
                return -1;
            }

            ret = setuid(ret);
        }
    }

    return ret;
}

int run(void)
{
    std::srand(std::time(nullptr));

    size_t num_step = (max_num - min_num) / num_steps;

    sam memory(nc, nf);

    std::ofstream fs_results;
    fs_results.open(filename, std::ios::out);

    if (!fs_results)
    {
        std::cerr << "failed to open the results file." << std::endl;
        return EXIT_FAILURE;
    }

    fs_results << "ntrials,nmsgs,peg,peb" << std::endl;
    std::cout << std::setw(CWIDTH) << "ntrials" << std::setw(CWIDTH) << "nmsgs";
    std::cout << std::setw(CWIDTH) << "peg" << std::setw(CWIDTH) << "peb" << std::endl;

    size_t num_clusters;
    size_t rnd_index;

    std::vector<std::vector<size_t>> vec_clusters;
    std::vector<std::vector<size_t>> vec_partial_messages;
    std::vector<std::vector<size_t>> vec_partial_clusters;

    for (size_t step = 0; step < num_steps + 1; step++)
    {

        size_t num_messages = max_num - num_step * step;

        size_t errors_guided           = 0;
        size_t errors_blind            = 0;
        float  float_err_guided        = 0;
        float  float_err_blind         = 0;
        size_t mindx                   = 0;
        size_t mtotal                  = 0;
        size_t mc_trials               = 0;
        std::vector<std::vector<size_t>> vec_resp, vec_resp_sorted;

        std::cout << std::endl;

        while (errors_guided < num_mc)
        {
            mindx = 0;
            mc_trials++;
            memory.reset();

            // generate the random messages with random orders.
            std::vector<std::vector<size_t>> vec_messages(num_messages, std::vector<size_t>(0));

            for (size_t indx = 0; indx < num_messages; indx++)
            {
                num_clusters = cmin + randint(cmax - cmin + 1) - 1;
                vec_messages[indx].reserve(num_clusters);
                for (size_t jndx = 0; jndx < num_clusters; jndx++)
                {
                    vec_messages[indx].push_back(randint(nf));
                }
            }

            // learn the uniformly random messages
            vec_clusters = memory.learn(vec_messages);

            // This part generates the partial messages where some of the sub-messages are removed.
            // The number of unknown sub-messages is given by 'num_unknowns'.
            vec_partial_messages = std::vector<std::vector<size_t>>(num_messages, std::vector<size_t>(0));
            vec_partial_clusters = std::vector<std::vector<size_t>>(num_messages, std::vector<size_t>(0));

            size_t num_remainders;
            size_t remainder_counter;

            for (size_t indx = 0; indx < num_messages; indx++)
            {
                num_clusters    = vec_messages[indx].size(); // get number of clusters in each message
                num_remainders  = num_clusters - num_unknowns;
                while (remainder_counter < num_remainders)
                {
                    rnd_index = randint(num_clusters) - 1;
                    if (!exist(vec_partial_clusters[indx], vec_clusters[indx][rnd_index]))
                    {
                        vec_partial_messages[indx].push_back(vec_messages[indx][rnd_index]);
                        vec_partial_clusters[indx].push_back(vec_clusters[indx][rnd_index]);
                        remainder_counter++;
                    }
                }

                remainder_counter = 0;
            }

            while (errors_guided < num_mc && mindx < num_messages)
            {

                vec_resp = memory.recall_guided(vec_partial_messages[mindx], vec_partial_clusters[mindx], vec_clusters[mindx], num_it);
                vec_resp_sorted = sort_clusters(vec_resp, vec_clusters[mindx]);
                if (vec_resp_sorted[0] != vec_messages[mindx]) errors_guided++;

                vec_resp = memory.recall_blind(vec_partial_messages[mindx], vec_partial_clusters[mindx]);
                vec_resp_sorted = sort_clusters(vec_resp, vec_clusters[mindx]);
                if (vec_resp_sorted[0] != vec_messages[mindx]) errors_blind++;

                mindx++;
                mtotal++;

                // compute the error rate and send them to the output stream.
                float_err_guided    = (float)errors_guided / mtotal;
                float_err_blind     = (float)errors_blind / mtotal;
            }

            if (mc_trials > 10 && errors_blind < 1.0e-5) break;
        }

        std::cout << std::setprecision(5)
                  << std::setw(CWIDTH) << mc_trials
                  << std::setw(CWIDTH) << num_messages
                  << std::setw(CWIDTH) << float_err_guided
                  << std::setw(CWIDTH) << float_err_blind;

        // writes the error rates in the file.
        fs_results  << mc_trials << ","
                    << num_messages << ","
                    << float_err_guided << ","
                    << float_err_blind << std::endl;
    }

    fs_results.close();
    std::cout << std::endl;

    return EXIT_SUCCESS;
}
