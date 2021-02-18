/**
 * PIGO: a parallel graph and matrix I/O and preprocessing library
 * Copyright (c) 2021 GT-TDALab
 */

#include <atomic>
#include <vector>
#include <string>

namespace pigo {
    template<class L, class O, class LS, class OS>
    CSR<L,O,LS,OS>::CSR(std::string fn) : CSR(fn, AUTO) { }

    template<class L, class O, class LS, class OS>
    CSR<L,O,LS,OS>::CSR(std::string fn, FileType ft) {
        // Open the file for reading
        ROFile f {fn};
        read_(f, ft);
    }

    template<class L, class O, class LS, class OS>
    CSR<L,O,LS,OS>::CSR(File& f, FileType ft) {
        read_(f, ft);
    }

    template<class L, class O, class LS, class OS>
    void CSR<L,O,LS,OS>::read_(File& f, FileType ft) {
        FileType ft_used = ft;
        // If the file type is AUTO, then try to detect it
        if (ft_used == AUTO) {
            ft_used = f.guess_file_type();
        }

        if (ft_used == MATRIX_MARKET || ft_used == EDGE_LIST ||
                ft_used == PIGO_COO_BIN) {
            // First build a COO, then load here
            COO<L,O,L*> coo { f, ft_used };
            convert_coo_(coo);
            coo.free();
        } else if (ft_used == PIGO_CSR_BIN) {
            read_bin_(f);
        } else if (ft_used == GRAPH) {
            FileReader r = f.reader();
            read_graph_(r);
        } else
            throw NotYetImplemented("This file type is not yet supported");
    }

    template<class L, class O, class LS, class OS>
    void CSR<L,O,LS,OS>::allocate_() {
        allocate_mem_<LS>(endpoints_, m_);
        allocate_mem_<OS>(offsets_, n_+1);
    }

    template<class L, class O, class LS, class OS> template<class COOStorage>
    CSR<L,O,LS,OS>::CSR(COO<L,O,COOStorage> &coo) {
        convert_coo_(coo);
    }

    template<class L, class O, class LS, class OS> template<class COOStorage>
    void CSR<L,O,LS,OS>::convert_coo_(COO<L, O, COOStorage>& coo) {
        // Set the sizes first
        n_ = coo.n();
        m_ = coo.m();

        // Allocate the offsets and endpoints
        allocate_();

        // This is a multi pass algorithm.
        // First, we need to count each vertex's degree and allocate the
        // space appropriately.
        // Next, we go through the degrees and change them to offsets
        // Finally, we need to go through the COO and copy memory

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
            size_t tid = omp_get_thread_num();

            L v_start = (tid*n_)/num_threads;
            L v_end = ((tid+1)*n_)/num_threads;
            // We need to initialize degrees to count for zero-degree
            // vertices
            #pragma omp for
            for (L v = 0; v < n_; ++v)
                all_degs[v] = 0;

            auto coo_x = coo.x();
            auto coo_y = coo.y();

            #pragma omp for
            for (O x_id = 0; x_id < m_; ++x_id) {
                size_t deg_inc = coo_x[x_id];
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
                set_value_(offsets_, c, cur_offset);
                cur_offset += label_degs[c];
            }
            #pragma omp single
            {
                // Patch the last offset to the end, making for easier
                // degree computation and iteration
                set_value_(offsets_, n_, m_);
            }

            #pragma omp barrier
            // Now, all offsets_ have been assigned

            // Here, we use the degrees computed earlier and treat them
            // instead as the remaining vertices, showing the current copy
            // position

            // Finally, copy over the actual endpoints
            #pragma omp for
            for (O coo_pos = 0; coo_pos < m_; ++coo_pos) {
                L src = coo_x[coo_pos];
                L dst = coo_y[coo_pos];

                O this_offset_pos;
                #pragma omp atomic capture
                {
                    this_offset_pos = label_degs[src];
                    label_degs[src]--;
                }
                O this_offset = get_value_<OS, O>(offsets_, src+1) - this_offset_pos;
                set_value_(endpoints_, this_offset, dst);
            }

        }

        delete [] label_degs;
        delete [] start_offsets;
        delete [] all_degs;

    }

    template<class L, class O, class LS, class OS>
    void CSR<L,O,LS,OS>::read_graph_(FileReader &r) {
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
        #pragma omp parallel
        {
            size_t tid = omp_get_thread_num();

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
            while (rs_p1.good()) {
                if (rs_p1.at_nl_or_eol())
                    ++tid_nls;
                else
                    ++tid_ints;
                rs_p1.move_to_next_int_or_nl();
            }

            nl_offsets[tid] = tid_nls;
            int_offsets[tid] = tid_ints;

            // Compute a prefix sum on the offsets
            #pragma omp barrier
            #pragma omp single
            {
                size_t sum_nl = 0;
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
                allocate_();
                set_value_(offsets_, 0, 0);
                set_value_(offsets_, n_, m_);
            }
            #pragma omp barrier

            // Pass 2: iterate through again, but now copy out the values
            // to the appropriate position in the endpoints / offsets
            FileReader rs_p2 = rs;
            O endpoint_pos = 0;
            L offset_pos = 0;
            if (tid > 0) {
                offset_pos = nl_offsets[tid-1];
                endpoint_pos = int_offsets[tid-1];
            }

            while (rs_p2.good()) {
                // Ignore any trailing data in the file
                if (offset_pos >= n_) break;

                // Copy and set the endpoint
                if (rs_p2.at_nl_or_eol()) {
                    // Set the offset to the current endpoint position
                    set_value_(offsets_, ++offset_pos, endpoint_pos);
                    rs_p2.move_to_next_int_or_nl();
                } else {
                    // Set the actual value
                    L endpoint = rs_p2.read_int<L>();
                    set_value_(endpoints_, endpoint_pos++, endpoint);
                    if (!rs_p2.at_nl_or_eol())
                        rs_p2.move_to_next_int_or_nl();
                }
            }
        }

        if (m_ == 2*read_m) {}
        else if (m_ != read_m) throw Error("Mismatch in CSR nonzeros and header");
        if (n_ < read_n) throw Error("Mismatch in CSR newlines and header");
        else n_ = read_n;

    }

    template<class L, class O, class LS, class OS>
    void CSR<L,O,LS,OS>::save(std::string fn) {
        // Before creating the file, we need to find the size
        size_t out_size = 0;
        std::string cfh { csr_file_header };
        out_size += cfh.size();
        // Find the template sizes
        out_size += sizeof(uint8_t)*2;
        // Find the size of the size of the CSR
        out_size += sizeof(L)+sizeof(O);
        // Finally, find the actual CSR size
        size_t voff_size = sizeof(O)*(n_+1);
        size_t vend_size = sizeof(L)*m_;
        out_size += voff_size + vend_size;

        // Create the output file
        WFile w {fn, out_size};

        // Output the file header
        w.write(cfh);

        // Output the template sizes
        uint8_t L_size = sizeof(L);
        uint8_t O_size = sizeof(O);
        w.write(L_size);
        w.write(O_size);

        // Output the sizes and data
        w.write(n_);
        w.write(m_);

        // Output the data
        char* voff = get_raw_data_<OS>(offsets_);
        w.parallel_write(voff, voff_size);

        char* vend = get_raw_data_<LS>(endpoints_);
        w.parallel_write(vend, vend_size);
    }

    template<class L, class O, class LS, class OS>
    void CSR<L,O,LS,OS>::read_bin_(File& f) {
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

        // Allocate space
        allocate_();

        size_t voff_size = sizeof(O)*(n_+1);
        size_t vend_size = sizeof(L)*m_;

        // Read out the vectors
        char* voff = get_raw_data_<OS>(offsets_);
        f.parallel_read(voff, voff_size);

        char* vend = get_raw_data_<LS>(endpoints_);
        f.parallel_read(vend, vend_size);
    }

}
