#pragma once

#include <istream>
#include <ostream>
#include <sstream>
#include <limits>
#include <list>
#include <queue>
#include <string>
#include <unordered_set>
#include <unordered_map>

#include "weighted-graph.hpp"

#include "graph-types.h"

// if the arrow is a box, change to the other line
#define ARROW_SEPARATOR " \u2192 "
// #define ARROW_SEPARATOR " -> "

#include "top-sort-helpers.h"

/**
 * @brief Returns a Topological Ordering of the Graph - https://en.wikipedia.org/wiki/Topological_sorting#Depth-first_search
 *
 * @tparam T type of data stored by a vertex
 * @param graph graph upon which to perform a topological ordering
 * @return std::list<value_type<T>> list of nodes in a topological order, or an empty list if no such ordering exists
 */
template <typename T>
std::list<value_type<T>> topologicalSort(const WeightedGraph<T>& graph)
{
    std::unordered_map<value_type<T>, int> indegrees;
    std::unordered_map<value_type<T>, int> topological_numbers;

    // TODO implement Topological Sort
    //compute the indegree of each vertex
    //indegree = number of edges pointing to the vertex
    std::list<value_type<T>> ordering;
    computeIndegrees<T>(graph, indegrees);

    //initialize a queue with all vertices with indegree of 0
    //these vertices have no dependencies and can appear first in the ordering
    std::queue<value_type<T>> zero_indegree;
    for(const auto& [vertex, degree] : indegrees){
        if(degree == 0){
            zero_indegree.push(vertex);
        }
    }

    //process vertices in the queue
    //main loop of Kahn's algorithm for topological sorting
    while(!zero_indegree.empty()){
        value_type<T> u = zero_indegree.front();
        zero_indegree.pop();
        //add current vertex to ordering
        ordering.push_back(u);

        //decrease indegree of all adjacent vertices
        for(const_edge_iterator<T> edge = graph.begin(u); edge != graph.end(u); edge++){
            value_type<T> v = edge->first;
            indegrees[v]--;
            //if v now has 0 indegree, it can be added to the queue
            if(indegrees[v] == 0){
                zero_indegree.push(v);
            }
        }
    }

    //check for cycles
    //if the ordering doesn't contain all vertices, the graph has at least one cycle
    //in that case, a topological ordering is not possible, so return empty list
    if(ordering.size() != graph.size()){
        return std::list<value_type<T>>();
    }

    return ordering;
}

// contains initializeSingleSource, relax, & updateHeap as well as the DijkstraComaparator
#include "dijkstras-helpers.h"

/**
 * @brief Dijkstra's Algorithm - https://canvas.tamu.edu/courses/136654/files/35930572/preview Slide 16
 *
 * @tparam T type of data stored by a vertex
 * @param graph weighted, directed graph to find single-source shortest-path
 * @param initial_node source node in graph for shortest path
 * @param destination_node destination node in graph for shortest path
 * @return std::list<value_type<T>> list of nodes along shortest path including initial_node and destination_node, empty if no path exists
 */
template <typename T>
std::list<value_type<T>> dijkstrasAlgorithm(const WeightedGraph<T>& graph, vertex_type<T> initial_node, vertex_type<T> destination_node)
{
    std::unordered_map<value_type<T>, weight_type<T>> distances;
    std::unordered_map<value_type<T>, std::optional<value_type<T>>> predecessors;
    std::unordered_set<value_type<T>> s;
    std::priority_queue<value_type<T>, std::vector<value_type<T>>, DijkstraComparator<T>> q(DijkstraComparator<T>{distances});

    // TODO implement Dijkstra's Algorithm
    //initializes distance and predecessors
    initializeSingleSource(graph, initial_node, distances, predecessors);

    //add all vertices to priority queue
    //ordered by current known shortest distance
    for(auto it = graph.begin(); it != graph.end(); ++it){
        q.push(it->first);
    }

    //main loop of Dijsktra's algorithm
    while(!q.empty()){
        //extract vertex that has smallest (tentative) distance
        value_type<T> u = q.top();
        q.pop();

        //remaining vertices aren't reachable if smallest distance is infinity
        if(distances[u] == infinity<T>()){
            break;
        }

        //add u to finalized set
        s.insert(u);

        //relax all outgoing edges
        for(const auto& [v, w] : graph.at(u)){
            //if v is already in cloud
            if(s.find(v) != s.end()){
                continue; 
            }

            if(relax<T>(u, v, w, distances, predecessors)){
                updateHeap(q, distances); //update priority queue if distance was updated
            }
        }
    }

    // TODO create list by walking backwards through predecessors from the end
    std::list<value_type<T>> path;
    value_type<T> current = destination_node;

    //while current node has a predecessor
    while(predecessors[current].has_value()){
        path.push_front(current);
        current = predecessors[current].value();
    }
    
    if(path.empty() && initial_node != destination_node){
        return std::list<value_type<T>>(); //return an empty list, bc no path exists
    }

    //add source node to the front
    path.push_front(initial_node);
    
    return path;
}

template <typename T>
std::ostream &operator<<(std::ostream &o, const WeightedGraph<T> &graph)
{
    for (auto it = graph.begin(); it != graph.end(); ++it)
    {
        const vertex_type<T> &vertex = it->first;
        const adjacency_list<T> &list = it->second;
        o << vertex << ": ";
        for (auto iit = list.begin(); iit != list.end(); ++iit)
        {
            const vertex_type<T> &destination = iit->first;
            const weight_type<T> &weight = iit->second;
            o << destination << "(" << weight << ')';
            auto next_iter = iit;
            if (++next_iter != list.end())
            {
                o << ARROW_SEPARATOR;
            }
        }
        auto next_iter = it;
        if (++next_iter != graph.end())
        {
            o << "\n";
        }
    }
    return o;
}

template <typename T>
std::istream &readEdge(std::istream &i, value_type<T> &vertex, weight_type<T> &weight)
{
    std::string s_vertex, s_weight;
    std::getline(i, s_vertex, '(');
    std::getline(i, s_weight, ')');
    std::istringstream stream(s_vertex + " " + s_weight);
    stream >> vertex >> weight;
    if (stream.fail())
    {
        i.setf(stream.flags());
    }
    return i;
}

template <typename T>
std::istream &operator>>(std::istream &i, WeightedGraph<T> &graph)
{
    std::string line;
    while (std::getline(i, line))
    {
        if (line.empty())
            break;
        std::istringstream line_stream(line);
        value_type<T> vertex;
        std::string s_vertex;
        std::getline(line_stream, s_vertex, ':');
        std::istringstream stream_vertex(s_vertex);
        stream_vertex >> vertex;
        if (stream_vertex.fail())
        {
            i.setf(stream_vertex.flags());
            break;
        }
        graph.push_vertex(vertex);
        std::string separator;
        value_type<T> edge_end;
        weight_type<T> edge_weight;
        while (readEdge<T>(line_stream, edge_end, edge_weight))
        {
            line_stream >> separator;
            graph.push_edge(vertex, edge_end, edge_weight);
        }
    }

    if(i.eof() and i.fail())
        i.clear(std::ios::eofbit);
    
    return i;
}