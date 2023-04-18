function KT = AQUINAS_BDTB_GN0G_matrix(eta,dat,Ns,DT)
% Last generated on Thu Mar 09 11:44:57 2023

eta2 = eta*eta; eta3 = eta2*eta; L = dat.L;
N01 = (2-3*eta+eta3)*0.25; N11 = L*(1-eta-eta2+eta3)*0.25; N02 = (2+3*eta-eta3)*0.25; N12 = L*(-1-eta+eta2+eta3)*0.25; % Eq. 2 in Rotter & Teng (Vol. 31)
r = N01*dat.r1 + N02*dat.r2 + N11*dat.drds1 + N12*dat.drds2; % Eqs. 1a & 2 in Rotter & Teng (Vol. 31)
phi = N01*dat.phi1 + N02*dat.phi2 + N11*dat.dphids1 + N12*dat.dphids2; % Cubic interpolation to compute the angle phi at the current Gauss station
dphids = N01*dat.dphids1 + N02*dat.dphids2 + N11*dat.d2phids2_1 + N12*dat.d2phids2_2; % Eqs 1c & 2 in Rotter & Teng (Vol. 31)
c = cos(phi); s = sin(phi);
elDOFs = dat.elDOFs;

Nphi = Ns(1);
Ntheta = Ns(2);
Nphitheta = Ns(3);

B0a = zeros(6,12);
Ga = zeros(6,12);

B0a_1_1 = 0.3e1 / 0.4e1 * c * (-1 + eta2) / L;
B0a_1_2 = (c * (-2 * eta - 1 + 3 * eta2)) / 0.4e1;
B0a_1_3 = 0;
B0a_1_4 = 0;
B0a_1_5 = -0.3e1 / 0.4e1 * s * (-1 + eta2) / L;
B0a_1_6 = -(s * (-2 * eta - 1 + 3 * eta2)) / 0.4e1;
B0a_1_7 = -0.3e1 / 0.4e1 * c * (-1 + eta2) / L;
B0a_1_8 = (c * (2 * eta - 1 + 3 * eta2)) / 0.4e1;
B0a_1_9 = 0;
B0a_1_10 = 0;
B0a_1_11 = 0.3e1 / 0.4e1 * s * (-1 + eta2) / L;
B0a_1_12 = -(s * (2 * eta - 1 + 3 * eta2)) / 0.4e1;

B0a_2_1 = ((eta + 2) * (eta - 1) ^ 2 * (c ^ 2 + s ^ 2) / r) / 0.4e1;
B0a_2_2 = (L * (eta + 1) * (eta - 1) ^ 2 * (c ^ 2 + s ^ 2) / r) / 0.4e1;
B0a_2_3 = 0;
B0a_2_4 = 0;
B0a_2_5 = 0;
B0a_2_6 = 0;
B0a_2_7 = -((eta - 2) * (eta + 1) ^ 2 * (c ^ 2 + s ^ 2) / r) / 0.4e1;
B0a_2_8 = (L * (eta - 1) * (eta + 1) ^ 2 * (c ^ 2 + s ^ 2) / r) / 0.4e1;
B0a_2_9 = 0;
B0a_2_10 = 0;
B0a_2_11 = 0;
B0a_2_12 = 0;

B0a_3_1 = 0;
B0a_3_2 = 0;
B0a_3_3 = 0;
B0a_3_4 = 0;
B0a_3_5 = 0;
B0a_3_6 = 0;
B0a_3_7 = 0;
B0a_3_8 = 0;
B0a_3_9 = 0;
B0a_3_10 = 0;
B0a_3_11 = 0;
B0a_3_12 = 0;

B0a_4_1 = (((-3 * c * eta2 + 3 * c) * dphids * L - 6 * eta * s) / L ^ 2) / 0.4e1;
B0a_4_2 = (-0.3e1 * (eta + 0.1e1 / 0.3e1) * (eta - 0.1e1) * c * dphids * L - 0.6e1 * eta * s + 0.2e1 * s) / L / 0.4e1;
B0a_4_3 = 0;
B0a_4_4 = 0;
B0a_4_5 = (((3 * eta2 * s - 3 * s) * dphids * L - 6 * c * eta) / L ^ 2) / 0.4e1;
B0a_4_6 = (0.3e1 * (eta + 0.1e1 / 0.3e1) * (eta - 0.1e1) * s * dphids * L - 0.6e1 * c * eta + 0.2e1 * c) / L / 0.4e1;
B0a_4_7 = (((3 * c * eta2 - 3 * c) * dphids * L + 6 * eta * s) / L ^ 2) / 0.4e1;
B0a_4_8 = (-0.3e1 * (eta + 1) * c * (eta - 0.1e1 / 0.3e1) * dphids * L - (6 * eta * s) - (2 * s)) / L / 0.4e1;
B0a_4_9 = 0;
B0a_4_10 = 0;
B0a_4_11 = (((-3 * eta2 * s + 3 * s) * dphids * L + 6 * c * eta) / L ^ 2) / 0.4e1;
B0a_4_12 = (0.3e1 * (eta + 1) * s * (eta - 0.1e1 / 0.3e1) * dphids * L - (6 * c * eta) - (2 * c)) / L / 0.4e1;

B0a_5_1 = -0.3e1 / 0.4e1 / r * c * s / L * (-1 + eta2);
B0a_5_2 = -0.3e1 / 0.4e1 * (eta + 0.1e1 / 0.3e1) * s * c * (eta - 0.1e1) / r;
B0a_5_3 = 0;
B0a_5_4 = 0;
B0a_5_5 = -0.3e1 / 0.4e1 / r * (c ^ 2) / L * (-1 + eta2);
B0a_5_6 = -(c ^ 2 * (eta - 1) / r * (3 * eta + 1)) / 0.4e1;
B0a_5_7 = 0.3e1 / 0.4e1 / r * c * s / L * (-1 + eta2);
B0a_5_8 = -0.3e1 / 0.4e1 * (eta + 1) * (eta - 0.1e1 / 0.3e1) * s * c / r;
B0a_5_9 = 0;
B0a_5_10 = 0;
B0a_5_11 = 0.3e1 / 0.4e1 / r * (c ^ 2) / L * (-1 + eta2);
B0a_5_12 = -(c ^ 2 * (eta + 1) / r * (3 * eta - 1)) / 0.4e1;

B0a_6_1 = 0;
B0a_6_2 = 0;
B0a_6_3 = 0;
B0a_6_4 = 0;
B0a_6_5 = 0;
B0a_6_6 = 0;
B0a_6_7 = 0;
B0a_6_8 = 0;
B0a_6_9 = 0;
B0a_6_10 = 0;
B0a_6_11 = 0;
B0a_6_12 = 0;


B0a(1,1) = B0a_1_1;
B0a(1,2) = B0a_1_2;
B0a(1,3) = B0a_1_3;
B0a(1,4) = B0a_1_4;
B0a(1,5) = B0a_1_5;
B0a(1,6) = B0a_1_6;
B0a(1,7) = B0a_1_7;
B0a(1,8) = B0a_1_8;
B0a(1,9) = B0a_1_9;
B0a(1,10) = B0a_1_10;
B0a(1,11) = B0a_1_11;
B0a(1,12) = B0a_1_12;

B0a(2,1) = B0a_2_1;
B0a(2,2) = B0a_2_2;
B0a(2,3) = B0a_2_3;
B0a(2,4) = B0a_2_4;
B0a(2,5) = B0a_2_5;
B0a(2,6) = B0a_2_6;
B0a(2,7) = B0a_2_7;
B0a(2,8) = B0a_2_8;
B0a(2,9) = B0a_2_9;
B0a(2,10) = B0a_2_10;
B0a(2,11) = B0a_2_11;
B0a(2,12) = B0a_2_12;

B0a(3,1) = B0a_3_1;
B0a(3,2) = B0a_3_2;
B0a(3,3) = B0a_3_3;
B0a(3,4) = B0a_3_4;
B0a(3,5) = B0a_3_5;
B0a(3,6) = B0a_3_6;
B0a(3,7) = B0a_3_7;
B0a(3,8) = B0a_3_8;
B0a(3,9) = B0a_3_9;
B0a(3,10) = B0a_3_10;
B0a(3,11) = B0a_3_11;
B0a(3,12) = B0a_3_12;

B0a(4,1) = B0a_4_1;
B0a(4,2) = B0a_4_2;
B0a(4,3) = B0a_4_3;
B0a(4,4) = B0a_4_4;
B0a(4,5) = B0a_4_5;
B0a(4,6) = B0a_4_6;
B0a(4,7) = B0a_4_7;
B0a(4,8) = B0a_4_8;
B0a(4,9) = B0a_4_9;
B0a(4,10) = B0a_4_10;
B0a(4,11) = B0a_4_11;
B0a(4,12) = B0a_4_12;

B0a(5,1) = B0a_5_1;
B0a(5,2) = B0a_5_2;
B0a(5,3) = B0a_5_3;
B0a(5,4) = B0a_5_4;
B0a(5,5) = B0a_5_5;
B0a(5,6) = B0a_5_6;
B0a(5,7) = B0a_5_7;
B0a(5,8) = B0a_5_8;
B0a(5,9) = B0a_5_9;
B0a(5,10) = B0a_5_10;
B0a(5,11) = B0a_5_11;
B0a(5,12) = B0a_5_12;

B0a(6,1) = B0a_6_1;
B0a(6,2) = B0a_6_2;
B0a(6,3) = B0a_6_3;
B0a(6,4) = B0a_6_4;
B0a(6,5) = B0a_6_5;
B0a(6,6) = B0a_6_6;
B0a(6,7) = B0a_6_7;
B0a(6,8) = B0a_6_8;
B0a(6,9) = B0a_6_9;
B0a(6,10) = B0a_6_10;
B0a(6,11) = B0a_6_11;
B0a(6,12) = B0a_6_12;


Ga_1_1 = 0.3e1 / 0.4e1 * s * (-1 + eta2) / L;
Ga_1_2 = (s * (-2 * eta - 1 + 3 * eta2)) / 0.4e1;
Ga_1_3 = 0;
Ga_1_4 = 0;
Ga_1_5 = 0.3e1 / 0.4e1 * c * (-1 + eta2) / L;
Ga_1_6 = (c * (-2 * eta - 1 + 3 * eta2)) / 0.4e1;
Ga_1_7 = -0.3e1 / 0.4e1 * s * (-1 + eta2) / L;
Ga_1_8 = (s * (2 * eta - 1 + 3 * eta2)) / 0.4e1;
Ga_1_9 = 0;
Ga_1_10 = 0;
Ga_1_11 = -0.3e1 / 0.4e1 * c * (-1 + eta2) / L;
Ga_1_12 = (c * (2 * eta - 1 + 3 * eta2)) / 0.4e1;

Ga_2_1 = 0;
Ga_2_2 = 0;
Ga_2_3 = 0;
Ga_2_4 = 0;
Ga_2_5 = 0;
Ga_2_6 = 0;
Ga_2_7 = 0;
Ga_2_8 = 0;
Ga_2_9 = 0;
Ga_2_10 = 0;
Ga_2_11 = 0;
Ga_2_12 = 0;

Ga_3_1 = 0.3e1 / 0.4e1 * c * (-1 + eta2) / L;
Ga_3_2 = (c * (-2 * eta - 1 + 3 * eta2)) / 0.4e1;
Ga_3_3 = 0;
Ga_3_4 = 0;
Ga_3_5 = -0.3e1 / 0.4e1 * s * (-1 + eta2) / L;
Ga_3_6 = -(s * (-2 * eta - 1 + 3 * eta2)) / 0.4e1;
Ga_3_7 = -0.3e1 / 0.4e1 * c * (-1 + eta2) / L;
Ga_3_8 = (c * (2 * eta - 1 + 3 * eta2)) / 0.4e1;
Ga_3_9 = 0;
Ga_3_10 = 0;
Ga_3_11 = 0.3e1 / 0.4e1 * s * (-1 + eta2) / L;
Ga_3_12 = -(s * (2 * eta - 1 + 3 * eta2)) / 0.4e1;

Ga_4_1 = 0;
Ga_4_2 = 0;
Ga_4_3 = 0;
Ga_4_4 = 0;
Ga_4_5 = 0;
Ga_4_6 = 0;
Ga_4_7 = 0;
Ga_4_8 = 0;
Ga_4_9 = 0;
Ga_4_10 = 0;
Ga_4_11 = 0;
Ga_4_12 = 0;

Ga_5_1 = ((eta + 2) * (eta - 1) ^ 2 * (c ^ 2 + s ^ 2) / r) / 0.4e1;
Ga_5_2 = (L * (eta + 1) * (eta - 1) ^ 2 * (c ^ 2 + s ^ 2) / r) / 0.4e1;
Ga_5_3 = 0;
Ga_5_4 = 0;
Ga_5_5 = 0;
Ga_5_6 = 0;
Ga_5_7 = -((eta - 2) * (eta + 1) ^ 2 * (c ^ 2 + s ^ 2) / r) / 0.4e1;
Ga_5_8 = (L * (eta - 1) * (eta + 1) ^ 2 * (c ^ 2 + s ^ 2) / r) / 0.4e1;
Ga_5_9 = 0;
Ga_5_10 = 0;
Ga_5_11 = 0;
Ga_5_12 = 0;

Ga_6_1 = 0;
Ga_6_2 = 0;
Ga_6_3 = 0;
Ga_6_4 = 0;
Ga_6_5 = 0;
Ga_6_6 = 0;
Ga_6_7 = 0;
Ga_6_8 = 0;
Ga_6_9 = 0;
Ga_6_10 = 0;
Ga_6_11 = 0;
Ga_6_12 = 0;


Ga(1,1) = Ga_1_1;
Ga(1,2) = Ga_1_2;
Ga(1,3) = Ga_1_3;
Ga(1,4) = Ga_1_4;
Ga(1,5) = Ga_1_5;
Ga(1,6) = Ga_1_6;
Ga(1,7) = Ga_1_7;
Ga(1,8) = Ga_1_8;
Ga(1,9) = Ga_1_9;
Ga(1,10) = Ga_1_10;
Ga(1,11) = Ga_1_11;
Ga(1,12) = Ga_1_12;

Ga(2,1) = Ga_2_1;
Ga(2,2) = Ga_2_2;
Ga(2,3) = Ga_2_3;
Ga(2,4) = Ga_2_4;
Ga(2,5) = Ga_2_5;
Ga(2,6) = Ga_2_6;
Ga(2,7) = Ga_2_7;
Ga(2,8) = Ga_2_8;
Ga(2,9) = Ga_2_9;
Ga(2,10) = Ga_2_10;
Ga(2,11) = Ga_2_11;
Ga(2,12) = Ga_2_12;

Ga(3,1) = Ga_3_1;
Ga(3,2) = Ga_3_2;
Ga(3,3) = Ga_3_3;
Ga(3,4) = Ga_3_4;
Ga(3,5) = Ga_3_5;
Ga(3,6) = Ga_3_6;
Ga(3,7) = Ga_3_7;
Ga(3,8) = Ga_3_8;
Ga(3,9) = Ga_3_9;
Ga(3,10) = Ga_3_10;
Ga(3,11) = Ga_3_11;
Ga(3,12) = Ga_3_12;

Ga(4,1) = Ga_4_1;
Ga(4,2) = Ga_4_2;
Ga(4,3) = Ga_4_3;
Ga(4,4) = Ga_4_4;
Ga(4,5) = Ga_4_5;
Ga(4,6) = Ga_4_6;
Ga(4,7) = Ga_4_7;
Ga(4,8) = Ga_4_8;
Ga(4,9) = Ga_4_9;
Ga(4,10) = Ga_4_10;
Ga(4,11) = Ga_4_11;
Ga(4,12) = Ga_4_12;

Ga(5,1) = Ga_5_1;
Ga(5,2) = Ga_5_2;
Ga(5,3) = Ga_5_3;
Ga(5,4) = Ga_5_4;
Ga(5,5) = Ga_5_5;
Ga(5,6) = Ga_5_6;
Ga(5,7) = Ga_5_7;
Ga(5,8) = Ga_5_8;
Ga(5,9) = Ga_5_9;
Ga(5,10) = Ga_5_10;
Ga(5,11) = Ga_5_11;
Ga(5,12) = Ga_5_12;

Ga(6,1) = Ga_6_1;
Ga(6,2) = Ga_6_2;
Ga(6,3) = Ga_6_3;
Ga(6,4) = Ga_6_4;
Ga(6,5) = Ga_6_5;
Ga(6,6) = Ga_6_6;
Ga(6,7) = Ga_6_7;
Ga(6,8) = Ga_6_8;
Ga(6,9) = Ga_6_9;
Ga(6,10) = Ga_6_10;
Ga(6,11) = Ga_6_11;
Ga(6,12) = Ga_6_12;


betas = Ga*elDOFs;
Omega = zeros(6,6);
Omega(1,1) = betas(1);
Omega(1,2) = betas(2);
Omega(1,3) = betas(3);
Omega(2,4) = betas(4);
Omega(2,5) = betas(5);
Omega(2,6) = betas(6);
Omega(3,1) = betas(4);
Omega(3,2) = betas(5);
Omega(3,3) = betas(6);
Omega(3,4) = betas(1);
Omega(3,5) = betas(2);
Omega(3,6) = betas(3);

BLa = Omega*Ga;

N0 = zeros(6,6);
N0(1,1) = Nphi;
N0(1,4) = Nphitheta;
N0(2,2) = Nphi;
N0(2,5) = Nphitheta;
N0(3,3) = Nphi;
N0(3,5) = Nphitheta;
N0(4,4) = Ntheta;
N0(4,1) = Nphitheta;
N0(5,5) = Ntheta;
N0(5,2) = Nphitheta;
N0(6,6) = Ntheta;
N0(6,3) = Nphitheta;


Ba = B0a + BLa;
KT = r*(Ba'*DT*Ba + Ga'*N0*Ga);

end
