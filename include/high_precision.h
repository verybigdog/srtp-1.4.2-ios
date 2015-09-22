#ifndef __HIGH_PRECISION_H__
#define __HIGH_PRECISION_H__

/**
 * @def xrtp_highprec_div
 * xrtp_highprec_div(a1, a2) - High Precision Division
 * Divide a2 from a1, and store the result in a1.
 */
#ifndef xrtp_highprec_div
#   define xrtp_highprec_div(a1,a2)   (a1 = a1 / a2)
#endif

/**
 * @def xrtp_highprec_mod
 * xrtp_highprec_mod(a1, a2) - High Precision Modulus
 * Get the modulus a2 from a1, and store the result in a1.
 */
#ifndef xrtp_highprec_mod
#   define xrtp_highprec_mod(a1,a2)   (a1 = a1 % a2)
#endif

#endif	/* __HIGH_PRECISION_H__ */
