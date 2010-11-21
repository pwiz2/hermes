#define HERMES_REPORT_WARN
#define HERMES_REPORT_INFO
#define HERMES_REPORT_VERBOSE
#include "config.h"
//#include <getopt.h>
#include <hermes3d.h>

// PDE: -\laplace u_1 = f_1,
//		-\laplace u_2 = f_2.	
//
// BC:         u_1 = x^2 + y^2 + z^2,
//             u_2 = x^3 + y^3 + z^3.
//
// exact solution: u_1 = (1 - x^2) (1 - y^2) (1 - z^2),
//				   u_2 = (1 - x^2) x^2 (1 - y^2) y^2 (1 - z^2) z^2. 	

// The following parameters can be changed:
MatrixSolverType matrix_solver = SOLVER_UMFPACK;  // Possibilities: SOLVER_AMESOS, SOLVER_MUMPS, 
                                                  // SOLVER_PARDISO, SOLVER_PETSC, SOLVER_UMFPACK.
const char* iterative_method = "bicgstab";        // Name of the iterative method employed by AztecOO (ignored
                                                  // by the other solvers). 
                                                  // Possibilities: gmres, cg, cgs, tfqmr, bicgstab.
const char* preconditioner = "jacobi";            // Name of the preconditioner employed by AztecOO (ignored by
                                                  // the other solvers). 
                                                  // Possibilities: none, jacobi, neumann, least-squares, or a
                                                  // preconditioner from IFPACK (see solver/aztecoo.h).

// Error should be smaller than this epsilon.
#define EPS								10e-10F

// Problem parameters.
template<typename T>
T u1(T x, T y, T z) {
	return x*x + y*y + z*z;
}

template<typename T>
T u2(T x, T y, T z) {
	return x*x*x + y*y*y + z*z*z;
}

// Exact solutions.
double exact_sln_fn_1(double x, double y, double z, double &dx, double &dy, double &dz) {
	dx = 2 * x;
	dy = 2 * y;
	dz = 2 * z;

	return u1(x, y, z);
}

double exact_sln_fn_2(double x, double y, double z, double &dx, double &dy, double &dz) {
	dx = 3 * x*x;
	dy = 3 * y*y;
	dz = 3 * z*z;

	return u2(x, y, z);
}

// Boundary condition types.
BCType bc_types(int marker) 
{
	return BC_ESSENTIAL;
}

// Dirichlet boundary conditions.
scalar essential_bc_values_1(int ess_bdy_marker, double x, double y, double z) {
	return u1(x, y, z);
}

// Dirichlet boundary conditions.
scalar essential_bc_values_2(int ess_bdy_marker, double x, double y, double z) {
	return u2(x, y, z);
}

template<typename Real, typename Scalar>
Scalar bilinear_form_1(int n, double *wt, Func<Scalar> *u_ext[], Func<Real> *u, Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *data) {
	return int_grad_u_grad_v<Real, Scalar>(n, wt, u, v, e);
}

template<typename Real, typename Scalar>
Scalar bilinear_form_2(int n, double *wt, Func<Scalar> *u_ext[], Func<Real> *u, Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *data) {
	return int_grad_u_grad_v<Real, Scalar>(n, wt, u, v, e);
}

template<typename T>
T f1(T x, T y, T z) {
	return -6.0;
}

template<typename Real, typename Scalar>
Scalar linear_form_1(int n, double *wt, Func<Scalar> *u_ext[], Func<Real> *u, Geom<Real> *e, ExtData<Scalar> *data) {
	return int_F_v<Real, Scalar>(n, wt, f1, u, e);
}

template<typename T>
T f2(T x, T y, T z) {
	return -(6 * x + 6 * y + 6 * z);
}

template<typename Real, typename Scalar>
Scalar linear_form_2(int n, double *wt, Func<Scalar> *u_ext[], Func<Real> *u, Geom<Real> *e, ExtData<Scalar> *data) {
	return int_F_v<Real, Scalar>(n, wt, f2, u, e);
}

int main(int argc, char **args)
{
  // Test variable.
  int success_test = 1;

	if (argc < 2) error("Not enough parameters.");

  // Load the mesh.
	Mesh mesh;
  H3DReader mloader;
  if (!mloader.load(args[1], &mesh)) error("Loading mesh file '%s'.", args[1]);

  // Initialize the space 1.
	Ord3 o1(2, 2, 2);
	H1Space space1(&mesh, bc_types, essential_bc_values_1, o1);

	// Initialize the space 2.
	Ord3 o2(4, 4, 4);
	H1Space space2(&mesh, bc_types, essential_bc_values_2, o2);

  // Initialize the weak formulation.
	WeakForm wf(2);
	wf.add_matrix_form(0, 0, bilinear_form_1<double, scalar>, bilinear_form_1<Ord, Ord>, HERMES_SYM);
	wf.add_vector_form(0, linear_form_1<double, scalar>, linear_form_1<Ord, Ord>);
	wf.add_matrix_form(1, 1, bilinear_form_2<double, scalar>, bilinear_form_2<Ord, Ord>, HERMES_SYM);
	wf.add_vector_form(1, linear_form_2<double, scalar>, linear_form_2<Ord, Ord>);

  // Initialize the FE problem.
  bool is_linear = true;
  DiscreteProblem dp(&wf, Tuple<Space *>(&space1, &space2), is_linear);

  // Set up the solver, matrix, and rhs according to the solver selection.
  SparseMatrix* matrix = create_matrix(matrix_solver);
  Vector* rhs = create_vector(matrix_solver);
  Solver* solver = create_linear_solver(matrix_solver, matrix, rhs);

  // Initialize the preconditioner in the case of SOLVER_AZTECOO.
  if (matrix_solver == SOLVER_AZTECOO) 
  {
    ((AztecOOSolver*) solver)->set_solver(iterative_method);
    ((AztecOOSolver*) solver)->set_precond(preconditioner);
    // Using default iteration parameters (see solver/aztecoo.h).
  }

  // Assemble the linear problem.
  info("Assembling (ndof: %d).", Space::get_num_dofs(Tuple<Space *>(&space1, &space2)));
  dp.assemble(matrix, rhs);

  // Solve the linear system. If successful, obtain the solution.
  info("Solving.");
		Solution sln1(&mesh);
		Solution sln2(&mesh);
  if(solver->solve()) Solution::vector_to_solutions(solver->get_solution(), Tuple<Space *>(&space1, &space2), Tuple<Solution *>(&sln1, &sln2));
  else error ("Matrix solver failed.\n");

  ExactSolution ex_sln1(&mesh, exact_sln_fn_1);
  ExactSolution ex_sln2(&mesh, exact_sln_fn_2);

  // Calculate exact error.
  info("Calculating exact error.");
  Adapt *adaptivity = new Adapt(Tuple<Space *>(&space1, &space2), Tuple<ProjNormType>(HERMES_H1_NORM, HERMES_H1_NORM));
  bool solutions_for_adapt = false;
  double err_exact = adaptivity->calc_err_exact(Tuple<Solution *>(&sln1, &sln2), Tuple<Solution *>(&ex_sln1, &ex_sln2), solutions_for_adapt, HERMES_TOTAL_ERROR_ABS);

  if (err_exact > EPS)
		// Calculated solution is not precise enough.
		success_test = 0;

  // Clean up.
  delete matrix;
  delete rhs;
  delete solver;
  delete adaptivity;
  
  if (success_test) {
    info("Success!");
    return ERR_SUCCESS;
	}
	else {
    info("Failure!");
    return ERR_FAILURE;
	}
}

