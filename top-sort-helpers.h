#pragma once

#include <unordered_map>

#include "weighted-graph.hpp"
#include "graph-types.h"

template <typename T>
void computeIndegrees(const WeightedGraph<T>& graph, std::unordered_map<value_type<T>, int>& indegrees) {
    // TODO store the indegree for each vertex in the graph in the indegrees map
    //default indegree of every vertex set to 0
    for(auto it = graph.begin(); it != graph.end(); it++){
        value_type<T> vertex = it->first;
        indegrees[vertex] = 0;
    }

    // Iterate over all vertices and their adjacency lists
    // For each edge (vertex -> adj_vertex), increment indegree of adj_vertex
    for(auto it2 = graph.begin(); it2 != graph.end(); it2++){
        const adjacency_list<T>& adj = it2->second;

        for(auto edge = adj.begin(); edge != adj.end(); edge++){
            value_type<T> adj_vertex = edge->first;
            indegrees[adj_vertex]++;    //increment degree of ending vertex
        }
    }
}