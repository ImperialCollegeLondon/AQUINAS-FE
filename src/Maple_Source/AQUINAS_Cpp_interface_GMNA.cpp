// N.B. compile with MinGW64 on Windows as: mex CXXFLAGS="-O3 -fopenmp -std=c++17" LDOPTIMFLAGS="$LDOPTIMFLAGS -fopenmp" -IC:\boost_1_75_0 AQUINAS_Cpp_interface_GMNA.cpp
// N.B. compile with gcc on Linux as: mex CXXFLAGS="-O3 -fopenmp -fPIC -std=c++17" LDOPTIMFLAGS="$LDOPTIMFLAGS -fopenmp" -lboost_system AQUINAS_Cpp_interface_GMNA.cpp
// AQUINAS is distributed under a BSD 3-Clause License, included at the end of this file.
#define _USE_MATH_DEFINES
#include <iostream>
#include <unordered_map>
#include <omp.h>
#include <math.h>
#include "mex.hpp"
#include "mexAdapter.hpp"
#include <boost/math/quadrature/gauss_kronrod.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/matrix_proxy.hpp>
#include <boost/numeric/ublas/lu.hpp>
#include <boost/numeric/ublas/io.hpp>
#include "AQUINAS_Cpp_Classes.hpp"
#include "AQUINAS_MEX_BDTB_GN0G_matrix.hpp"
#include "AQUINAS_MEX_B0DTB0_matrix.hpp"

namespace bnu = boost::numeric::ublas;
matlab::data::ArrayFactory m_Factory;
#pragma omp declare reduction(+ : matlab::data::TypedArray<double> : \
	std::transform(omp_in.begin(), omp_in.end(), \
				   omp_out.begin(), omp_out.begin(), \
				   std::plus<double>() ) ) \
	initializer (omp_priv(omp_orig))

//////////////////
// MEX Function //
//////////////////
class MexFunction : public matlab::mex::Function
{
public:
    void operator()(matlab::mex::ArgumentList outputs, matlab::mex::ArgumentList inputs)
    {
        int nElements = std::move(inputs[0][0]); // No. elements in the segment
        matlab::data::TypedArray<double> daSegNode_r = std::move(inputs[1]); // Array of nodal r coordinates
        matlab::data::TypedArray<double> daSegNode_z = std::move(inputs[2]); // Array of nodal z coordinates
        unsigned int nDP = std::move(inputs[3][0]); // No of Distributed Pressures that are applied on the current segment
        matlab::data::TypedArray<unsigned int> daDPtype = std::move(inputs[4]); // Integer corresponding to the direction of the Distributed Pressures that are applied on the current segment
        matlab::data::TypedArray<double> daDPmagnitudes = std::move(inputs[5]); // Array holding the magnitudes of the Distributed Pressures that are applied on the current segment, evaluated at the nodes
        unsigned int thickShell = std::move(inputs[6][0]); // Inclusion of through thickness shear strains for thick shell theory
        double dThickness = std::move(inputs[7][0]); // Segment thickness
        double dnu = std::move(inputs[8][0]); // Poisson Ratio
        double dE = std::move(inputs[8][1]); // Elastic stiffness modulus
        double dG = std::move(inputs[8][2]); // Shear stiffness modulus
        unsigned int unNoMatCurvePoints = std::move(inputs[9][0]); // No. of Gauss quadrature integration stations
        matlab::data::TypedArray<double> daeps = std::move(inputs[10]); // Array holding the plastic strains for the true stress - strain curve of the material
        matlab::data::TypedArray<double> dasys = std::move(inputs[11]); // Array holding the yield stresses for the true stress - strain curve of the material
        unsigned int unNoGaussStations = std::move(inputs[12][0]); // No. of Gauss quadrature integration stations
        unsigned int unNoSimpsonStations = std::move(inputs[13][0]); // No. of stations for through thickness integration according to Simpson's 1/3 rule
        unsigned int unElCounter = std::move(inputs[14][0]); // Element counter at start of Segment Object
        matlab::data::TypedArray<double> daElDOFs = std::move(inputs[15]); // Incoming element dof data
        matlab::data::TypedArray<double> daSIGMAS = std::move(inputs[16]); // Incoming stress resultant data
        matlab::data::TypedArray<double> daSigmas = std::move(inputs[17]); // Incoming stresses per element, per Gauss point and per Simpson station
        matlab::data::TypedArray<double> daSigma_ys = std::move(inputs[18]); // Incoming yield stresses, for comparison with the von Mises criterion, per element, per Gauss point and per Simpson station
        matlab::data::TypedArray<double> daEpns = std::move(inputs[19]); // Incoming equivalent plastic strains at material points, per element, per Gauss point and per Simpson station
        unsigned int include_nonlinear_G = std::move(inputs[20][0]); // Option to include or not the nonlinear strains in formulating the material stiffness matrix
        unsigned int include_nonlinear_M = std::move(inputs[21][0]); // Option to include or not the nonlinear tangent modulus matrices in formulating the material stiffness matrix
        matlab::data::TypedArray<double> daMTV = std::move(inputs[22]); // Matrix stiffness coefficients in vector form
        matlab::data::TypedArray<double> daVCV = std::move(inputs[23]); // Nodal load vector coefficients
        matlab::data::TypedArray<double> daOStorage = std::move(inputs[24]); // Storage matrix for pre-computed variables per element
        matlab::data::TypedArray<unsigned int> daOffset_vals = std::move(inputs[25]); // Offset values
        unsigned int unNoThreads = std::move(inputs[26][0]); // No. of OpenMP threads

        std::unordered_map<std::string, unsigned int> umOffsets; // Hash table for segment offsets
        umOffsets["D0"] = daOffset_vals[0];
        umOffsets["U1"] = daOffset_vals[1];
        umOffsets["L1"] = daOffset_vals[2];
        umOffsets["U2"] = daOffset_vals[3];
        umOffsets["L2"] = daOffset_vals[4];
        umOffsets["U3"] = daOffset_vals[5];
        umOffsets["L3"] = daOffset_vals[6];
        umOffsets["U4"] = daOffset_vals[7];
        umOffsets["L4"] = daOffset_vals[8];
        umOffsets["U5"] = daOffset_vals[9];
        umOffsets["L5"] = daOffset_vals[10];
        umOffsets["U6"] = daOffset_vals[11];
        umOffsets["L6"] = daOffset_vals[12];
        umOffsets["U7"] = daOffset_vals[13];
        umOffsets["L7"] = daOffset_vals[14];
        umOffsets["TT"] = daOffset_vals[15];

        cSegment* p_cSegment = NULL; // Pointer to a cSegment Base Class Object
        std::vector<cDistributedPressure*> cDistributedPressures; // Vector of cDistributedPressure Class Objects
        std::vector<double> dvGauss_Nodes, dvGauss_Weights, dvGauss_NodesForPressure, dvGauss_WeightsForPressure; // Vectors of nodes and weights for Gaussian quadrature, both for stiffness and distributed pressures numerical integration

        // Create Distributed Pressure Object, Gauss weights, build DT matrix and create storage containers for uncondensed K and F
        for (int dp = 0; dp < nDP; ++dp)
        {
            cDistributedPressures.push_back(new cDistributedPressure(daDPtype[dp])) ;
        }
        cMaterial* p_cMaterial = new cMaterial(0, dE, dnu, dG, unNoMatCurvePoints, dasys, daeps, 0, 0); // Pointer to a cMaterial object
        this->obtain_Gauss_Legendre_nodes_weights(unNoGaussStations, dvGauss_Nodes, dvGauss_Weights);
        this->obtain_Gauss_Legendre_nodes_weights(unNoGaussStations+1, dvGauss_NodesForPressure, dvGauss_WeightsForPressure);

        matlab::data::TypedArray<double> daKStorage = m_Factory.createArray<double>({12,12,(unsigned int)nElements},{0.0});
        matlab::data::TypedArray<double> daFStorage = m_Factory.createArray<double>({12,(unsigned int)nElements},{0.0});
        matlab::data::TypedArray<double> daDTs = m_Factory.createArray<double>({(unsigned int)nElements,(unsigned int)unNoGaussStations,6,6},{0.0});

        // Main element loop
        #pragma omp parallel shared(daKStorage,daFStorage,daDTs,daSegNode_r,daSegNode_z,nElements,dThickness,unNoGaussStations,unNoSimpsonStations,p_cMaterial,cDistributedPressures,\
            unElCounter,umOffsets,daElDOFs,daSIGMAS,daSigmas,daSigma_ys,daEpns,dvGauss_Nodes,dvGauss_Weights,dvGauss_NodesForPressure,dvGauss_WeightsForPressure,daOStorage,nDP,daDPmagnitudes,\
            include_nonlinear_G,include_nonlinear_M) num_threads(unNoThreads) reduction(+:daMTV) reduction(+:daVCV)
        {
            // All variables declared within the parallel region are private by default
            double r1, r2, z1, z2;
            double drds1, drds2, dzds1, dzds2;
            double phi1, phi2, dphids1, dphids2, d2phids2_1, d2phids2_2;
            double L, psi, eta, eta2, eta3;
            double Nphi, Ntheta, Nphitheta;
            double N01, N11, N02, N12, r, z, phi, dphids, c, s;
            unsigned int el_off, el_off1, el_off2, el_off3;

            bnu::matrix<double> DT(6,6);
            bnu::matrix<double> dm_KBARE(12,12);
            bnu::vector<double> dv_FBARE(12);
            double c1, s1, c2, s2;
            double dV1, dV5, dV7, dV11;
            bnu::vector<double> dc_KBARE_R1, dc_KBARE_R5, dc_KBARE_R7, dc_KBARE_R11;
            bnu::vector<double> dr_KBARE_R1, dr_KBARE_R5, dr_KBARE_R7, dr_KBARE_R11;

            bnu::matrix<double> dm_KRR(8,8), dm_KRC(8,2), dm_KCC(2,2);
            bnu::vector<double> dv_FR(8), dv_FC(2);
            bnu::matrix<double> dm_K(8,8), dm_Kpr1(8,8);
            bnu::vector<double> dv_F(8);
            bnu::vector<double> elDOFs(12);

            if (include_nonlinear_M == 0) { DT = p_cMaterial->DTe(dThickness); }

            #pragma	omp for schedule(dynamic,1)
            for (int E = 0; E < nElements; ++E)
            {
                // Recall element geometry
                r1 = daSegNode_r[E]; z1 = daSegNode_z[E]; // Element top node r-z coordinates
                r2 = daSegNode_r[E+1]; z2 = daSegNode_z[E+1]; // Element bottom node r-z coordinates
                drds1 = daOStorage[1][E]; drds2 = daOStorage[0][E];
                dzds1 = daOStorage[3][E]; dzds2 = daOStorage[2][E];
                phi1 = daOStorage[5][E]; phi2 = daOStorage[4][E];
                dphids2 = daOStorage[6][E]; dphids1 = daOStorage[7][E];
                d2phids2_2 = daOStorage[8][E]; d2phids2_1 = daOStorage[9][E];
                L = daOStorage[10][E]; psi = daOStorage[11][E];

                // Nodal dof values at element ends for axisymmetric pre-buckling state
                for (unsigned int I = 0; I < 12; ++I)
                    elDOFs(I) = daElDOFs[I][unElCounter + E];

                // Initialization to zeros of element's equivalent nodal loads vector and stiffness matrix
                for (unsigned int I = 0; I < 12; ++I)
                {
                    dv_FBARE(I) = 0.0;
                    for (unsigned int J = 0; J < 12; ++J)
                    {
                        dm_KBARE(I,J) = 0.0;
                    }
                }
                for (unsigned int P = 0; P < (unNoGaussStations + 1); ++P)
                {
                    eta = dvGauss_NodesForPressure[P];
                    eta2 = eta * eta;
                    eta3 = eta2 * eta;
                    N01 = (2. - 3. * eta + eta3) * 0.25;
                    N11 = L * (1. - eta - eta2 + eta3) * 0.25;
                    N02 = (2. + 3. * eta - eta3) * 0.25;
                    N12 = L * (-1. - eta + eta2 + eta3) * 0.25;
                    r = N01 * r1 + N02 * r2 + N11 * drds1 + N12 * drds2;
                    z = N01 * z1 + N02 * z2 + N11 * dzds1 + N12 * dzds2;
                    phi = N01 * phi1 + N02 * phi2 + N11 * dphids1 + N12 * dphids2;
                    dphids = N01 * dphids1 + N02 * dphids2 + N11 * d2phids2_1 + N12 * d2phids2_2;
                    c = cos(phi);
                    s = sin(phi);
                    for (int dp = 0; dp < nDP; ++dp)
                    {
                        for (unsigned int I = 0; I < 12; ++I)
                        {
                            dv_FBARE(I) += dvGauss_WeightsForPressure[P] * L * r * cDistributedPressures[dp]->eta_integral_equivalent_nodal_load_vector(I,eta,daDPmagnitudes[dp][E],daDPmagnitudes[dp][E+1],N01,N11,N02,N12,L,c,s);
                        }
                    }
                }
                // Integrate element stiffness matrix and nodal load vector
                for (unsigned int P = 0; P < unNoGaussStations; ++P)
                {
                    if (include_nonlinear_M == 1)
                    {
                        DT = p_cMaterial->DTep(dThickness,daSigmas,daSigma_ys,daEpns,E+unElCounter,P,unNoSimpsonStations);
                        for (unsigned int I = 0; I < 6; ++I)
                        {
                            for (unsigned int J = 0; J < 6; ++J)
                            {
                                daDTs[E][P][I][J] = DT(I,J);
                            }
                        }
                    }
                    eta = dvGauss_Nodes[P];
                    eta2 = eta * eta;
                    eta3 = eta2 * eta;
                    N01 = (2. - 3. * eta + eta3) * 0.25;
                    N11 = L * (1. - eta - eta2 + eta3) * 0.25;
                    N02 = (2. + 3. * eta - eta3) * 0.25;
                    N12 = L * (-1. - eta + eta2 + eta3) * 0.25;
                    r = N01 * r1 + N02 * r2 + N11 * drds1 + N12 * drds2;
                    z = N01 * z1 + N02 * z2 + N11 * dzds1 + N12 * dzds2;
                    phi = N01 * phi1 + N02 * phi2 + N11 * dphids1 + N12 * dphids2;
                    dphids = N01 * dphids1 + N02 * dphids2 + N11 * d2phids2_1 + N12 * d2phids2_2;
                    c = cos(phi);
                    s = sin(phi);
                    Nphi = daSIGMAS[0][E+unElCounter][P]; Ntheta = daSIGMAS[1][E+unElCounter][P]; Nphitheta = daSIGMAS[2][E+unElCounter][P];
                    if (include_nonlinear_G == 1)
                    {
                        dm_KBARE += dvGauss_Weights[P] * L * r * AQUINAS_BDTB_GN0G_matrix(eta, L, r, dphids, c, s, psi, Nphi, Ntheta, Nphitheta, elDOFs, DT);
                    }
                    else
                    {
                        dm_KBARE += dvGauss_Weights[P] * L * r * AQUINAS_B0DTB0(eta, 0, L, r, dphids, c, s, 0, psi, DT);
                    }
                }
                // Storing element equivalent nodal vector and stiffness matrix
                for (unsigned int I = 0; I < 12; ++I)
                {
                    daFStorage[I][E] = dv_FBARE(I);
                    for (unsigned int J = 0; J < 12; ++J)
                    {
                        daKStorage[I][J][E] = dm_KBARE(I,J);
                    }
                }

                // Transform element matrix & nodal load vector
                c1 = cos(phi1); s1 = sin(phi1); c2 = cos(phi2); s2 = sin(phi2);
                dc_KBARE_R1 = column(dm_KBARE, 1);
                dc_KBARE_R5 = column(dm_KBARE, 5);
                dc_KBARE_R7 = column(dm_KBARE, 7);
                dc_KBARE_R11 = column(dm_KBARE, 11);
                column(dm_KBARE, 1) = -s1 * dc_KBARE_R1 - c1 * dc_KBARE_R5;
                column(dm_KBARE, 5) = c1 * dc_KBARE_R1 - s1 * dc_KBARE_R5;
                column(dm_KBARE, 7) = -s2 * dc_KBARE_R7 - c2 * dc_KBARE_R11;
                column(dm_KBARE,11) = c2 * dc_KBARE_R7 - s2 * dc_KBARE_R11;

                dr_KBARE_R1 = row(dm_KBARE, 1);
                dr_KBARE_R5 = row(dm_KBARE, 5);
                dr_KBARE_R7 = row(dm_KBARE, 7);
                dr_KBARE_R11 = row(dm_KBARE, 11);
                row(dm_KBARE, 1) = -s1 * dr_KBARE_R1 - c1 * dr_KBARE_R5;
                row(dm_KBARE, 5) = c1 * dr_KBARE_R1 - s1 * dr_KBARE_R5;
                row(dm_KBARE, 7) = -s2 * dr_KBARE_R7 - c2 * dr_KBARE_R11;
                row(dm_KBARE,11) = c2 * dr_KBARE_R7 - s2 * dr_KBARE_R11;

                dV1 = dv_FBARE(1); dV5 = dv_FBARE(5); dV7 = dv_FBARE(7); dV11 = dv_FBARE(11);
                dv_FBARE(1) = -s1 * dV1 - c1 * dV5;
                dv_FBARE(5) = c1 * dV1 - s1 * dV5;
                dv_FBARE(7) = -s2 * dV7 - c2 * dV11;
                dv_FBARE(11) = c2 * dV7 - s2 * dV11;

                // Partition element matrix
                std::vector<unsigned int> vR_IDs{0,2,4,1,6,8,10,7}, vC_IDs{5,11};
                for (unsigned int I = 0; I < 8; ++I)
                {
                    for (unsigned int J = 0; J < 8; ++J)
                        dm_KRR(I,J) = dm_KBARE(vR_IDs[I],vR_IDs[J]);
                    for (unsigned int J = 0; J < 2; ++J)
                    {
                        dm_KRC(I,J) = dm_KBARE(vR_IDs[I],vC_IDs[J]);
                        if (I < 2) dm_KCC(I,J) = dm_KBARE(vC_IDs[I],vC_IDs[J]);
                    }
                    dv_FR(I) = dv_FBARE(vR_IDs[I]);
                    if (I < 2) dv_FC(I) = dv_FBARE(vC_IDs[I]);
                }
                bnu::matrix<double> dm_KCC_inv = bnu::identity_matrix<float>(dm_KCC.size1());
                bnu::permutation_matrix<size_t> pm(dm_KCC.size1());
                bnu::lu_factorize(dm_KCC, pm);
                bnu::lu_substitute(dm_KCC, pm, dm_KCC_inv);

                // Condense element matrix
                dm_Kpr1 = prod(dm_KRC, dm_KCC_inv);
                dm_K = dm_KRR - prod(dm_Kpr1, trans(dm_KRC));
                dv_F = dv_FR - prod(dm_Kpr1, trans(dv_FC));

                // Insert condensed matrix / vector coefficients into vector form containers
                el_off = (E + 1 - 1) * 4; el_off1 = (E + 1 - 1) * 3; el_off2 = (E + 1 - 1) * 2; el_off3 = (E + 1 - 1);
                for (unsigned int I = 0; I < 8; ++I)
                {
                    daMTV[I + el_off + umOffsets["D0"]] += dm_K(I,I); // Main diagonal
                    daVCV[I + el_off + umOffsets["D0"]] += dv_F(I); // Nodal load vector
                }
                for (unsigned int I = 0; I < 7; ++I)
                {
                    daMTV[I + el_off + umOffsets["U1"]] += dm_K(I,I+1); // First upper off-diagonal
                    daMTV[I + el_off + umOffsets["L1"]] += dm_K(I+1,I); // First lower off-diagonal
                }
                for (unsigned int I = 0; I < 6; ++I)
                {
                    daMTV[I + el_off + umOffsets["U2"]] += dm_K(I,I+2); // Second upper off-diagonal
                    daMTV[I + el_off + umOffsets["L2"]] += dm_K(I+2,I); // Second lower off-diagonal
                }
                for (unsigned int I = 0; I < 5; ++I)
                {
                    daMTV[I + el_off + umOffsets["U3"]] += dm_K(I,I+3); // Third upper off-diagonal
                    daMTV[I + el_off + umOffsets["L3"]] += dm_K(I+3,I); // Third lower off-diagonal
                }
                for (unsigned int I = 0; I < 4; ++I)
                {
                    daMTV[I + el_off + umOffsets["U4"]] += dm_K(I,I+4); // Fourth upper off-diagonal
                    daMTV[I + el_off + umOffsets["L4"]] += dm_K(I+4,I); // Fourth lower off-diagonal
                }
                for (unsigned int I = 0; I < 3; ++I)
                {
                    daMTV[I + el_off1 + umOffsets["U5"]] += dm_K(I,I+5); // Fifth upper off-diagonal
                    daMTV[I + el_off1 + umOffsets["L5"]] += dm_K(I+5,I); // Fifth lower off-diagonal
                }
                for (unsigned int I = 0; I < 2; ++I)
                {
                    daMTV[I + el_off2 + umOffsets["U6"]] += dm_K(I,I+6); // Sixth upper off-diagonal
                    daMTV[I + el_off2 + umOffsets["L6"]] += dm_K(I+6,I); // Sixth lower off-diagonal
                }
                daMTV[0 + el_off3 + umOffsets["U7"]] += dm_K(0,0+7); // Seventh upper off-diagonal
                daMTV[0 + el_off3 + umOffsets["L7"]] += dm_K(0+7,0); // Seventh lower off-diagonal
            } // End of parallel loop
        } // End of parallel region

        outputs[0] = daMTV;
        outputs[1] = daVCV;
        outputs[2] = daKStorage;
        outputs[3] = daFStorage;
        outputs[4] = daDTs;
    }


    void build_DT_matrix(double dThickness, matlab::data::TypedArray<double> daDE, bnu::matrix<double>& daDT)
    {
        double zmax = 0.5 * dThickness;
        double zmin = -zmax;
        double zm = zmax - zmin;
        double zmx2 = zmax * zmax;
        double zmn2 = zmin * zmin;
        double zmx3 = zmx2 * zmax;
        double zmn3 = zmn2 * zmin;
        double zm2 = (zmx2 - zmn2) * 0.5, zm3 = (zmx3 - zmn3) / 3.0;

        double I11 = daDE[0][0]*zm;
        double I12 = daDE[0][1]*zm;
        double I13 = daDE[0][2]*zm;
        double I14 = daDE[0][0]*zm2;
        double I15 = daDE[0][1]*zm2;
        double I16 = daDE[0][2]*zm2;
        double I21 = daDE[1][0]*zm;
        double I22 = daDE[1][1]*zm;
        double I23 = daDE[1][2]*zm;
        double I24 = daDE[1][0]*zm2;
        double I25 = daDE[1][1]*zm2;
        double I26 = daDE[1][2]*zm2;
        double I31 = daDE[2][0]*zm;
        double I32 = daDE[2][1]*zm;
        double I33 = daDE[2][2]*zm;
        double I34 = daDE[2][0]*zm2;
        double I35 = daDE[2][1]*zm2;
        double I36 = daDE[2][2]*zm2;
        double I44 = daDE[0][0]*zm3;
        double I45 = daDE[0][1]*zm3;
        double I46 = daDE[0][2]*zm3;
        double I54 = daDE[1][0]*zm3;
        double I55 = daDE[1][1]*zm3;
        double I56 = daDE[1][2]*zm3;
        double I64 = daDE[2][0]*zm3;
        double I65 = daDE[2][1]*zm3;
        double I66 = daDE[2][2]*zm3;

        daDT(0,0) = I11; daDT(0,1) = I12; daDT(0,2) = I13; daDT(0,3) = I14; daDT(0,4) = I15; daDT(0,5) = I16;
        daDT(1,0) = I21; daDT(1,1) = I22; daDT(1,2) = I23; daDT(1,3) = I24; daDT(1,4) = I25; daDT(1,5) = I26;
        daDT(2,0) = I31; daDT(2,1) = I32; daDT(2,2) = I33; daDT(2,3) = I34; daDT(2,4) = I35; daDT(2,5) = I36;
        daDT(3,0) = I14; daDT(3,1) = I15; daDT(3,2) = I16; daDT(3,3) = I44; daDT(3,4) = I45; daDT(3,5) = I46;
        daDT(4,0) = I24; daDT(4,1) = I25; daDT(4,2) = I26; daDT(4,3) = I54; daDT(4,4) = I55; daDT(4,5) = I56;
        daDT(5,0) = I34; daDT(5,1) = I35; daDT(5,2) = I36; daDT(5,3) = I64; daDT(5,4) = I65; daDT(5,5) = I66;
    }


    void obtain_Gauss_Legendre_nodes_weights(unsigned int unNoGaussStations, std::vector<double>& dvGauss_Nodes, std::vector<double>& dvGauss_Weights)
    {
        switch (unNoGaussStations)
        {
            case 1:
                dvGauss_Nodes.push_back( 0.0 );
                dvGauss_Weights.push_back( 2.0 );
                break;
            case 2:
                dvGauss_Nodes.push_back(-0.577350269189626 );
                dvGauss_Nodes.push_back( 0.577350269189626 );
                dvGauss_Weights.push_back( 1.0 );
                dvGauss_Weights.push_back( 1.0 );
                break;
            case 3:
                dvGauss_Nodes.push_back(-0.774596669241483 );
                dvGauss_Nodes.push_back( 0.0 );
                dvGauss_Nodes.push_back( 0.774596669241483 );
                dvGauss_Weights.push_back( 0.555555555555556 );
                dvGauss_Weights.push_back( 0.888888888888889 );
                dvGauss_Weights.push_back( 0.555555555555556 );
                break;
            case 4:
                dvGauss_Nodes.push_back(-0.861136311594053 );
                dvGauss_Nodes.push_back(-0.339981043584856 );
                dvGauss_Nodes.push_back( 0.339981043584856 );
                dvGauss_Nodes.push_back( 0.861136311594053 );
                dvGauss_Weights.push_back( 0.347854845137454 );
                dvGauss_Weights.push_back( 0.652145154862546 );
                dvGauss_Weights.push_back( 0.652145154862546 );
                dvGauss_Weights.push_back( 0.347854845137454 );
                break;
            case 5:
                dvGauss_Nodes.push_back(-0.906179845938664 );
                dvGauss_Nodes.push_back(-0.538469310105683 );
                dvGauss_Nodes.push_back( 0.0 );
                dvGauss_Nodes.push_back( 0.538469310105683 );
                dvGauss_Nodes.push_back( 0.906179845938664 );
                dvGauss_Weights.push_back( 0.236926885056189 );
                dvGauss_Weights.push_back( 0.478628670499366 );
                dvGauss_Weights.push_back( 0.568888888888889 );
                dvGauss_Weights.push_back( 0.478628670499366 );
                dvGauss_Weights.push_back( 0.236926885056189 );
                break;
            case 6:
                dvGauss_Nodes.push_back(-0.9324695142031521 );
                dvGauss_Nodes.push_back(-0.6612093864662645 );
                dvGauss_Nodes.push_back(-0.2386191860831969 );
                dvGauss_Nodes.push_back( 0.2386191860831969 );
                dvGauss_Nodes.push_back( 0.6612093864662645 );
                dvGauss_Nodes.push_back( 0.9324695142031521 );
                dvGauss_Weights.push_back( 0.1713244923791704 );
                dvGauss_Weights.push_back( 0.3607615730481386 );
                dvGauss_Weights.push_back( 0.4679139345726910 );
                dvGauss_Weights.push_back( 0.4679139345726910 );
                dvGauss_Weights.push_back( 0.3607615730481386 );
                dvGauss_Weights.push_back( 0.1713244923791704 );
                break;
            case 7:
                dvGauss_Nodes.push_back(-0.9491079123427585 );
                dvGauss_Nodes.push_back(-0.7415311855993945 );
                dvGauss_Nodes.push_back(-0.4058451513773972 );
                dvGauss_Nodes.push_back( 0.0 );
                dvGauss_Nodes.push_back( 0.4058451513773972 );
                dvGauss_Nodes.push_back( 0.7415311855993945 );
                dvGauss_Nodes.push_back( 0.9491079123427585 );
                dvGauss_Weights.push_back( 0.1294849661688697 );
                dvGauss_Weights.push_back( 0.2797053914892766 );
                dvGauss_Weights.push_back( 0.3818300505051189 );
                dvGauss_Weights.push_back( 0.4179591836734694 );
                dvGauss_Weights.push_back( 0.3818300505051189 );
                dvGauss_Weights.push_back( 0.2797053914892766 );
                dvGauss_Weights.push_back( 0.1294849661688697 );
                break;
        }
    }
};


// BSD 3-Clause License
//  
// Copyright (c) 2023, Mr Achilleas Filippidis and Dr Adam Jan Sadowski of 
// Imperial College London. All rights reserved.
//  
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//  
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//  
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//  
// 3. Neither the name of the copyright holder, nor of Imperial College, nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//  
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//  
// THE USE OF THE MATLAB LANGUAGE DOES NOT IMPLY ENDORSEMENT BY MATHWORKS.
