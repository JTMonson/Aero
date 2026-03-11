// Control_filter.h
#ifndef CONTROL_FILTER_H
#define CONTROL_FILTER_H

#define FILTER_LEN 4    // Adaptive filter length
#define SEC_LEN 4       // Secondary path length
#define MU 0.0001f            // Step size

/*
    w(n+1) = w(n) - mu * e(n) * xf(n)
    y(n) = sum( w(n) * x(n) )
*/

class Control_filter {
public:
    // ======== Properties ========
    float Wn[FILTER_LEN] = {};      // Vecter of filter coefficients w(n)

    float En;                       // Error signal at step n where En = Dn - Yn
    float Dn;                       // Reference signal to cancel measured at error mic
    float Yn;                       // Current anti-noise output

    float Xfn[FILTER_LEN] = {};     // Filtered reference signal: Xfn = Xn * Sn
    float Xn[FILTER_LEN] = {};      // Reference signal to cancel measured at source
    float Sn[SEC_LEN] = {};         // How speaker sound is changed before reaching err mic
                                    // - sec matric?

    // ======== Methods ========
    Control_filter();               // Constructor

    // FxLMS methods
    void update_Xfn();              // Filter reference through secondary path
    void compute_Yn();              // Compute anti-noise output
    void compute_error();           // Compute error at error mic
    void update_Wn();               // Update filter coefficients
    void step();                    // Run one step of FxLMS
};

#endif // CONTROL_FILTER_H