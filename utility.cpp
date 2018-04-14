#include "utility.hpp"

size_t randint(size_t uint_max)
{
    return ((size_t)std::rand() % uint_max + 1);
}

std::vector<size_t> max_indices(std::vector<size_t> vec_arg)
{

    size_t uint_vec_size = vec_arg.size();
    if (uint_vec_size == 0)
        return std::vector<size_t>(0);
    size_t uint_max_value = vec_arg[0];
    std::vector<size_t> vec_indices(0);

    // find the maximum
    for (size_t uint_indx = 1; uint_indx < uint_vec_size; uint_indx++)
    {
        if (vec_arg[uint_indx] > uint_max_value)
            uint_max_value = vec_arg[uint_indx];
    }

    // find the indices
    for (size_t uint_indx = 0; uint_indx < uint_vec_size; uint_indx++)
    {
        if (vec_arg[uint_indx] == uint_max_value)
            vec_indices.push_back(uint_indx);
    }

    return vec_indices;
}

size_t max(std::vector<size_t> vec_arg)
{
    size_t uint_max_value = vec_arg[0];
    size_t uint_max_indx = 0;
    size_t uint_vec_size = vec_arg.size();

    for (size_t uint_indx = 1; uint_indx < uint_vec_size; uint_indx++)
    {
        if (vec_arg[uint_indx] > uint_max_value)
        {
            uint_max_value = vec_arg[uint_indx];
            uint_max_indx = uint_indx;
        }
    }

    return uint_max_value;
}

bool exist(std::vector<size_t> vec_arg, size_t uint_arg)
{
    size_t uint_size = vec_arg.size();

    for (size_t uint_indx = 0; uint_indx < uint_size; uint_indx++)
    {
        if (vec_arg[uint_indx] == uint_arg)
            return true;
    }

    return false;
}

int find_index(std::vector<size_t> vec_arg, size_t uint_arg)
{
    size_t uint_size = vec_arg.size();

    for (size_t uint_indx = 0; uint_indx < uint_size; uint_indx++)
    {
        if (vec_arg[uint_indx] == uint_arg)
            return uint_indx;
    }

    return (-1);
}

std::vector<std::vector<size_t>> sort_clusters(std::vector<std::vector<size_t>> vec_message, std::vector<size_t> vec_clusters)
{

    if (vec_message.size() == 0)
        return std::vector<std::vector<size_t>>(2, std::vector<size_t>(vec_clusters.size(), 0));

    if (vec_message[0].size() == 0 || vec_message[1].size() == 0)
        return std::vector<std::vector<size_t>>(2, std::vector<size_t>(vec_clusters.size(), 0));

    if (vec_message[0].size() != vec_clusters.size())
        return std::vector<std::vector<size_t>>(2, std::vector<size_t>(vec_clusters.size(), 0));

    std::vector<std::vector<size_t>> vec_return(2, std::vector<size_t>(vec_message[0].size(), 0));
    int int_index = 0;

    for (size_t uint_index = 0; uint_index < vec_clusters.size(); uint_index++)
    {
        int_index = find_index(vec_message[1], vec_clusters[uint_index]);
        if (int_index == -1)
        {
            vec_return[0][uint_index] = 0;
            vec_return[1][uint_index] = SIZE_MAX - 1; // This is to indicate that a cluster error occurred.
        }
        else
        {
            vec_return[0][uint_index] = vec_message[0][int_index];
            vec_return[1][uint_index] = vec_clusters[uint_index];
        }
    }

    return vec_return;
}
