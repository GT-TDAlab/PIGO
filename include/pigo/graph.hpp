/**
 * PIGO: a parallel graph and matrix I/O and preprocessing library
 * Copyright (c) 2021 GT-TDALab
 *
 * This contains the Graph class, interfacing to a CSR
 */

#ifndef PIGO_GRAPH_HPP
#define PIGO_GRAPH_HPP

#include <string>
#include <memory>

namespace pigo {
    /** @brief An iterator type for edges
     *
     * @tparam V the vertex ID type
     * @tparam O the ordinal type
     * @tparam S the storage type for the edges
     */
    template<class V, class O, class S>
    class EdgeItT {
        private:
            /** The current position in the edge list */
            O pos;
            /** The edge list in the given storage format */
            S s;
        public:
            /** @brief Initialize the edge list iterator type
            *
            * @param s the storage for the edge list endpoints
            * @param pos the position in the storage
            */
            EdgeItT(S s, O pos) : pos(pos), s(s) { }
            bool operator!=(EdgeItT& rhs) { return pos != rhs.pos; }
            V& operator*();
            void operator++() { ++pos; }
    };

    /** @brief An iterable edge list
     *
     * @tparam V the vertex ID type
     * @tparam O the ordinal type
     * @tparam S the storage type for the edges
     * @param begin the starting offset
     * @param end one passed the starting offset
     */
    template<class V, class O, class S>
    class EdgeIt {
        private:
            /** The beginning position */
            O begin_;
            /** The ending position */
            O end_;
            /** The storage */
            S s;
        public:
            /** @brief Construct the edge iterator for the given offset values
            *
            * @param begin the starting offset
            * @param end the ending offset
            * @param s the storage
            */
            EdgeIt(O begin, O end, S s) : begin_(begin),
                    end_(end), s(s) { }

            /** Return the iterator start */
            EdgeItT<V,O,S> begin() { return EdgeItT<V,O,S> {s, begin_}; }
            /** Return the iterator end */
            EdgeItT<V,O,S> end() { return EdgeItT<V,O,S> {s, end_}; }
    };


    /** @brief Used to load graphs from disk
     *
     * This class provides a graph-specific naming on top of CSRs.
     *
     * @tparam vertex_t the type to use for vertices
     * @tparam edge_ctr_t the type to use for an edge counter
     * @tparam edge_storage the storage type of the endpoints of the CSR.
     * @tparam edge_ctr_storage the storage type of the offsets of the
     *         CSR.
     */
    template<
        class vertex_t=uint32_t,
        class edge_ctr_t=uint32_t,
        class edge_storage=vertex_t*,
        class edge_ctr_storage=edge_ctr_t*
    >
    class BaseGraph : public CSR<vertex_t, edge_ctr_t,
            edge_storage, edge_ctr_storage> {
        public:
            using CSR<vertex_t, edge_ctr_t, edge_storage, edge_ctr_storage>::CSR;

            /** @brief Return the neighbors of a vertex
             *
             * @param v the vertex to find the neighbors at
             * @return an edge_ctr_t containing the neighbor start offset
             */
            edge_ctr_t neighbor_start(vertex_t v);

            /** @brief Return beyond the neighbors of a vertex
             *
             * @param v the vertex to end the neighbors at
             * @return an edge_ctr_t just beyond the neighbor list
             */
            edge_ctr_t neighbor_end(vertex_t v);

            /** @brief Return an iterator for the neighbors the vertex
             *
             * @param v the vertex to iterate over the neighbors of
             * @return an edge iterator
             */
            EdgeIt<vertex_t, edge_ctr_t, edge_storage> neighbors(vertex_t v) {
                return EdgeIt<vertex_t, edge_ctr_t, edge_storage> {neighbor_start(v),
                    neighbor_end(v), this->endpoints_};
            }
    };

    /** @brief A basic Graph suitable for most cases */
    using Graph = BaseGraph<>;

    /** @brief A Graph suitable for large graphs requiring 64-bit ids */
    using BigGraph = BaseGraph<uint64_t, uint64_t>;

}

#endif
