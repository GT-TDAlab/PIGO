/**
 * PIGO: a parallel graph and matrix I/O and preprocessing library
 * Copyright (c) 2022 GT-TDALab
 */

#ifdef _OPENMP
#include <omp.h>
#endif
#include <atomic>
#include <vector>
#include <type_traits>

namespace pigo {

    template<class L, class O, class S, class W, class WS, bool wgt>
    Tensor<L,O,S,W,WS,wgt>::Tensor(std::string fn) : Tensor(fn, AUTO) { }

    template<class L, class O, class S, class W, class WS, bool wgt>
    Tensor<L,O,S,W,WS,wgt>::Tensor(std::string fn, FileType ft) {
        // Open the file for reading
        ROFile f { fn };

        read_(f, ft);
    }

    template<class L, class O, class S, class W, class WS, bool wgt>
    Tensor<L,O,S,W,WS,wgt>::Tensor(File& f, FileType ft) {
        read_(f, ft);
    }

    template<class L, class O, class S, class W, class WS, bool wgt>
    void Tensor<L,O,S,W,WS,wgt>::read_(File& f, FileType ft) {
        FileType ft_used = ft;
        // If the file type is AUTO, then try to detect it
        if (ft_used == AUTO) {
            ft_used = f.guess_file_type();
        }

        if (ft_used == EDGE_LIST) {
            FileReader r = f.reader();
            read_el_(r);
        } else if (ft_used == PIGO_TENSOR_BIN) {
            read_bin_(f);
        } else {
            throw NotYetImplemented("Unknown file format");
        }
    }

    template<class L, class O, class S, class W, class WS, bool wgt>
    void Tensor<L,O,S,W,WS,wgt>::allocate_() {
        detail::allocate_mem_<S>(c_, order_*m_);
        detail::allocate_mem_<WS,wgt>(w_, m_);
    }

    namespace detail {
        template<class L, class O, class S, class W, class WS, bool count_only, bool wgt>
        struct read_tensor_entry_i_;

        /** Count-only implementation of reading a coord entry without
         * flags */
        template<class L, class O, class S, class W, class WS, bool wgt>
        struct read_tensor_entry_i_<L,O,S,W,WS,wgt,true> {
            static inline void op_(O order_, S&, WS&, size_t &coord_pos, FileReader &r) {
                for (O idx = 0; idx < order_; ++idx) {
                    if (detail::if_true_<wgt>()) {
                        r.move_to_next_int();
                    } else {
                        r.read_int<L>();
                    }
                    if (!r.good()) return;
                }
                r.move_to_eol();
                r.move_to_next_int();
                ++coord_pos;
            }
        };

        /** Setting implementation of reading a coord entry without flags */
        template<class L, class O, class S, class W, class WS, bool wgt>
        struct read_tensor_entry_i_<L,O,S,W,WS,wgt,false> {
            static inline void op_(O order_, S& c_, WS& w_, size_t &coord_pos, FileReader &r) {
                for (O idx = 0; idx < order_; ++idx) {
                    L v = r.read_int<L>();
                    if (!r.good()) return;
                    set_value_(c_, coord_pos*order_+idx, v);
                }
                read_wgt_<wgt, W, WS, false>(coord_pos, w_, r);

                r.move_to_eol();
                r.move_to_next_int();
                ++coord_pos;
            }
        };
    }

    template<class L, class O, class S, class W, class WS, bool wgt>
    template<bool count_only>
    void Tensor<L,O,S,W,WS,wgt>::read_coord_entry_(size_t &coord_pos, FileReader &r) {
        detail::read_tensor_entry_i_<L,O,S,W,WS,wgt,count_only>::op_(order_, c_, w_, coord_pos, r);
    }

    template<class L, class O, class S, class W, class WS, bool wgt>
    void Tensor<L,O,S,W,WS,wgt>::read_el_(FileReader& r) {
        // Get the number of threads
        size_t num_threads = 1;
        #ifdef _OPENMP
        omp_set_dynamic(0);
        #pragma omp parallel shared(num_threads)
        {
            #pragma omp single
            {
                num_threads = omp_get_num_threads();
            }
        }
        #endif

        // Find the order by reading the first line
        auto first_line = r;
        first_line.move_to_first_int();
        order_ = first_line.count_spaces_to_eol();
        if (!(detail::if_true_<wgt>()))
            ++order_;

        // This takes two passes:
        // first, count the number of newlines to determine how to
        // allocate storage
        // second, copy over the values appropriately

        std::vector<size_t> nl_offsets(num_threads);

        #pragma omp parallel
        {
            #ifdef _OPENMP
            size_t tid = omp_get_thread_num();
            #else
            size_t tid = 0;
            #endif

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
                read_coord_entry_<true>(tid_nls, rs_p1);
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
                read_coord_entry_<false>(coord_pos, rs_p2);
            }
        }
    }

    template<class L, class O, class S, class W, class WS, bool wgt>
    void Tensor<L,O,S,W,WS,wgt>::save(std::string fn) {
        // Before creating the file, we need to find the size
        size_t out_size = 0;
        std::string cfh { tensor_file_header };
        out_size += cfh.size();
        // Find the template sizes
        out_size += sizeof(uint8_t)*2;
        if (detail::if_true_<wgt>())
            out_size += sizeof(uint8_t);

        // Find the size of the entries, order
        out_size += 2*sizeof(O)+order_*sizeof(L);
        // Finally, find the actual Tensor sizes
        out_size += sizeof(L)*m_*order_;
        size_t w_size = detail::weight_size_<wgt, W, O>(m_);
        out_size += w_size;

        // Create the output file
        WFile w {fn, out_size};

        // Output the PIGO Tensor file header
        w.write(cfh);

        // Output the template sizes
        uint8_t L_size = sizeof(L);
        uint8_t O_size = sizeof(O);
        w.write(L_size);
        w.write(O_size);
        if (detail::if_true_<wgt>()) {
            uint8_t W_size = sizeof(W);
            w.write(W_size);
        }

        // Output the sizes and data
        w.write(order_);
        w.write(m_);

        // Output the data
        char* vc = detail::get_raw_data_<S>(c_);
        size_t vc_size = sizeof(L)*order_*m_;
        w.parallel_write(vc, vc_size);

        if (w_size > 0) {
            char* vw = detail::get_raw_data_<WS>(w_);
            w.parallel_write(vw, w_size);
        }
    }

    template<class L, class O, class S, class W, class WS, bool wgt>
    void Tensor<L,O,S,W,WS,wgt>::read_bin_(File& f) {
        // Read and confirm the header
        f.read(tensor_file_header);

        // Confirm the sizes
        uint8_t L_size, O_size;
        L_size = f.read<uint8_t>();
        O_size = f.read<uint8_t>();

        if (L_size != sizeof(L)) throw Error("Invalid Tensor template parameters to match binary");
        if (O_size != sizeof(O)) throw Error("Invalid Tensor template parameters to match binary");

        if (detail::if_true_<wgt>()) {
            uint8_t W_size = f.read<uint8_t>();
            if (W_size != sizeof(W)) throw Error("Invalid Tensor template parameters to match binary");
        }

        // Read the sizes
        order_ = f.read<O>();
        m_ = f.read<O>();

        // Allocate space
        allocate_();

        // Read out the vectors
        char* vc = detail::get_raw_data_<S>(c_);
        size_t vc_size = sizeof(L)*order_*m_;
        f.parallel_read(vc, vc_size);

        size_t w_size = detail::weight_size_<wgt, W, O>(m_);
        if (w_size > 0) {
            char* vw = detail::get_raw_data_<WS>(w_);
            f.parallel_read(vw, w_size);
        }
    }

    template<class L, class O, class S, class W, class WS, bool wgt>
    void Tensor<L,O,S,W,WS,wgt>::write(std::string fn) {
        // Writing occurs in two passes
        // First, each thread will simulate writing and compute how the
        // space taken
        // After the first pass, the output file is allocated
        // Second, each thread actually writes

        // Get the number of threads
        size_t num_threads = 1;
        #ifdef _OPENMP
        omp_set_dynamic(0);
        #pragma omp parallel shared(num_threads)
        {
            #pragma omp single
            {
                num_threads = omp_get_num_threads();
            }
        }
        #endif

        std::vector<size_t> pos_offsets(num_threads+1);
        std::shared_ptr<File> f;
        #pragma omp parallel shared(f) shared(pos_offsets)
        {
            #ifdef _OPENMP
            size_t tid = omp_get_thread_num();
            #else
            size_t tid = 0;
            #endif
            size_t my_size = 0;

            #pragma omp for
            for (O e = 0; e < m_; ++e) {
                for (O idx = 0; idx < order_; ++idx) {
                    auto x = detail::get_value_<S, L>(c_, e*order_+idx);
                    my_size += write_size(x);
                    // Account for the separating space or newline
                    my_size += 1;
                }

                if (detail::if_true_<wgt>()) {
                    // Account for the separating space
                    my_size += 1;

                    auto w = detail::get_value_<WS, W>(w_, e);
                    my_size += write_size(w);
                }
            }

            pos_offsets[tid+1] = my_size;
            #pragma omp barrier

            #pragma omp single
            {
                // Compute the total size and perform a prefix sum
                pos_offsets[0] = 0;
                for (size_t thread = 1; thread <= num_threads; ++thread)
                    pos_offsets[thread] = pos_offsets[thread-1] + pos_offsets[thread];

                // Allocate the file
                f = std::make_shared<File>(fn, WRITE, pos_offsets[num_threads]);
            }

            #pragma omp barrier

            FilePos my_fp = f->fp()+pos_offsets[tid];

            // Perform the second pass, actually writing out to the file
            #pragma omp for
            for (O e = 0; e < m_; ++e) {
                for (O idx = 0; idx < order_; ++idx) {
                    auto x = detail::get_value_<S, L>(c_, e*order_+idx);
                    write_ascii(my_fp, x);
                    if (idx < order_-1)
                        pigo::write(my_fp, ' ');
                }
                if (detail::if_true_<wgt>()) {
                    pigo::write(my_fp, ' ');
                    auto w = detail::get_value_<WS, W>(w_, e);
                    write_ascii(my_fp, w);
                }
                pigo::write(my_fp, '\n');
            }
        }
    }

    template<class L, class O, class S, class W, class WS, bool wgt>
    std::vector<L> Tensor<L,O,S,W,WS,wgt>::max_labels() const {
        // Get the number of threads
        size_t num_threads = 1;
        #ifdef _OPENMP
        omp_set_dynamic(0);
        #pragma omp parallel shared(num_threads)
        {
            #pragma omp single
            {
                num_threads = omp_get_num_threads();
            }
        }
        #endif

        std::vector<L*> maxes(num_threads);
        #pragma omp parallel
        {
            #ifdef _OPENMP
            size_t tid = omp_get_thread_num();
            #else
            size_t tid = 0;
            #endif
            L* my_maxes = new L[order_];
            for (O idx = 0; idx < order_; ++idx) my_maxes[idx] = 0;

            const L* c = (const L*)detail::get_const_data_<S>(c_);
            #pragma omp for
            for (O e = 0; e < m_; ++e) {
                for (O idx = 0; idx < order_; ++idx) {
                    L val = c[e*order_+idx];
                    if (val > my_maxes[idx]) my_maxes[idx] = val;
                }
            }

            maxes[tid] = my_maxes;
        }

        std::vector<L> ret(order_);
        for (size_t tid = 0; tid < num_threads; ++tid) {
            for (O idx = 0; idx < order_; ++idx)
                if (maxes[tid][idx] > ret[idx]) ret[idx] = maxes[tid][idx];
            delete [] maxes[tid];
        }

        return ret;
    }

}
