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
     * @tparam weighted if true, support and use weights
     * @tparam Weight the weight data type.
     * @tparam WeightStorage the storage type for the weights. This can be
     *         a raw pointer (Weight*), a std::vector
     *         (std::vector<Weight>), or a std::shared_ptr<Weight>.
     */
    template<
        class vertex_t=uint32_t,
        class edge_ctr_t=uint32_t,
        class edge_storage=vertex_t*,
        class edge_ctr_storage=edge_ctr_t*,
        bool weighted=false,
        class Weight=float,
        class WeightStorage=Weight*
    >
    class BaseGraph : public CSR<
                        vertex_t,
                        edge_ctr_t,
                        edge_storage,
                        edge_ctr_storage,
                        weighted,
                        Weight,
                        WeightStorage
                    > {
        public:
            using CSR<vertex_t, edge_ctr_t, edge_storage, edge_ctr_storage,
                  weighted, Weight, WeightStorage>::CSR;

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

            /** @brief Return the number of neighbors (degree) of a vertex
             *
             * @param v the vertex to return the degree of
             * @return the number of neighbors of the vertex
             */
            edge_ctr_t degree(vertex_t v) {
                return neighbor_end(v)-neighbor_start(v);
            }

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

    /** @brief Used to load directed graphs from disk
     *
     * This class provides a graph-specific naming on top of non-symmetric
     * matrices (Matrix).
     *
     * @tparam vertex_t the type to use for vertices
     * @tparam edge_ctr_t the type to use for an edge counter
     * @tparam edge_storage the storage type of the endpoints of the
     *         CSR/CSC.
     * @tparam edge_ctr_storage the storage type of the offsets of the
     *         CSR/CSC.
     * @tparam weighted if true, support and use weights
     * @tparam Weight the weight data type.
     * @tparam WeightStorage the storage type for the weights. This can be
     *         a raw pointer (Weight*), a std::vector
     *         (std::vector<Weight>), or a std::shared_ptr<Weight>.
     */
    template<
        class vertex_t=uint32_t,
        class edge_ctr_t=uint32_t,
        class edge_storage=vertex_t*,
        class edge_ctr_storage=edge_ctr_t*,
        bool weighted=false,
        class Weight=float,
        class WeightStorage=Weight*
    >
    class DiGraph {
        private:
            /** Holds the in-edges */
            BaseGraph<
                    vertex_t,
                    edge_ctr_t,
                    edge_storage,
                    edge_ctr_storage,
                    weighted,
                    Weight,
                    WeightStorage
                > in_;

            /** Holds the out-edges */
            BaseGraph<
                    vertex_t,
                    edge_ctr_t,
                    edge_storage,
                    edge_ctr_storage,
                    weighted,
                    Weight,
                    WeightStorage
                > out_;

            /** @brief Read the DiGraph from the given file and format
             *
             * @param f the File to read from
             * @param ft the FileFormat to use to read
             */
            void read_(File& f, FileType ft);

            /** @build a DiGraph from a COO */
            template <class COOvertex_t, class COOedge_ctr_t, class COOStorage,
                     bool COOsym, bool COOut, bool COOsl,
                     class COOW, class COOWS>
            void from_coo_(COO<COOvertex_t, COOedge_ctr_t, COOStorage, COOsym, COOut,
                    COOsl, weighted, COOW, COOWS>& coo) {
                auto coo_copy = coo;
                coo_copy.transpose();

                in_ = BaseGraph<
                            vertex_t,
                            edge_ctr_t,
                            edge_storage,
                            edge_ctr_storage,
                            weighted,
                            Weight,
                            WeightStorage
                        > { coo_copy };
                coo_copy.free();

                out_ = BaseGraph<
                            vertex_t,
                            edge_ctr_t,
                            edge_storage,
                            edge_ctr_storage,
                            weighted,
                            Weight,
                            WeightStorage
                        > { coo };
            }


        public:
            /** @brief Initialize from a COO
             *
             * This creates a DiGraph from an already-loaded COO.
             *
             * This first copies the COO and transposes the copy, setting
             * that as the in-edges.
             * The out-edges are the original COO.
             *
             * @tparam COOvertex_t the label for the COO format
             * @tparam COOedge_ctr_t the ordinal for the COO format
             * @tparam COOStorage the storage format of the COO
             * @tparam COOsym whether the COO is symmetrized
             * @tparam COOut whether the COO only keeps the upper triangle
             * @tparam COOsl whether the COO removes self loops
             * @tparam COOW the weight type of the COO
             * @tparam COOWS the weight storage type of the COO
             * @param coo the COO object to load the CSR from
             */
            template <class COOvertex_t, class COOedge_ctr_t, class COOStorage,
                     bool COOsym, bool COOut, bool COOsl,
                     class COOW, class COOWS>
            DiGraph(COO<COOvertex_t, COOedge_ctr_t, COOStorage, COOsym, COOut,
                    COOsl, weighted, COOW, COOWS>& coo) {
                from_coo_(coo);
            }

            /** @brief Initialize from a file
             *
             * The file type will attempt to be determined automatically.
             *
             * @param fn the filename to open
             */
            DiGraph(std::string fn);

            /** @brief Initialize from a file with a specific type
             *
             * @param fn the filename to open
             * @param ft the FileType to use
             */
            DiGraph(std::string fn, FileType ft);

            /** @brief Initialize from an open file with a specific type
             *
             * @param f the open File
             * @param ft the FileType to use
             */
            DiGraph(File& f, FileType ft);

            /** @brief Free the associated memory */
            void free() {
                in_.free();
                out_.free();
            }

            /** @brief Return the number of non-zeros or edges */
            edge_ctr_t m() { return out_.m(); }

            /** @brief Return the number of vertices */
            edge_ctr_t n() { return out_.n(); }

            /** @brief Return the number of rows */
            edge_ctr_t nrows() { return out_.nrows(); }

            /** @brief Return the number of columns */
            edge_ctr_t ncols() { return out_.ncols(); }

            /** @brief Return the out-edges */
            BaseGraph<
                vertex_t,
                edge_ctr_t,
                edge_storage,
                edge_ctr_storage,
                weighted,
                Weight,
                WeightStorage
            >& out() { return out_; }

            /** @brief Return the in-edges */
            BaseGraph<
                vertex_t,
                edge_ctr_t,
                edge_storage,
                edge_ctr_storage,
                weighted,
                Weight,
                WeightStorage
            >& in() { return in_; }

            /** @brief Save the loaded DiGraph as a PIGO binary file
             *
             * This saves the current DiGraph to disk
             *
             * @param fn the filename to save as
             */
            void save(std::string fn);

            /** The output file header for reading/writing */
            static constexpr const char* digraph_file_header = "PIGO-DiGraph-v1";
    };

}

#endif
