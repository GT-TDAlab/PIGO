/**
 * PIGO: a parallel graph and matrix I/O and preprocessing library
 * Copyright (c) 2021 GT-TDALab
 */

#include <omp.h>
#include <atomic>
#include <vector>
#include <type_traits>

namespace pigo {

    template<class L, class O, class S, bool sym, bool ut, bool sl, bool me>
    COO<L,O,S,sym,ut,sl,me>::COO(std::string fn) : COO(fn, AUTO) { }

    template<class L, class O, class S, bool sym, bool ut, bool sl, bool me>
    COO<L,O,S,sym,ut,sl,me>::COO(std::string fn, FileType ft) {
        // Open the file for reading
        ROFile f { fn };

        read_(f, ft);
    }

    template<class L, class O, class S, bool sym, bool ut, bool sl, bool me>
    COO<L,O,S,sym,ut,sl,me>::COO(File& f, FileType ft) {
        read_(f, ft);
    }

    template<class L, class O, class S, bool sym, bool ut, bool sl, bool me>
    void COO<L,O,S,sym,ut,sl,me>::read_(File& f, FileType ft) {
        FileType ft_used = ft;
        // If the file type is AUTO, then try to detect it
        if (ft_used == AUTO) {
            ft_used = f.guess_file_type();
        }

        if (ft_used == MATRIX_MARKET) {
            FileReader r = f.reader();
            read_mm_(r);
        } else if (ft_used == EDGE_LIST) {
            FileReader r = f.reader();
            read_el_(r);
        } else if (ft_used == PIGO_COO_BIN) {
            read_bin_(f);
        } else if (ft_used == PIGO_CSR_BIN ||
                ft_used == GRAPH) {
            // First build a CSR, then convert to a COO
            CSR<L,O,S,S> csr {f, ft_used};
            convert_csr_(csr);
            csr.free();
        } else {
            // We need to first build a CSR, then move back to a COO
            throw NotYetImplemented("Coming in v0.6");
        }
    }

    template<class L, class O, class S, bool sym, bool ut, bool sl, bool me>
    template<typename LS, typename OS>
    COO<L,O,S,sym,ut,sl,me>::COO(CSR<L,O,LS,OS>& csr) {
        convert_csr_(csr);
    }

    template<class L, class O, class S, bool sym, bool ut, bool sl, bool me>
    template<typename LS, typename OS>
    void COO<L,O,S,sym,ut,sl,me>::convert_csr_(CSR<L,O,LS,OS>& csr) {
        // First, set our sizes and allocate space
        n_ = csr.n();
        m_ = csr.m();

        allocate_();

        auto storage_offsets = csr.offsets();
        auto storage_endpoints = csr.endpoints();
        O* offsets = (O*)get_raw_data_(storage_offsets);
        L* endpoints = (L*)get_raw_data_(storage_endpoints);

        #pragma omp parallel for schedule(dynamic, 10240)
        for (L v = 0; v < n_; ++v) {
            auto start = endpoints + offsets[v];
            auto end = endpoints + offsets[v+1];
            size_t coo_cur = offsets[v];
            for (auto cur = start; cur < end; ++cur, ++coo_cur) {
                set_value_(x_, coo_cur, v);
                set_value_(y_, coo_cur, *cur);
            }
        }
    }

    template<class L, class O, class S, bool sym, bool ut, bool sl, bool me>
    void COO<L,O,S,sym,ut,sl,me>::allocate_() {
        allocate_mem_<S>(x_, m_);
        allocate_mem_<S>(y_, m_);
    }

    /** Handle different options for reading out entries */
    template<class L, bool sl>
    struct coo_read_rsl_i_;

    template<class L>
    struct coo_read_rsl_i_<L, true> {
        static bool op_(L &x, L &y) {
            return x != y;
        }
    };

    template<class L>
    struct coo_read_rsl_i_<L, false> {
        static bool op_(L&, L&) { return true; }
    };

    template<class L, class O, class S, bool sym, bool ut, bool sl, bool me>
    void COO<L,O,S,sym,ut,sl,me>::read_coord_entry_(size_t &coord_pos, FileReader &r,
            L &max_row, L &max_col, bool count_only) {
        // Get the values
        if (!r.good()) return; // Abort early, which will not update the pos or max
        L x = r.read_int<L>();
        r.move_to_next_int();
        if (!r.good()) return; // Abort early, which will not update the pos or max
        L y = r.read_int<L>();
        if (!r.good()) return; // Abort early, which will not update the pos or max

        if (!coo_read_rsl_i_<L, sl>::op_(x, y))
            return read_coord_entry_(coord_pos, r, max_row, max_col, count_only);

        if (!count_only) {
            set_value_(x_, coord_pos, x);
            set_value_(y_, coord_pos, y);
        }

        ++coord_pos;
        if (x > max_row) max_row = x;
        if (y > max_col) max_col = y;
    }

    template<class L, class O, class S, bool sym, bool ut, bool sl, bool me>
    void COO<L,O,S,sym,ut,sl,me>::read_mm_(FileReader& r) {
        // Matrix market is ready similar to edge lists, however first the
        // header is skipped
        // Furthermore, any attributes are ignored (symmetric, etc.)
        // This should change in a future PIGO version
        if (!r.at_str("%%MatrixMarket matrix coordinate"))
            throw NotYetImplemented("Unable to handle different MatrixMarket formats");

        // Read out the first line
        r.move_to_next_int();
        L nrows = r.read_int<L>()+1;        // account for MM starting at 1
        r.move_to_next_int();
        L ncols = r.read_int<L>()+1;        // account for MM starting at 1
        r.move_to_next_int();
        O nnz = r.read_int<O>();
        r.move_to_eol();
        r.move_to_next_int();

        // Now, read out the actual contents
        read_el_(r);

        // Finally, sanity check the file
        if (nrows >= nrows_)
            nrows_ = nrows;
        else {
            free();
            throw Error("Too many row labels in file contradicting header");
        }

        if (ncols >= ncols_)
            ncols_ = ncols;
        else {
            free();
            throw Error("Too many col labels in file contradicting header");
        }
        if (nnz != m_) {
            free();
            throw Error("Header contradicts number of read non-zeros");
        }
    }

    template<class L, class O, class S, bool sym, bool ut, bool sl, bool me>
    void COO<L,O,S,sym,ut,sl,me>::read_el_(FileReader& r) {
        // Get the number of threads
        omp_set_dynamic(0);
        size_t num_threads = 0;
        #pragma omp parallel shared(num_threads)
        {
            #pragma omp single
            {
                num_threads = omp_get_num_threads();
            }
        }

        // This takes two passes:
        // first, count the number of newlines to determine how to
        // allocate storage
        // second, copy over the values appropriately

        std::vector<size_t> nl_offsets(num_threads);

        L max_row = 0;
        L max_col = 0;
        #pragma omp parallel reduction(max : max_row) \
                reduction(max : max_col)
        {
            size_t tid = omp_get_thread_num();

            // Find our offsets in the file
            size_t size = r.size();
            size_t tid_start_i = (tid*size)/num_threads;
            size_t tid_end_i = ((tid+1)*size)/num_threads;
            FileReader rs = r + tid_start_i;
            FileReader re = r + tid_end_i;

            // Now, move to the appropriate starting point to move off of
            // overlapping entries
            re.move_to_eol();
            re.move_to_next_int();
            if (tid != 0) {
                rs.move_to_eol();
                rs.move_to_next_int();
            } else
                rs.move_to_first_int();

            // Set our file reader to end either at the full end or at
            // the thread id local end
            rs.smaller_end(re);

            // Pass 1
            // Iterate through, counting the number of newlines
            FileReader rs_p1 = rs;
            size_t tid_nls = 0;
            while (rs_p1.good()) {
                rs_p1.move_to_eol();
                rs_p1.move_to_next_int();
                ++tid_nls;
            }

            nl_offsets[tid] = tid_nls;

            // Compute a prefix sum on the newline offsets
            #pragma omp barrier
            #pragma omp single
            {
                size_t sum_nl = 0;
                for (size_t tid = 0; tid < num_threads; ++tid) {
                    sum_nl += nl_offsets[tid];
                    nl_offsets[tid] = sum_nl;
                }

                // Now, allocate the space appropriately
                m_ = nl_offsets[num_threads-1];
                allocate_();
            }
            #pragma omp barrier

            // Pass 2
            // Iterate through again, but now copying out the integers
            FileReader rs_p2 = rs;
            size_t coord_pos = 0;
            if (tid > 0)
                coord_pos = nl_offsets[tid-1];

            while (rs_p2.good()) {
                read_coord_entry_(coord_pos, rs_p2, max_row, max_col);
                rs_p2.move_to_eol();
                rs_p2.move_to_next_int();
            }
        }

        // Set the number of labels in the matrix represented by the COO
        nrows_ = max_row + 1;
        ncols_ = max_col + 1;
        if (nrows_ > ncols_) n_ = nrows_;
        else n_ = ncols_;
    }

    template<class L, class O, class S, bool sym, bool ut, bool sl, bool me>
    void COO<L,O,S,sym,ut,sl,me>::save(std::string fn) {
        // Before creating the file, we need to find the size
        size_t out_size = 0;
        std::string cfh { coo_file_header };
        out_size += cfh.size();
        // Find the template sizes
        out_size += sizeof(uint8_t)*2;
        // Find the size of the number of rows/etc.
        out_size += sizeof(L)*3+sizeof(O);
        // Finally, find the actual COO sizes
        out_size += sizeof(L)*m_*2;

        // Create the output file
        WFile w {fn, out_size};

        // Output the PIGO COO file header
        w.write(cfh);

        // Output the template sizes
        uint8_t L_size = sizeof(L);
        uint8_t O_size = sizeof(O);
        w.write(L_size);
        w.write(O_size);

        // Output the sizes and data
        w.write(nrows_);
        w.write(ncols_);
        w.write(n_);
        w.write(m_);

        // Output the data
        char* vx = get_raw_data_<S>(x_);
        size_t vx_size = sizeof(L)*m_;
        w.parallel_write(vx, vx_size);

        char* vy = get_raw_data_<S>(y_);
        size_t vy_size = sizeof(L)*m_;
        w.parallel_write(vy, vy_size);
    }

    template<class L, class O, class S, bool sym, bool ut, bool sl, bool me>
    void COO<L,O,S,sym,ut,sl,me>::read_bin_(File& f) {
        // Read and confirm the header
        f.read(coo_file_header);

        // Confirm the sizes
        uint8_t L_size, O_size;
        L_size = f.read<uint8_t>();
        O_size = f.read<uint8_t>();

        if (L_size != sizeof(L)) throw Error("Invalid COO template parameters to match binary");
        if (O_size != sizeof(O)) throw Error("Invalid COO template parameters to match binary");

        // Read the sizes
        nrows_ = f.read<L>();
        ncols_ = f.read<L>();
        n_ = f.read<L>();
        m_ = f.read<O>();

        // Allocate space
        allocate_();

        // Read out the vectors
        char* vx = get_raw_data_<S>(x_);
        size_t vx_size = sizeof(L)*m_;
        f.parallel_read(vx, vx_size);

        char* vy = get_raw_data_<S>(y_);
        size_t vy_size = sizeof(L)*m_;
        f.parallel_read(vy, vy_size);
    }

}
