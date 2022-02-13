/**
 * PIGO: a parallel graph and matrix I/O and preprocessing library
 * Copyright (c) 2022 GT-TDALab
 */

#include <algorithm>
#include <atomic>
#include <vector>
#include <string>
#ifdef _OPENMP
#include <omp.h>
#endif

namespace pigo {
    template<class L, class O, class LS, class OS, bool wgt, class W, class WS>
    CSR<L,O,LS,OS,wgt,W,WS>::CSR(std::string fn) : CSR(fn, AUTO) { }

    template<class L, class O, class LS, class OS, bool wgt, class W, class WS>
    CSR<L,O,LS,OS,wgt,W,WS>::CSR(std::string fn, FileType ft) {
        // Open the file for reading
        ROFile f {fn};
        read_(f, ft);
    }

    template<class L, class O, class LS, class OS, bool wgt, class W, class WS>
    CSR<L,O,LS,OS,wgt,W,WS>::CSR(File& f, FileType ft) {
        read_(f, ft);
    }

    namespace detail {
        template<bool wgt>
        struct fail_if_weighted_i_ { static void op_() {} };
        template<>
        struct fail_if_weighted_i_<true> {
            static void op_() {
                throw NotYetImplemented("Not yet implemented for weights.");
            }
        };
        template<bool wgt>
        void fail_if_weighted() {
            fail_if_weighted_i_<wgt>::op_();
        }
    }

    template<class L, class O, class LS, class OS, bool wgt, class W, class WS>
    void CSR<L,O,LS,OS,wgt,W,WS>::read_(File& f, FileType ft) {
        FileType ft_used = ft;
        // If the file type is AUTO, then try to detect it
        if (ft_used == AUTO) {
            ft_used = f.guess_file_type();
        }

        if (ft_used == MATRIX_MARKET || ft_used == EDGE_LIST ||
                ft_used == PIGO_COO_BIN) {
            // First build a COO, then load here
            COO<L,O,L*, false, false, false, wgt, W, WS> coo { f, ft_used };
            convert_coo_(coo);
            coo.free();
        } else if (ft_used == PIGO_CSR_BIN) {
            read_bin_(f);
        } else if (ft_used == GRAPH) {
            detail::fail_if_weighted<wgt>();
            FileReader r = f.reader();
            read_graph_(r);
        } else
            throw NotYetImplemented("This file type is not yet supported");
    }

    template<class L, class O, class LS, class OS, bool wgt, class W, class WS>
    void CSR<L,O,LS,OS,wgt,W,WS>::allocate_() {
        detail::allocate_mem_<LS>(endpoints_, m_);
        detail::allocate_mem_<WS,wgt>(weights_, m_);
        detail::allocate_mem_<OS>(offsets_, n_+1);
    }
    namespace detail {
        template<bool wgt, class W, class WS, class COOW, class COOWS, class O>
        struct copy_weight_i_ {
            static void op_(WS&, O, COOWS&, O) {}
        };

        template<class W, class WS, class COOW, class COOWS, class O>
        struct copy_weight_i_<true, W, WS, COOW, COOWS, O> {
            static void op_(WS& w, O p, COOWS& coow, O coop) {
                COOW val = get_value_<COOWS, COOW>(coow, coop);
                set_value_<WS, W>(w, p, (W)val);
            }
        };

        template<bool wgt, class W, class WS, class COOW, class COOWS, class O>
        void copy_weight(WS& w, O offset, COOWS& coo_w, O coo_pos) {
            copy_weight_i_<wgt, W, WS, COOW, COOWS, O>::op_(w, offset,
                    coo_w, coo_pos);
        }
    }

    template<class L, class O, class LS, class OS, bool wgt, class W,
        class WS> template<class COOL, class COOO, class COOStorage, bool
            COOsym, bool COOut, bool COOsl, class COOW, class
            COOWS>
    CSR<L,O,LS,OS,wgt,W,WS>::CSR(COO<COOL,COOO,COOStorage,COOsym,
            COOut,COOsl,wgt,COOW,COOWS>
            &coo) {
        convert_coo_(coo);
    }

    template<class L, class O, class LS, class OS, bool wgt, class W,
        class WS> template<class COOL, class COOO, class COOStorage,
        bool COOsym, bool COOut, bool COOsl,
        class COOW, class COOWS>
    void CSR<L,O,LS,OS,wgt,W,WS>::convert_coo_(COO<
            COOL,COOO,COOStorage,COOsym,COOut,COOsl,wgt,COOW,COOWS>&
            coo) {
        // Set the sizes first
        n_ = coo.n();
        m_ = coo.m();
        nrows_ = coo.nrows();
        ncols_ = coo.ncols();

        // Allocate the offsets and endpoints
        allocate_();

        // This is a multi pass algorithm.
        // First, we need to count each vertex's degree and allocate the
        // space appropriately.
        // Next, we go through the degrees and change them to offsets
        // Finally, we need to go through the COO and copy memory

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

        // Temporarily keep track of the degree for each label (this can
        // easily be computed later)
        O* label_degs = new O[n_];

        // Keep track of the starting offsets for each thread
        O* start_offsets = new O[num_threads];

        // Each thread will compute the degrees for each label on its own.
        // This is then used to reduce them all
        O* all_degs = new O[n_];
        #pragma omp parallel shared(all_degs) shared(label_degs) shared(start_offsets)
        {
            #ifdef _OPENMP
            size_t tid = omp_get_thread_num();
            #else
            size_t tid = 0;
            #endif

            L v_start = (tid*n_)/num_threads;
            L v_end = ((tid+1)*n_)/num_threads;
            // We need to initialize degrees to count for zero-degree
            // vertices
            #pragma omp for
            for (L v = 0; v < n_; ++v)
                all_degs[v] = 0;

            auto coo_x = coo.x();
            auto coo_y = coo.y();
            auto coo_w = coo.w();

            #pragma omp for
            for (O x_id = 0; x_id < m_; ++x_id) {
                size_t deg_inc = detail::get_value_<COOStorage, L>(coo_x, x_id);
                #pragma omp atomic
                ++all_degs[deg_inc];
            }

            // Reduce the degree vectors
            #pragma omp barrier
            // Now all degs (via all_degs) have been computed

            O my_degs = 0;
            for (L c = v_start; c < v_end; ++c) {
                O this_deg = all_degs[c];
                label_degs[c] = this_deg;
                my_degs += this_deg;
            }

            // Save our local degree count to do a prefix sum on
            start_offsets[tid] = my_degs;

            // Get a memory allocation
            // Do a prefix sum to keep everything compact by row
            #pragma omp barrier
            #pragma omp single
            {
                O total_degs = 0;
                for (size_t cur_tid = 0; cur_tid < num_threads; ++cur_tid) {
                    total_degs += start_offsets[cur_tid];
                    start_offsets[cur_tid] = total_degs;
                }
            }
            #pragma omp barrier

            // Get the starting offset
            // The prefix sum array is off by one, so the start is at zero
            O cur_offset = 0;
            if (tid > 0)
                cur_offset = start_offsets[tid-1];

            // Now, assign the offsets to each label
            for (L c = v_start; c < v_end; ++c) {
                detail::set_value_(offsets_, c, cur_offset);
                cur_offset += label_degs[c];
            }
            #pragma omp single
            {
                // Patch the last offset to the end, making for easier
                // degree computation and iteration
                detail::set_value_(offsets_, n_, m_);
            }

            #pragma omp barrier
            // Now, all offsets_ have been assigned

            // Here, we use the degrees computed earlier and treat them
            // instead as the remaining vertices, showing the current copy
            // position

            // Finally, copy over the actual endpoints
            #pragma omp for
            for (O coo_pos = 0; coo_pos < m_; ++coo_pos) {
                L src = detail::get_value_<COOStorage, L>(coo_x, coo_pos);
                L dst = detail::get_value_<COOStorage, L>(coo_y, coo_pos);

                O this_offset_pos;
                #pragma omp atomic capture
                {
                    this_offset_pos = label_degs[src];
                    label_degs[src]--;
                }
                O this_offset = detail::get_value_<OS, O>(offsets_, src+1) - this_offset_pos;
                detail::set_value_(endpoints_, this_offset, dst);
                detail::copy_weight<wgt,W,WS,COOW,COOWS>(weights_, this_offset, coo_w, coo_pos);
            }

        }

        delete [] label_degs;
        delete [] start_offsets;
        delete [] all_degs;

    }

    template<class L, class O, class LS, class OS, bool wgt, class W, class WS>
    void CSR<L,O,LS,OS,wgt,W,WS>::read_graph_(FileReader &r) {
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

        // We have a header containing the number of vertices and edges.
        // This is used to verify and check correctness
        r.move_to_first_int();
        L read_n = r.read_int<L>();
        r.move_to_next_int();
        O read_m = r.read_int<O>();
        r.move_to_eol();
        r.move_to_next_int();

        // This takes two passes:
        // first, count the number of newlines and integers to allocate
        // storage
        // second, copy over the values appropriately

        std::vector<size_t> nl_offsets(num_threads);
        std::vector<size_t> int_offsets(num_threads);
        std::vector<L> max_labels(num_threads);
        std::vector<bool> have_zeros(num_threads, false);
        bool have_zero;
        #pragma omp parallel shared(have_zero) shared(have_zeros)
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

            // Now, move to the appropriate starting point
            re.move_to_next_int();
            if (tid != 0) {
                rs.move_to_next_int();
            } else
                rs.move_to_first_int();

            // Set our file reader to end either at the full end or at
            // the thread id local end
            rs.smaller_end(re);

            // Now, perform the first pass and count
            FileReader rs_p1 = rs;
            size_t tid_nls = 0;
            size_t tid_ints = 0;
            bool my_have_zero = false;
            while (rs_p1.good()) {
                if (rs_p1.at_nl_or_eol())
                    ++tid_nls;
                else
                    ++tid_ints;
                // Determine if this is a 0
                if (rs_p1.at_zero() && !my_have_zero)
                    my_have_zero = true;

                rs_p1.move_to_next_int_or_nl();
            }
            if (my_have_zero) {
                have_zeros[tid] = true;
            }

            #pragma omp barrier
            #pragma omp single
            {
                bool found_zero = false;
                for (size_t tid = 0; tid < num_threads; ++tid) {
                    if (have_zeros[tid]) {
                        found_zero = true;
                        break;
                    }
                }
                if (found_zero) have_zero = true;
                else have_zero = false;
            }
            #pragma omp barrier

            nl_offsets[tid] = tid_nls;
            int_offsets[tid] = tid_ints;

            // Compute a prefix sum on the offsets
            #pragma omp barrier
            #pragma omp single
            {
                size_t sum_nl = (have_zero) ? 0 : 1;
                size_t sum_ints = 0;
                for (size_t tid = 0; tid < num_threads; ++tid) {
                    sum_nl += nl_offsets[tid];
                    nl_offsets[tid] = sum_nl;

                    sum_ints += int_offsets[tid];
                    int_offsets[tid] = sum_ints;
                }

                // Now, allocate the space appropriately
                m_ = int_offsets[num_threads-1];
                n_ = nl_offsets[num_threads-1];
                nrows_ = n_;
                allocate_();
                detail::set_value_(offsets_, 0, 0);
                if (!have_zero)
                    detail::set_value_(offsets_, 1, 0);
                detail::set_value_(offsets_, n_, m_);
            }
            #pragma omp barrier

            // Pass 2: iterate through again, but now copy out the values
            // to the appropriate position in the endpoints / offsets
            L my_max = 0;
            FileReader rs_p2 = rs;
            O endpoint_pos = 0;
            L offset_pos = 0;
            if (tid > 0) {
                offset_pos = nl_offsets[tid-1];
                endpoint_pos = int_offsets[tid-1];
            } else if (!have_zero)
                offset_pos = 1;

            while (rs_p2.good()) {
                // Ignore any trailing data in the file
                if (offset_pos >= n_) break;

                // Copy and set the endpoint
                if (rs_p2.at_nl_or_eol()) {
                    // Set the offset to the current endpoint position
                    detail::set_value_(offsets_, ++offset_pos, endpoint_pos);
                    rs_p2.move_to_next_int_or_nl();
                } else {
                    // Set the actual value
                    L endpoint = rs_p2.read_int<L>();
                    if (endpoint > my_max)
                        my_max = endpoint;
                    detail::set_value_(endpoints_, endpoint_pos++, endpoint);
                    if (!rs_p2.at_nl_or_eol())
                        rs_p2.move_to_next_int_or_nl();
                }
            }
            max_labels[tid] = my_max;
        }

        if (m_ == 2*read_m) {}
        else if (m_ != read_m) throw Error("Mismatch in CSR nonzeros and header");
        if (n_ < read_n) throw Error("Mismatch in CSR newlines and header");
        else n_ = read_n;

        L col_max = max_labels[0];
        for (size_t thread = 1; thread < num_threads; ++thread) {
            if (max_labels[thread] > col_max)
                col_max = max_labels[thread];
        }
        ncols_ = col_max+1;
    }

    template<class L, class O, class LS, class OS, bool wgt, class W, class WS>
    size_t CSR<L,O,LS,OS,wgt,W,WS>::save_size() const {
        size_t out_size = 0;
        std::string cfh { csr_file_header };
        out_size += cfh.size();
        // Find the template sizes
        out_size += sizeof(uint8_t)*2;
        // Find the size of the size of the CSR
        out_size += sizeof(L)*3+sizeof(O);
        // Finally, find the actual CSR size
        size_t voff_size = sizeof(O)*(n_+1);
        size_t vend_size = sizeof(L)*m_;
        size_t w_size = detail::weight_size_<wgt, W, O>(m_);
        out_size += voff_size + vend_size + w_size;

        return out_size;
    }

    template<class L, class O, class LS, class OS, bool wgt, class W, class WS>
    void CSR<L,O,LS,OS,wgt,W,WS>::save(std::string fn) {
        // Before creating the file, we need to find the size
        size_t out_size = save_size();

        // Create the output file
        WFile w {fn, out_size};

        // Now, perform the actual save
        save(w);
    }

    template<class L, class O, class LS, class OS, bool wgt, class W, class WS>
    void CSR<L,O,LS,OS,wgt,W,WS>::save(File& w) {
        // Output the file header
        std::string cfh { csr_file_header };
        w.write(cfh);

        // Output the template sizes
        uint8_t L_size = sizeof(L);
        uint8_t O_size = sizeof(O);
        w.write(L_size);
        w.write(O_size);

        // Output the sizes and data
        w.write(n_);
        w.write(m_);
        w.write(nrows_);
        w.write(ncols_);

        size_t voff_size = sizeof(O)*(n_+1);
        size_t vend_size = sizeof(L)*m_;
        size_t w_size = detail::weight_size_<wgt, W, O>(m_);

        // Output the data
        char* voff = detail::get_raw_data_<OS>(offsets_);
        w.parallel_write(voff, voff_size);

        char* vend = detail::get_raw_data_<LS>(endpoints_);
        w.parallel_write(vend, vend_size);

        if (w_size > 0) {
            char* wend = detail::get_raw_data_<WS>(weights_);
            w.parallel_write(wend, w_size);
        }
    }

    template<class L, class O, class LS, class OS, bool wgt, class W, class WS>
    void CSR<L,O,LS,OS,wgt,W,WS>::read_bin_(File& f) {
        // Read and confirm the header
        f.read(csr_file_header);

        // Confirm the sizes
        uint8_t L_size, O_size;
        L_size = f.read<uint8_t>();
        O_size = f.read<uint8_t>();

        if (L_size != sizeof(L)) throw Error("Invalid CSR template parameters to match binary");
        if (O_size != sizeof(O)) throw Error("Invalid CSR template parameters to match binary");

        // Read the sizes
        n_ = f.read<L>();
        m_ = f.read<O>();
        nrows_ = f.read<L>();
        ncols_ = f.read<L>();

        // Allocate space
        allocate_();

        size_t voff_size = sizeof(O)*(n_+1);
        size_t vend_size = sizeof(L)*m_;

        // Read out the vectors
        char* voff = detail::get_raw_data_<OS>(offsets_);
        f.parallel_read(voff, voff_size);

        char* vend = detail::get_raw_data_<LS>(endpoints_);
        f.parallel_read(vend, vend_size);

        size_t w_size = detail::weight_size_<wgt, W, O>(m_);
        if (w_size > 0) {
            char* wend = detail::get_raw_data_<WS>(weights_);
            f.parallel_read(wend, w_size);
        }
    }

    template<class L, class O, class LS, class OS, bool wgt, class W, class WS>
    void CSR<L,O,LS,OS,wgt,W,WS>::sort() {
        #pragma omp parallel for schedule(dynamic, 10240)
        for (L v = 0; v < n_; ++v) {
            // Get the start and end range
            O start = detail::get_value_<OS, O>(offsets_, v);
            O end = detail::get_value_<OS, O>(offsets_, v+1);

            // Sort the range from the start to the end
            if (detail::if_true_<wgt>()) {
                // This should be improved, e.g., without replicating
                // everything. For now, make a joint array, sort that, and
                // then pull out the resulting data
                std::vector<std::pair<L, W>> vec;
                vec.reserve(end-start);
                for (O cur = start; cur < end; ++cur) {
                    L l = detail::get_value_<LS, L>(endpoints_, cur);
                    W w = detail::get_value_<WS, W>(weights_, cur);
                    std::pair<L, W> val = {l, w};
                    vec.emplace_back(val);
                }
                std::sort(vec.begin(), vec.end());
                O cur = start;
                for (auto& pair : vec) {
                    L l = std::get<0>(pair);
                    W w = std::get<1>(pair);
                    detail::set_value_(endpoints_, cur, l);
                    detail::set_value_(weights_, cur, w);
                    ++cur;
                }
            } else {
                L* endpoints = (L*)detail::get_raw_data_(endpoints_);

                L* range_start = endpoints+start;
                L* range_end = endpoints+end;

                std::sort(range_start, range_end);
            }
        }
    }

    template<class L, class O, class LS, class OS, bool wgt, class W, class WS>
    template<class nL, class nO, class nLS, class nOS, bool nw, class nW, class nWS>
    CSR<nL, nO, nLS, nOS, nw, nW, nWS> CSR<L,O,LS,OS,wgt,W,WS>::new_csr_without_dups() {
        // First, sort ourselves
        sort();

        // Next, count the degrees for each vertex, excluding duplicates
        std::shared_ptr<L> degs_storage;
        detail::allocate_mem_(degs_storage, n_);
        L* degs = degs_storage.get();

        nO new_m = 0;

        #pragma omp parallel for schedule(dynamic, 10240) shared(degs) reduction(+ : new_m)
        for (L v = 0; v < n_; ++v) {
            O start = detail::get_value_<OS, O>(offsets_, v);
            O end = detail::get_value_<OS, O>(offsets_, v+1);
            if (end-start == 0) {
                degs[v] = 0;
                continue;
            }

            L prev_val = detail::get_value_<LS, L>(endpoints_, start++);
            L new_deg = 1;

            while (start != end) {
                L cur_val = detail::get_value_<LS, L>(endpoints_, start++);
                if (cur_val != prev_val) {
                    prev_val = cur_val;
                    ++new_deg;
                }
            }

            degs[v] = new_deg;
            new_m += new_deg;
        }

        // Allocate the new CSR
        CSR<nL, nO, nLS, nOS, nw, nW, nWS> ret { (nL)n_, new_m, (nL)nrows_, (nL)ncols_ };

        auto& new_endpoints = ret.endpoints();
        auto& new_offsets = ret.offsets();
        auto& new_weights = ret.weights();

        // Set the offsets by doing a prefix sum on the degrees

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

        // Keep track of the starting offsets for each thread
        std::shared_ptr<O> so_storage;
        detail::allocate_mem_(so_storage, num_threads);
        O* start_offsets = so_storage.get();

        #pragma omp parallel shared(start_offsets)
        {
            #ifdef _OPENMP
            size_t tid = omp_get_thread_num();
            #else
            size_t tid = 0;
            #endif

            L v_start = (tid*n_)/num_threads;
            L v_end = ((tid+1)*n_)/num_threads;

            O my_degs = 0;
            for (L c = v_start; c < v_end; ++c) {
                // FIXME get the degs
                O this_deg = degs[c];
                my_degs += this_deg;
            }

            // Save our local degree count to do a prefix sum on
            start_offsets[tid] = my_degs;

            #pragma omp barrier
            #pragma omp single
            {
                O total_degs = 0;
                for (size_t cur_tid = 0; cur_tid < num_threads; ++cur_tid) {
                    total_degs += start_offsets[cur_tid];
                    start_offsets[cur_tid] = total_degs;
                }
            }
            #pragma omp barrier

            // Get the starting offset
            // The prefix sum array is off by one, so the start is at zero
            O cur_offset = 0;
            if (tid > 0)
                cur_offset = start_offsets[tid-1];

            // Now, assign the offsets to each label
            for (L c = v_start; c < v_end; ++c) {
                detail::set_value_(new_offsets, c, cur_offset);
                cur_offset += degs[c];
            }
            #pragma omp single
            {
                // Patch the last offset to the end, making for easier
                // degree computation and iteration
                detail::set_value_(new_offsets, (nL)n_, new_m);
            }

            // Now, all new offsets have been assigned
        }

        // Repeat going through the edges, copying out the endpoints
        #pragma omp parallel for schedule(dynamic, 10240)
        for (L v = 0; v < n_; ++v) {
            O o_start = detail::get_value_<OS, O>(offsets_, v);
            O o_end = detail::get_value_<OS, O>(offsets_, v+1);
            if (o_end-o_start == 0) continue;

            nO n_cur = detail::get_value_<nOS, nO>(new_offsets, (nL)v);

            L prev_val = detail::get_value_<LS, L>(endpoints_, o_start++);
            detail::set_value_(new_endpoints, n_cur++, prev_val);
            if (detail::if_true_<nw>()) {
                W w = detail::get_value_<WS, W>(weights_, o_start-1);
                detail::set_value_(new_weights, n_cur-1, (nW)w);
            }

            while (o_start != o_end) {
                L cur_val = detail::get_value_<LS, L>(endpoints_, o_start++);
                if (prev_val != cur_val) {
                    prev_val = cur_val;
                    detail::set_value_(new_endpoints, n_cur++, (nL)prev_val);
                    if (detail::if_true_<nw>()) {
                        W w = detail::get_value_<WS, W>(weights_, o_start-1);
                        detail::set_value_(new_weights, n_cur-1, (nW)w);
                    }
                }
            }
        }

        return ret;
    }

}
