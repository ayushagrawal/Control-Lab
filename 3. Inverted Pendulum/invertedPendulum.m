% Code to solve for the optimum state feedback law u = -Kx
% given the state-space model for the inverted pendulum

%% Mechanical Parameters in SI Units
Mp = 0.027;			% Mass of the pendulum assembly
lp = 0.153;			% Length of pendulum centre of mass from pivot
Lp = 0.191;			% Total length of pendulum
r = 0.08260;		% Length of arm pivot to pendulum pivot
Jm = 3E-5;			% Motor shaft moment of inertia
Marm = 0.028;		% Mass of arm
g = 9.810;			% Gravitational acceleration constant
Jeq = 1.23E-4;		% Equivalent moment of inertia about the motor shaft pivot axis
Jp = 1.1E-4;		% Pendulum moment of inertia about its pivot axis
Beq = 0;			% Arm viscous damping
Bp = 0;				% Pendulum viscous damping

%% Electro-Mechanical Parameters in SI Units
Rm = 3.3;			% Motor armature resistance
Kt = 0.02797;		% Motor torque constant
Km =  0.02797;		% Motor back-electromotive force constant

%% State-Space Definition
% dx/dt = Ax + Bu

denom = (Jp*Jeq + Mp*lp^2*Jeq + Jp*Mp*r^2);		%Frequently used denominator term

% The A Matrix
A = [0, 0, 1, 0; 
	0, 0, 0, 1;
	0, r*Mp^2*lp^2*g / denom, -Kt*Km*(Jp + Mp*lp^2) / (Rm*denom), 0;
	0, Mp*lp*g*(Jeq + Mp*r^2) / denom, -Mp*lp*Kt*r*Km / (Rm*denom), 0];
A
% The B Matrix
B = [0; 0; Kt*(Jp + Mp*lp^2) / (Rm*denom); Mp*lp*Kt*r / (Rm*denom)];

%% The Quadratic Minimisation Matrices (Tuning Component)

%{
%	Design an LQR control, that is tune the Q weighing matrix, such that 
%	the closed-loop response meets the following specifications:
%	(1) Arm Regulation: |theta(t)| < 30°
%	(2) Pendulum Regulation: |alpha(t)| < 3°
%	(3) Control input limit: Vm < 12 V
%}

% The Q Matrix: State variables Cost
Q = [50, 0, 0, 0;
	0, 25, 0, 0;
	0, 0, 4.25, 0;
	0, 0, 0, 3];	% x'Qx where x = [theta, alpha, dtheta/dt, dalpha/dt]

% The R Matrix: Input Cost
R = 4;				% Because we have only one input

%% The LQR Solution using the in-built MATLAB Function

% help lqr;
%{ 
%	Syntax:  [K,S,E] = lqr(SYS,Q,R,N) calculates the optimal gain matrix K
%	such that:
%
%	For a continuous-time state-space model SYS, the state-feedback
%	law u = -Kx  minimizes the cost function
%
%		J = Integral {x'Qx + u'Ru + 2*x'Nu} dt
%
%	subject to the system dynamics  dx/dt = Ax + Bu
%	The matrix N is set to zero when omitted.  Also returned are the
%	the solution S of the associated algebraic Riccati equation and
%	the closed-loop eigenvalues E = EIG(A-B*K).
%
%	[K,S,E] = lqr(A,B,Q,R,N) is an equivalent syntax for continuous-time
%	models with dynamics  dx/dt = Ax + Bu
%}

[K, ~, ~] = lqr(A, B, Q, R);
disp(K);


