#ifndef PIGO_STB_HPP
#define PIGO_STB_HPP

namespace pigo {

    //-=-=-=-=-=-=-=-=-=-=*-=-=-=-=-=-=-=-=-=-=*-=-=-=-=-=-=-=-=-=-=*-=-=-=-=-=-=
    // Beginning of:
    // stb_sprintf - v1.10 - public domain snprintf() implementation
    // originally by Jeff Roberts / RAD Game Tools, 2015/10/20
    // http://github.com/nothings/stb
    //
    // Modified by Kasimir Gabert for use in PIGO.
    //
    // LICENSE:
    //
    //   This part until the end of stb_sprintf v1.10 was originally published
    //   under the open domain.  This has been modified and is now part of PIGO,
    //   and included under the PIGO license and software.

    #define stbsp__uint32 unsigned int
    #define stbsp__int32 signed int

    #define stbsp__uint64 unsigned long long
    #define stbsp__int64 signed long long
    #define stbsp__uint16 unsigned short

    #ifndef stbsp__uintptr
    #if defined(__ppc64__) || defined(__powerpc64__) || defined(__aarch64__) || defined(_M_X64) || defined(__x86_64__) || defined(__x86_64) || defined(__s390x__)
    #define stbsp__uintptr stbsp__uint64
    #else
    #define stbsp__uintptr stbsp__uint32
    #endif
    #endif

    #define STBSP__UNALIGNED(code) code

    // internal float utility functions
    inline stbsp__int32 stbsp__real_to_str(char const **start, stbsp__uint32 *len, char *out, stbsp__int32 *decimal_pos, double value, stbsp__uint32 frac_digits);
    #define STBSP__SPECIAL 0x7000

    static char stbsp__period = '.';
    static char stbsp__comma = ',';
    static struct
    {
       short temp; // force next field to be 2-byte aligned
       char pair[201];
    } stbsp__digitpair =
    {
      0,
       "00010203040506070809101112131415161718192021222324"
       "25262728293031323334353637383940414243444546474849"
       "50515253545556575859606162636465666768697071727374"
       "75767778798081828384858687888990919293949596979899"
    };

    #define STBSP__LEADINGPLUS 2
    #define STBSP__LEADINGSPACE 4
    #define STBSP__TRIPLET_COMMA 64
    #define STBSP__NEGATIVE 128

    inline void stbsp__lead_sign(stbsp__uint32 fl, char *sign)
    {
       sign[0] = 0;
       if (fl & STBSP__NEGATIVE) {
          sign[0] = 1;
          sign[1] = '-';
       } else if (fl & STBSP__LEADINGSPACE) {
          sign[0] = 1;
          sign[1] = ' ';
       } else if (fl & STBSP__LEADINGPLUS) {
          sign[0] = 1;
          sign[1] = '+';
       }
    }

    inline size_t stbsp_to_chars(char* buf, double fv) {

       char *bf = buf;

       {
          stbsp__int32 fw, pr, tz;
          stbsp__uint32 fl;

          #define stbsp__cb_buf_clamp(cl, v)                \
             cl = v;


          // ok, we have a percent, read the modifiers first
          fw = 0;
          pr = 6;
          fl = 0;
          tz = 0;

          // handle each replacement
          {
             #define STBSP__NUMSZ 512 // big enough for e308 (with commas) or e-307
             char num[STBSP__NUMSZ];
             char lead[8];
             char tail[8];
             char *s;
             stbsp__uint32 l, n, cs;
             stbsp__int32 dp;
             char const *sn;

             // read the double into a string
             if (stbsp__real_to_str(&sn, &l, num, &dp, fv, pr))
                fl |= STBSP__NEGATIVE;
             tail[0] = 0;
             stbsp__lead_sign(fl, lead);
             if (dp == STBSP__SPECIAL) {
                s = (char *)sn;
                cs = 0;
                pr = 0;
                goto scopy;
             }
             s = num + 64;

             // handle the three decimal varieties
             if (dp <= 0) {
                stbsp__int32 i;
                // handle 0.000*000xxxx
                *s++ = '0';
                if (pr)
                   *s++ = stbsp__period;
                n = -dp;
                if ((stbsp__int32)n > pr)
                   n = pr;
                i = n;
                while (i) {
                   if ((((stbsp__uintptr)s) & 3) == 0)
                      break;
                   *s++ = '0';
                   --i;
                }
                while (i >= 4) {
                   *(stbsp__uint32 *)s = 0x30303030;
                   s += 4;
                   i -= 4;
                }
                while (i) {
                   *s++ = '0';
                   --i;
                }
                if ((stbsp__int32)(l + n) > pr)
                   l = pr - n;
                i = l;
                while (i) {
                   *s++ = *sn++;
                   --i;
                }
                tz = pr - (n + l);
                cs = 1 + (3 << 24); // how many tens did we write (for commas below)
             } else {
                cs = (fl & STBSP__TRIPLET_COMMA) ? ((600 - (stbsp__uint32)dp) % 3) : 0;
                if ((stbsp__uint32)dp >= l) {
                   // handle xxxx000*000.0
                   n = 0;
                   for (;;) {
                      if ((fl & STBSP__TRIPLET_COMMA) && (++cs == 4)) {
                         cs = 0;
                         *s++ = stbsp__comma;
                      } else {
                         *s++ = sn[n];
                         ++n;
                         if (n >= l)
                            break;
                      }
                   }
                   if (n < (stbsp__uint32)dp) {
                      n = dp - n;
                      if ((fl & STBSP__TRIPLET_COMMA) == 0) {
                         while (n) {
                            if ((((stbsp__uintptr)s) & 3) == 0)
                               break;
                            *s++ = '0';
                            --n;
                         }
                         while (n >= 4) {
                            *(stbsp__uint32 *)s = 0x30303030;
                            s += 4;
                            n -= 4;
                         }
                      }
                      while (n) {
                         if ((fl & STBSP__TRIPLET_COMMA) && (++cs == 4)) {
                            cs = 0;
                            *s++ = stbsp__comma;
                         } else {
                            *s++ = '0';
                            --n;
                         }
                      }
                   }
                   cs = (int)(s - (num + 64)) + (3 << 24); // cs is how many tens
                   if (pr) {
                      *s++ = stbsp__period;
                      tz = pr;
                   }
                } else {
                   // handle xxxxx.xxxx000*000
                   n = 0;
                   for (;;) {
                      if ((fl & STBSP__TRIPLET_COMMA) && (++cs == 4)) {
                         cs = 0;
                         *s++ = stbsp__comma;
                      } else {
                         *s++ = sn[n];
                         ++n;
                         if (n >= (stbsp__uint32)dp)
                            break;
                      }
                   }
                   cs = (int)(s - (num + 64)) + (3 << 24); // cs is how many tens
                   if (pr)
                      *s++ = stbsp__period;
                   if ((l - dp) > (stbsp__uint32)pr)
                      l = pr + dp;
                   while (n < l) {
                      *s++ = sn[n];
                      ++n;
                   }
                   tz = pr - (l - dp);
                }
             }
             pr = 0;

             // get the length that we copied
             l = (stbsp__uint32)(s - (num + 64));
             s = num + 64;
             goto scopy;

          scopy:
             // get fw=leading/trailing space, pr=leading zeros
             if (pr < (stbsp__int32)l)
                pr = l;
             n = pr + lead[0] + tail[0] + tz;
             if (fw < (stbsp__int32)n)
                fw = n;
             fw -= n;
             pr -= l;

             // copy leader if there is still one
             sn = lead + 1;
             while (lead[0]) {
                stbsp__int32 i;
                stbsp__cb_buf_clamp(i, lead[0]);
                lead[0] -= (char)i;
                while (i) {
                   *bf++ = *sn++;
                   --i;
                }
             }

             // copy the string
             n = l;
             while (n) {
                stbsp__int32 i;
                stbsp__cb_buf_clamp(i, n);
                n -= i;
                STBSP__UNALIGNED(while (i >= 4) {
                   *(stbsp__uint32 volatile *)bf = *(stbsp__uint32 volatile *)s;
                   bf += 4;
                   s += 4;
                   i -= 4;
                })
                while (i) {
                   *bf++ = *s++;
                   --i;
                }
             }

             // copy trailing zeros
             while (tz) {
                stbsp__int32 i;
                stbsp__cb_buf_clamp(i, tz);
                tz -= i;
                while (i) {
                   if ((((stbsp__uintptr)bf) & 3) == 0)
                      break;
                   *bf++ = '0';
                   --i;
                }
                while (i >= 4) {
                   *(stbsp__uint32 *)bf = 0x30303030;
                   bf += 4;
                   i -= 4;
                }
                while (i) {
                   *bf++ = '0';
                   --i;
                }
             }

             // copy tail if there is one
             sn = tail + 1;
             while (tail[0]) {
                stbsp__int32 i;
                stbsp__cb_buf_clamp(i, tail[0]);
                tail[0] -= (char)i;
                while (i) {
                   *bf++ = *sn++;
                   --i;
                }
             }

          }
       }

        *bf = 0;

       return (int)(bf - buf);
    }

    // cleanup
    #undef STBSP__LEADINGPLUS
    #undef STBSP__LEADINGSPACE
    #undef STBSP__TRIPLET_COMMA
    #undef STBSP__NEGATIVE
    #undef STBSP__NUMSZ
    #undef stbsp__cb_buf_clamp

    // =======================================================================
    //   low level float utility functions

    // copies d to bits w/ strict aliasing (this compiles to nothing on /Ox)
    #define STBSP__COPYFP(dest, src)                   \
       {                                               \
          int cn;                                      \
          for (cn = 0; cn < 8; cn++)                   \
             ((char *)&dest)[cn] = ((char *)&src)[cn]; \
       }

    static double const stbsp__bot[23] = {
       1e+000, 1e+001, 1e+002, 1e+003, 1e+004, 1e+005, 1e+006, 1e+007, 1e+008, 1e+009, 1e+010, 1e+011,
       1e+012, 1e+013, 1e+014, 1e+015, 1e+016, 1e+017, 1e+018, 1e+019, 1e+020, 1e+021, 1e+022
    };
    static double const stbsp__negbot[22] = {
       1e-001, 1e-002, 1e-003, 1e-004, 1e-005, 1e-006, 1e-007, 1e-008, 1e-009, 1e-010, 1e-011,
       1e-012, 1e-013, 1e-014, 1e-015, 1e-016, 1e-017, 1e-018, 1e-019, 1e-020, 1e-021, 1e-022
    };
    static double const stbsp__negboterr[22] = {
       -5.551115123125783e-018,  -2.0816681711721684e-019, -2.0816681711721686e-020, -4.7921736023859299e-021, -8.1803053914031305e-022, 4.5251888174113741e-023,
       4.5251888174113739e-024,  -2.0922560830128471e-025, -6.2281591457779853e-026, -3.6432197315497743e-027, 6.0503030718060191e-028,  2.0113352370744385e-029,
       -3.0373745563400371e-030, 1.1806906454401013e-032,  -7.7705399876661076e-032, 2.0902213275965398e-033,  -7.1542424054621921e-034, -7.1542424054621926e-035,
       2.4754073164739869e-036,  5.4846728545790429e-037,  9.2462547772103625e-038,  -4.8596774326570872e-039
    };
    static double const stbsp__top[13] = {
       1e+023, 1e+046, 1e+069, 1e+092, 1e+115, 1e+138, 1e+161, 1e+184, 1e+207, 1e+230, 1e+253, 1e+276, 1e+299
    };
    static double const stbsp__negtop[13] = {
       1e-023, 1e-046, 1e-069, 1e-092, 1e-115, 1e-138, 1e-161, 1e-184, 1e-207, 1e-230, 1e-253, 1e-276, 1e-299
    };
    static double const stbsp__toperr[13] = {
       8388608,
       6.8601809640529717e+028,
       -7.253143638152921e+052,
       -4.3377296974619174e+075,
       -1.5559416129466825e+098,
       -3.2841562489204913e+121,
       -3.7745893248228135e+144,
       -1.7356668416969134e+167,
       -3.8893577551088374e+190,
       -9.9566444326005119e+213,
       6.3641293062232429e+236,
       -5.2069140800249813e+259,
       -5.2504760255204387e+282
    };
    static double const stbsp__negtoperr[13] = {
       3.9565301985100693e-040,  -2.299904345391321e-063,  3.6506201437945798e-086,  1.1875228833981544e-109,
       -5.0644902316928607e-132, -6.7156837247865426e-155, -2.812077463003139e-178,  -5.7778912386589953e-201,
       7.4997100559334532e-224,  -4.6439668915134491e-247, -6.3691100762962136e-270, -9.436808465446358e-293,
       8.0970921678014997e-317
    };

    static stbsp__uint64 const stbsp__powten[20] = {
       1,
       10,
       100,
       1000,
       10000,
       100000,
       1000000,
       10000000,
       100000000,
       1000000000,
       10000000000ULL,
       100000000000ULL,
       1000000000000ULL,
       10000000000000ULL,
       100000000000000ULL,
       1000000000000000ULL,
       10000000000000000ULL,
       100000000000000000ULL,
       1000000000000000000ULL,
       10000000000000000000ULL
    };
    #define stbsp__tento19th (1000000000000000000ULL)

    #define stbsp__ddmulthi(oh, ol, xh, yh)                            \
       {                                                               \
          double ahi = 0, alo, bhi = 0, blo;                           \
          stbsp__int64 bt;                                             \
          oh = xh * yh;                                                \
          STBSP__COPYFP(bt, xh);                                       \
          bt &= ((~(stbsp__uint64)0) << 27);                           \
          STBSP__COPYFP(ahi, bt);                                      \
          alo = xh - ahi;                                              \
          STBSP__COPYFP(bt, yh);                                       \
          bt &= ((~(stbsp__uint64)0) << 27);                           \
          STBSP__COPYFP(bhi, bt);                                      \
          blo = yh - bhi;                                              \
          ol = ((ahi * bhi - oh) + ahi * blo + alo * bhi) + alo * blo; \
       }

    #define stbsp__ddtoS64(ob, xh, xl)          \
       {                                        \
          double ahi = 0, alo, vh, t;           \
          ob = (stbsp__int64)xh;                \
          vh = (double)ob;                      \
          ahi = (xh - vh);                      \
          t = (ahi - xh);                       \
          alo = (xh - (ahi - t)) - (vh + t);    \
          ob += (stbsp__int64)(ahi + alo + xl); \
       }

    #define stbsp__ddrenorm(oh, ol) \
       {                            \
          double s;                 \
          s = oh + ol;              \
          ol = ol - (s - oh);       \
          oh = s;                   \
       }

    #define stbsp__ddmultlo(oh, ol, xh, xl, yh, yl) ol = ol + (xh * yl + xl * yh);

    #define stbsp__ddmultlos(oh, ol, xh, yl) ol = ol + (xh * yl);

    static void stbsp__raise_to_power10(double *ohi, double *olo, double d, stbsp__int32 power) // power can be -323 to +350
    {
       double ph, pl;
       if ((power >= 0) && (power <= 22)) {
          stbsp__ddmulthi(ph, pl, d, stbsp__bot[power]);
       } else {
          stbsp__int32 e, et, eb;
          double p2h, p2l;

          e = power;
          if (power < 0)
             e = -e;
          et = (e * 0x2c9) >> 14; /* %23 */
          if (et > 13)
             et = 13;
          eb = e - (et * 23);

          ph = d;
          pl = 0.0;
          if (power < 0) {
             if (eb) {
                --eb;
                stbsp__ddmulthi(ph, pl, d, stbsp__negbot[eb]);
                stbsp__ddmultlos(ph, pl, d, stbsp__negboterr[eb]);
             }
             if (et) {
                stbsp__ddrenorm(ph, pl);
                --et;
                stbsp__ddmulthi(p2h, p2l, ph, stbsp__negtop[et]);
                stbsp__ddmultlo(p2h, p2l, ph, pl, stbsp__negtop[et], stbsp__negtoperr[et]);
                ph = p2h;
                pl = p2l;
             }
          } else {
             if (eb) {
                e = eb;
                if (eb > 22)
                   eb = 22;
                e -= eb;
                stbsp__ddmulthi(ph, pl, d, stbsp__bot[eb]);
                if (e) {
                   stbsp__ddrenorm(ph, pl);
                   stbsp__ddmulthi(p2h, p2l, ph, stbsp__bot[e]);
                   stbsp__ddmultlos(p2h, p2l, stbsp__bot[e], pl);
                   ph = p2h;
                   pl = p2l;
                }
             }
             if (et) {
                stbsp__ddrenorm(ph, pl);
                --et;
                stbsp__ddmulthi(p2h, p2l, ph, stbsp__top[et]);
                stbsp__ddmultlo(p2h, p2l, ph, pl, stbsp__top[et], stbsp__toperr[et]);
                ph = p2h;
                pl = p2l;
             }
          }
       }
       stbsp__ddrenorm(ph, pl);
       *ohi = ph;
       *olo = pl;
    }

    // given a float value, returns the significant bits in bits, and the position of the
    //   decimal point in decimal_pos.  +/-INF and NAN are specified by special values
    //   returned in the decimal_pos parameter.
    // frac_digits is absolute normally, but if you want from first significant digits (got %g and %e), or in 0x80000000
    inline stbsp__int32 stbsp__real_to_str(char const **start, stbsp__uint32 *len, char *out, stbsp__int32 *decimal_pos, double value, stbsp__uint32 frac_digits)
    {
       double d;
       stbsp__int64 bits = 0;
       stbsp__int32 expo, e, ng, tens;

       d = value;
       STBSP__COPYFP(bits, d);
       expo = (stbsp__int32)((bits >> 52) & 2047);
       ng = (stbsp__int32)((stbsp__uint64) bits >> 63);
       if (ng)
          d = -d;

       if (expo == 2047) // is nan or inf?
       {
          *start = (bits & ((((stbsp__uint64)1) << 52) - 1)) ? "NaN" : "Inf";
          *decimal_pos = STBSP__SPECIAL;
          *len = 3;
          return ng;
       }

       if (expo == 0) // is zero or denormal
       {
          if (((stbsp__uint64) bits << 1) == 0) // do zero
          {
             *decimal_pos = 1;
             *start = out;
             out[0] = '0';
             *len = 1;
             return ng;
          }
          // find the right expo for denormals
          {
             stbsp__int64 v = ((stbsp__uint64)1) << 51;
             while ((bits & v) == 0) {
                --expo;
                v >>= 1;
             }
          }
       }

       // find the decimal exponent as well as the decimal bits of the value
       {
          double ph, pl;

          // log10 estimate - very specifically tweaked to hit or undershoot by no more than 1 of log10 of all expos 1..2046
          tens = expo - 1023;
          tens = (tens < 0) ? ((tens * 617) / 2048) : (((tens * 1233) / 4096) + 1);

          // move the significant bits into position and stick them into an int
          stbsp__raise_to_power10(&ph, &pl, d, 18 - tens);

          // get full as much precision from double-double as possible
          stbsp__ddtoS64(bits, ph, pl);

          // check if we undershot
          if (((stbsp__uint64)bits) >= stbsp__tento19th)
             ++tens;
       }

       // now do the rounding in integer land
       frac_digits = (frac_digits & 0x80000000) ? ((frac_digits & 0x7ffffff) + 1) : (tens + frac_digits);
       if ((frac_digits < 24)) {
          stbsp__uint32 dg = 1;
          if ((stbsp__uint64)bits >= stbsp__powten[9])
             dg = 10;
          while ((stbsp__uint64)bits >= stbsp__powten[dg]) {
             ++dg;
             if (dg == 20)
                goto noround;
          }
          if (frac_digits < dg) {
             stbsp__uint64 r;
             // add 0.5 at the right position and round
             e = dg - frac_digits;
             if ((stbsp__uint32)e >= 24)
                goto noround;
             r = stbsp__powten[e];
             bits = bits + (r / 2);
             if ((stbsp__uint64)bits >= stbsp__powten[dg])
                ++tens;
             bits /= r;
          }
       noround:;
       }

       // kill long trailing runs of zeros
       if (bits) {
          stbsp__uint32 n;
          for (;;) {
             if (bits <= 0xffffffff)
                break;
             if (bits % 1000)
                goto donez;
             bits /= 1000;
          }
          n = (stbsp__uint32)bits;
          while ((n % 1000) == 0)
             n /= 1000;
          bits = n;
       donez:;
       }

       // convert to string
       out += 64;
       e = 0;
       for (;;) {
          stbsp__uint32 n;
          char *o = out - 8;
          // do the conversion in chunks of U32s (avoid most 64-bit divides, worth it, constant denomiators be damned)
          if (bits >= 100000000) {
             n = (stbsp__uint32)(bits % 100000000);
             bits /= 100000000;
          } else {
             n = (stbsp__uint32)bits;
             bits = 0;
          }
          while (n) {
             out -= 2;
             *(stbsp__uint16 *)out = *(stbsp__uint16 *)&stbsp__digitpair.pair[(n % 100) * 2];
             n /= 100;
             e += 2;
          }
          if (bits == 0) {
             if ((e) && (out[0] == '0')) {
                ++out;
                --e;
             }
             break;
          }
          while (out != o) {
             *--out = '0';
             ++e;
          }
       }

       *decimal_pos = tens;
       *start = out;
       *len = e;
       return ng;
    }

    #undef stbsp__ddmulthi
    #undef stbsp__ddrenorm
    #undef stbsp__ddmultlo
    #undef stbsp__ddmultlos
    #undef STBSP__SPECIAL
    #undef STBSP__COPYFP

    // clean up
    #undef stbsp__uint16
    #undef stbsp__uint32
    #undef stbsp__int32
    #undef stbsp__uint64
    #undef stbsp__int64
    #undef STBSP__UNALIGNED

    // End of:
    // stb_sprintf - v1.10 - public domain snprintf() implementation
    //-=-=-=-=-=-=-=-=-=-=*-=-=-=-=-=-=-=-=-=-=*-=-=-=-=-=-=-=-=-=-=*-=-=-=-=-=-=

}

#endif
