/**

@{

\brief Example usage of Sacado for tensor calculus

\author jfriedlein

@tableofcontents

@mainpage Trilinos::Sacado example documentation

@section intro Introduction
The way we see and use Sacado here is as follows: \n
If you usually compute the following equation \n
\f[	c = a + b \f]
for instance with data types double as \n
\f[ 1.0 + 1.0 \rightarrow 2.0 \f]
your results is just a double number \f$ c \f$ that contains the value \f$ 2 \f$. \n

Using Sacado, on the other hand, the variable \f$ c_{fad} \f$ is now of, for example, data type
@code
	Sacado::Fad::DFad<double> c_fad;
@endcode
As a result, \f$ c_{fad} \f$ now contains not just the number \f$ 2 \f$, but also all the derivatives of \f$ c_{fad} \f$
with respect to the previously defined degrees of freedom (set via command *.diff(*)). \n
The following figure tries to visualize this:
\image html Sacado_data-type.png
@todo update this figure or add another one for second derivatives
@todo add another less general figure with c, a and b and explain what is meant by point p

If you right away want to use Sacado, then you might skip the first examples and jump to Ex3B.
There we show how to use the "Sacado_Wrapper" that does everything from Ex2 and Ex3 in just a view lines of code. This does not mean that
the here shown approach is the fastest or most efficient, it is just simple and easy to use.

Furthermore, if you, for instance, compute problems with two-fields (e.g. displacement and scalar damage) and you need
tangents with respect to both a tensor (e.g. strain tensor) and a scalar (e.g. damage variable), you can use the Sacado_Wrapper as shown in Ex4.

Some more basics: \n
One can access the double value of \f$ c_{fad} \f$ with the Sacado command *.val():
@code
	double c_value = c_fad.val();
@endcode
The derivatives of \f$ c_{fad} \f$ can be accessed with the command *.dx():
@code
	double d_c_d_a = c_fad.dx(0);
	double c_c_d_b = c_fad.dx(1);
@endcode
The arguments of \a dx, namely 0 and 1 are the numbers corresponding to the dof that belong to \a a and \a b. More details on how to set this up
and use it, are given in example Ex1.

Some resources/links: \n
You can use Sacado to compute general derivatives of functions (with or without tensors) with respect to variables (double, Tensors, ...).
@todo link the Sacado and DII pages

The here shown examples shall solely show how Sacado can be applied and give some background and a look under the hood.
The code is neither elegant nor efficient, but it works. A more user-friendly version is provided by means of the "Sacado_Wrapper". \n
@todo add list of files and an overview
@todo explain how to use the Wrapper (download the file Sacado_Wrapper.h, #include, ...)
@todo Check if factor 0.5 is also necessary for d_sigma / d_phi

@note This documentation and code only protocol my first steps with Sacado. They are not guaranteed to be correct neither are they verified.
Any comments, criticism, corrections, feedback, improvements, ... are appreciated and very well welcomed.

@section code The commented program

\code
 
/*
 * Author: jfriedlein, 2019
 * 		dsoldner, 2019
 */
 
\endcode
@section includes Include Files
The data type SymmetricTensor and some related operations, such as trace, symmetrize, deviator, ... for tensor calculus
\code
#include <deal.II/base/symmetric_tensor.h>
 
\endcode
C++ headers (some basics, standard stuff)
\code
#include <iostream>
#include <fstream>
#include <cmath>
 
\endcode
Sacado (from Trilinos, data types, operations, ...)
\code
#include <Sacado.hpp>
 
#include "Sacado_Wrapper.h"
 
 
\endcode
Those headers are related to data types and autodiff, but don't seem to be needed
\code
//#  include <deal.II/base/numbers.h>
//#  include <deal.II/differentiation/ad/ad_number_traits.h>
//#  include <deal.II/differentiation/ad/sacado_number_types.h>
 
\endcode
According to the basics of deal.ii-programming (see dealii.org and https://www.dealii.org/current/doxygen/deal.II/step_1.html for a start)
\code
using namespace dealii;
 
\endcode
Defining a data type for the Sacado variables (here we simply used the standard types from the deal.ii step-33 tutorial's introduction)
\code
using fad_double = Sacado::Fad::DFad<double>;	// this data type now represents a double, but also contains the derivative of this variable with respect to the defined dofs (set via command *.diff(*))
 
 
\endcode
@section Ex1 1. example: simple scalar equation
1. example: simple scalar equation from deal.ii-tutorial step-33 (see the introduction there to get a first impression, https://www.dealii.org/current/doxygen/deal.II/step_33.html)
@todo clean up the documentation of the classes

\code
void sacado_test_scalar ()
{
	std::cout << "Scalar Test:" << std::endl;
\endcode
define the variables used in the computation (inputs: a, b; output: c; auxiliaries: *) as the Sacado-data type
\code
	 fad_double a,b,c;
\endcode
initialize the input variables a and b; This (a,b) = (1,2) will be the point where the derivatives are computed.
Compare: y=x² -> (dy/dx)(\@x=1) = 2. We can only compute the derivative numerically at a certain point.
\code
	 a = 1;
	 b = 2;
 
	a.diff(0,2);  // Set a to be dof 0, in a 2-dof system.
	b.diff(1,2);  // Set b to be dof 1, in a 2-dof system.
\endcode
Our equation here is very simply. But you can use nested equations and many standard mathematical operations, such as sqrt, pow, sin, ...
\code
	c = 2*a + std::cos(a*b);
	double *derivs = &c.fastAccessDx(0); // Access the derivatives of
\endcode
Output the derivatives of c with respect to the two above defined degrees of freedom (dof)
\code
	std::cout << "Derivatives at the point (" << a << "," << b << ")" << std::endl;
	std::cout << "dc/da = " << derivs[0] << ", dc/db=" << derivs[1] << std::endl;
}
 
 
\endcode
@section Ex2 2. example: Preparation for the use of Sacado with tensors
Here we want to introduce tensors for the first time. Hence, we limit ourselves to a trivial equation relating the strain tensor \a eps
with dim x dim components with the stress tensor \a sigma. Both here used tensors are symmetric, hence we use the SymmetricTensor class and
have to keep some details in mind (see below factor 0.5 related to Voigt-Notation).
\code
/*
 * 2. example: use of tensors
 */
void sacado_test_2 ()
{
	std::cout << "Test 2:" << std::endl;
 
\endcode
First we set the dimension \a dim: 2D->dim=2; 3D->dim=3 \n This defines the "size" of the tensors and the number of dofs. Ex2 only works in 3D, whereas the following Ex3 is set up dimension-independent.
\code
	const unsigned int dim = 3;
 
\endcode
Declare our input, auxiliary and output variables as SymmetricTensors consisting of fad_doubles (instead of the standard SymmetricTensor out of doubles)
\code
	SymmetricTensor<2,dim, fad_double> sigma, eps;
 
\endcode
Init the strain tensor (the point at which the derivative shall be computed)
\code
	eps[0][0] = 1;
	eps[1][1] = 2;
	eps[2][2] = 3;
	eps[0][1] = 4;
	eps[0][2] = 5;
	eps[1][2] = 6;
 
\endcode
Now we declare the dofs. The derivative to a tensor requires all components, therefore we set the components of the strain tensor here one by one as the dofs.
Because our tensors are symmetric, we only need 6 components in 3D instead of 9 for a full second order tensor
\code
	eps[0][0].diff(0,6);
	eps[1][1].diff(1,6);
	eps[2][2].diff(2,6);
	eps[0][1].diff(3,6);
	eps[0][2].diff(4,6);
	eps[1][2].diff(5,6);
 
\endcode
The equation describing the stresses (here just a simple test case)
\code
	sigma = eps;
 
\endcode
Let's output the computed stress tensor.
\code
	std::cout << sigma << std::endl;
\endcode
The resulting values of \a sigma are fairly boring, due to our simple equation. It is the additional output generated by
this, that is interesting here: \n
output: \n
1 [ 1 0 0 0 0 0 ] 4 [ 0 0 0 1 0 0 ] 5 [ 0 0 0 0 1 0 ] 4 [ 0 0 0 1 0 0 ] 2 [ 0 1 0 0 0 0 ] 6 [ 0 0 0 0 0 1 ] 5 [ 0 0 0 0 1 0 ] 6 [ 0 0 0 0 0 1 ] 3 [ 0 0 1 0 0 0 ] \n
The numbers 1, 4, 5, 4, ... are the entries in the stress tensor \a sigma. In square brackets we see the derivatives of sigma with respect to all the dofs set previously
given in the order we defined them above. Meaning: The first entry in the square brackets corresponds to the 0-th dof set by
@code eps[0][0].diff(0,6); @endcode referring to the component (0,0) in the strain tensor \a eps.
 
Computing the derivatives for certain components of the resulting tangent modulus: \n
We now access these lists of derivatives (output above in square brackets) for one component of the stress tensor \a sigma at a time.
\code
	{
\endcode
Access the derivatives corresponding to the component (0,0) of the stress tensor \a sigma
\code
		double *derivs = &sigma[0][0].fastAccessDx(0);
\endcode
The following output will show us the same derivatives that we already saw above, just formatted differently \n
output: d_sigma[0][0]/d_eps = 1 , 0 , 0 , 0 , 0 , 0 ,
\code
		std::cout << "d_sigma[0][0]/d_eps = ";
		for ( unsigned int i=0; i<6; ++i)
			std::cout << derivs[i] << " , ";
		std::cout << std::endl;
	}
	{
\endcode
Access the derivatives corresponding to the component (1,2) of the stress tensor \a sigma
\code
		double *derivs = &sigma[1][2].fastAccessDx(0);
\endcode
output: d_sigma[1][2]/d_eps = 0 , 0 , 0 , 0 , 0 , 1 ,
\code
		std::cout << "d_sigma[1][2]/d_eps = ";
		for ( unsigned int i=0; i<6; ++i)
			std::cout << derivs[i] << " , ";
		std::cout << std::endl;
	}
}
 
 
\endcode
@section Ex3 3. example: Using a slightly more complicated stress equation
\code
void sacado_test_3 ()
{
	std::cout << "Test 3:" << std::endl;
 
	const unsigned int dim = 3;
 
\endcode
Here we also define some constant, for instance the bulk modulus \a kappa and the second Lamè parameter \a mu.
We now also define one of our constants as fad_double. By doing this we can use the normal multiplication (see below).
\code
	double kappa_param = 5;
	fad_double kappa (kappa_param);
\endcode
The second constant remains as a double just to show the difference.
\code
	double mu = 2;
 
	SymmetricTensor<2,dim, fad_double> sigma, eps;
 
\endcode
To simplify the access to the dofs we define a map that relate the components of our strain tensor to the dof-nbr
\code
    std::map<unsigned int,std::pair<unsigned int,unsigned int>> std_map_indicies;
 
\endcode
The point at which the derivative shall be computed: \n
As mentioned previously, we will implement this example for 2D and 3D, hence we once have to set up a strain tensor
and the derivatives for 3D with 6 independent components ...
\code
	if(dim==3)
	{
		eps[0][0] = 1;
		eps[1][1] = 2;
		eps[2][2] = 3;
 
		eps[0][1] = 4;
		eps[0][2] = 5;
		eps[1][2] = 6;
 
 
		eps[0][0].diff(0,6);
		eps[0][1].diff(1,6);
		eps[0][2].diff(2,6);
		eps[1][1].diff(3,6);
		eps[1][2].diff(4,6);
		eps[2][2].diff(5,6);
 
\endcode
By using the map and the following pairs, we have to set up the relation between strain components and dofs only once
and can use the map to access the entries of the list later, without possibly mixing up indices and creating errors.
Please don't be confused, but the dofs in the Wrapper are set up
in a different order that we showed earlier. Earlier: (0,0)-(1,1)-(2,2)-...; Now: (0,0)-(0,1)-(0,2)-...
\code
		std::pair<unsigned int, unsigned int> tmp_pair;
		tmp_pair.first=0; tmp_pair.second=0;
		std_map_indicies[0] = tmp_pair;
 
		tmp_pair.first=0; tmp_pair.second=1;
		std_map_indicies[1] = tmp_pair;
 
		tmp_pair.first=0; tmp_pair.second=2;
		std_map_indicies[2] = tmp_pair;
 
		tmp_pair.first=1; tmp_pair.second=1;
		std_map_indicies[3] = tmp_pair;
 
		tmp_pair.first=1; tmp_pair.second=2;
		std_map_indicies[4] = tmp_pair;
 
		tmp_pair.first=2; tmp_pair.second=2;
		std_map_indicies[5] = tmp_pair;
	}
\endcode
... and once for 2D with just 3 independent components.
\code
	else if(dim==2)
	{
		eps[0][0] = 1;
		eps[1][1] = 2;
 
		eps[0][1] = 4;
 
 
		eps[0][0].diff(0,3);
		eps[0][1].diff(1,3);
		eps[1][1].diff(2,3);
 
        std::pair<unsigned int, unsigned int> tmp_pair;
        tmp_pair.first=0; tmp_pair.second=0;
        std_map_indicies[0] = tmp_pair;
 
        tmp_pair.first=0; tmp_pair.second=1;
        std_map_indicies[1] = tmp_pair;
 
        tmp_pair.first=1; tmp_pair.second=1;
        std_map_indicies[2] = tmp_pair;        
	}
	else
	{
		throw std::runtime_error("only dim==2 or dim==3 allowed");
	}
 
\endcode
Instead of calling the *.diff(*) on the components one-by-one we could also use the following for-loop, so
we also use the map to set the dofs (as we will do in the Wrapper later).
@code
for ( unsigned int x=0; x<((dim==2)?3:6); ++x )
{
	unsigned int i=std_map_indicies[x].first;
	unsigned int j=std_map_indicies[x].second;
	eps[i][j].diff(x,((dim==2)?3:6));
}
@endcode
 
For our slightly more complicated stress equation we need the unit and deviatoric tensors.
We can simply define them by writing the values of the already existing deal.ii functions into newly
defined SymmetricTensors build from fad_doubles.
\code
	SymmetricTensor<2,dim, fad_double> stdTensor_I (( unit_symmetric_tensor<dim,fad_double>()) );
	SymmetricTensor<4,dim, fad_double> stdTensor_Idev ( (deviator_tensor<dim,fad_double>()) );
 
\endcode
With everything set and defined, we can compute our stress \a sigma according to:
\f[ \sigma = \kappa \cdot trace(\varepsilon) \cdot \boldsymbol{I} + 2 \cdot \mu \cdot \varepsilon^{dev} \f]
Here you can see that we can directly multiply the constant and the tensors when kappa is also declared as fad_double
\code
	sigma = kappa * (trace(eps) *  stdTensor_I);
\endcode
We didn't do the same for mu to once again emphasize the difference between constants as double and as fad_double. \n
The remaining code uses a normal double constant.
\code
    SymmetricTensor<2,dim,fad_double> tmp = deviator<dim,fad_double>(symmetrize<dim,fad_double>(eps)); tmp*=(mu*2);
    sigma +=  tmp;
\endcode
The fairly cumbersome computation is caused by the way the operators are set up for tensors out of fad_doubles.
 
\code
	std::cout << "sigma=" << sigma << std::endl;
 
\endcode
Now we want to actually build our tangent modulus called \a C_Sacado that contains all the derivatives and relates
the stress tensor with the strain tensor. \n
The fourth-order tensor \a C_Sacado is our final goal, we don't have to compute anything that is related to Sacado with
this tensor, so we can finally return to our standard SymmetricTensor out of doubles. The latter is necessary to use
the tangent in the actual FE code.
\code
	SymmetricTensor<4,dim> C_Sacado;
 
\endcode
As in Ex2 we access the components of the stress tensor one by one. In order to capture all of them we sum over the
components i and j of the stress tensor.
\code
	for ( unsigned int i=0; i<dim; ++i)
		for ( unsigned int j=0; j<dim; ++j )
		{
			double *derivs = &sigma[i][j].fastAccessDx(0); // Access the derivatives of the (i,j)-th component of \a sigma
 
\endcode
To visually ensure that every stress component has in fact all 6 derivatives for 3D or 3 for 2D, we output the size:
\code
            std::cout<<"size: "<<sigma[i][j].size()<<std::endl;
 
\endcode
We loop over all the dofs. To be able to use this independent of the chosen dimension \a dim, we use a ternary operator
to decide whether we have to loop over 6 derivatives or just 3.
\code
            for(unsigned int x=0;x<((dim==2)?3:6);++x)
            {
                unsigned int k=std_map_indicies[x].first;
                unsigned int l=std_map_indicies[x].second;
 
                if(k!=l)/*Compare to Voigt notation since only SymmetricTensor instead of Tensor*/
                {
                    C_Sacado[i][j][k][l] = 0.5*derivs[x];
                    C_Sacado[i][j][l][k] = 0.5*derivs[x];
                }
                else
                    C_Sacado[i][j][k][l] = derivs[x];
            }            
 
		}
 
\endcode
After resembling the fourth-order tensor, we now have got our tangent saved in \a C_Sacado ready to be used
 
To ensure that Sacado works properly, we can compute the analytical tangent for comparison
\code
	double kappa_d = 5;
	double mu_d = 2;
\endcode
Our stress equation in this example is still simple enough to derive the tangent analytically by hand:
\f[ \overset{4}{C_{analy}} = \kappa \cdot \boldsymbol{I} \otimes \boldsymbol{I} + 2 \cdot \mu \cdot \overset{4}{I^{dev}} \f]
\code
	SymmetricTensor<4,dim> C_analy = kappa_d * outer_product(unit_symmetric_tensor<dim>(), unit_symmetric_tensor<dim>()) + 2* mu_d * deviator_tensor<dim>();
 
 
\endcode
We again define our strain tensor \a eps_d (*_d for standard double in contrast to fad_double)
\code
	SymmetricTensor<2,dim> eps_d;
 
	if(dim==3)
	{
		eps_d[0][0] = 1;
		eps_d[1][1] = 2;
		eps_d[2][2] = 3;
 
		eps_d[0][1] = 4;
		eps_d[0][2] = 5;
		eps_d[2][1] = 6;
 
	}
	else if(dim==2)
	{
		eps_d[0][0] = 1;
		eps_d[1][1] = 2;
 
		eps_d[1][0] = 4;
 
	}
	else
	{
		throw std::runtime_error("only dim==2 or dim==3 allowed");
	}
\endcode
@todo use boldsymbol for tensors

To output the stress tensor we first have to compute it. We do this here via
\f[ \sigma = \overset{4}{C_{analy}} : \varepsilon \f]
The output exactly matched the result obtained with Sacado.
@note Checking the Sacado stress tensor against an analytically computed or otherwise determined stress tensor is absolutely no way to check whether
the tangent computed via Sacado is correct. When we compute the stress tensor with Sacado and for example mix up a + and - sign, this might not matter
at all if the number that is added or subtracted is small. However, for the tangent this nasty sign can be very critical. Just keep in mind: the
tangent has 81 components and the stress tensor just 9, so how does one want to verify 81 variables by comparing 9?

\code
    std::cout << "sigma_analy: " << (C_analy*eps_d) << std::endl;
 
\endcode
That's the reason we compare all the entries in the Sacado and the analytical tensor one by one
\code
	for (unsigned int i=0; i<dim; ++i)
		for ( unsigned int j=0; j<dim; ++j)
			for ( unsigned int k=0; k<dim; ++k)
				for ( unsigned int l=0; l<dim; ++l)
					std::cout << "C_analy["<<i<<"]["<<j<<"]["<<k<<"]["<<l<<"] = " << C_analy[i][j][k][l] << " vs C_Sacado: " << C_Sacado[i][j][k][l] << std::endl;
 
 
\endcode
To simplify the comparison we compute a scalar error as the sum of the absolute differences of each component
\code
	double error_Sacado_vs_analy=0;
	for (unsigned int i=0; i<dim; ++i)
		for ( unsigned int j=0; j<dim; ++j)
			for ( unsigned int k=0; k<dim; ++k)
				for ( unsigned int l=0; l<dim; ++l)
					error_Sacado_vs_analy += std::fabs(C_Sacado[i][j][k][l] - C_analy[i][j][k][l]);
 
 
\endcode
As desired: The numerical error is zero (0 in double precision) and the tensor components are equal
\code
	std::cout << "numerical error: " << error_Sacado_vs_analy << std::endl;
}
 
 
\endcode
@section Ex3B 3B. Example: Using the wrapper for Ex3
\code
void sacado_test_3B ()
{
	std::cout << "Test 3B:" << std::endl;
    const unsigned int dim=3;
 
\endcode
The following declarations are usually input arguments. So you receive the strain tensor and the constants out of doubles.
\code
    SymmetricTensor<2,dim> eps_d;
	eps_d[0][0] = 1;
	eps_d[1][1] = 2;
	eps_d[2][2] = 3;
 
	eps_d[0][1] = 4;
	eps_d[0][2] = 5;
	eps_d[1][2] = 6;
 
	double kappa = 5;
	double mu = 2;
 
\endcode
Now we start working with Sacado: \n
When we use the index notation to compute e.g. our stress we do not need to declare our constants (here kappa, mu) as
fad_double.
 
We declare our strain tensor as the special data type Sacado_Wrapper::SymTensor from the file "Sacado_Wrapper.h"
where this data type was derived from the SymmetricTensor<2,dim,fad_double>.
\code
	 Sacado_Wrapper::SymTensor<dim> eps;
 
\endcode
Next we initialize our Sacado strain tensor with the values of the inputed double strain tensor:
\code
	 eps.init(eps_d);
 
\endcode
We define all the entries in the symmetric tensor \a eps as the dofs. So we can later derive any variable
with respect to the strain tensor \a eps.
\code
	 eps.set_dofs();
 
\endcode
Now we declare our output and auxiliary variables as Sacado-Tensors.
\code
	 SymmetricTensor<2,dim,fad_double> sigma;
 
	 SymmetricTensor<2,dim, fad_double> stdTensor_I (( unit_symmetric_tensor<dim,fad_double>()) );
 
\endcode
Our stress equation is now computed in index notation to simplify the use of the constants and
especially the use of the \a deviator.
\code
	  for ( unsigned int i=0; i<dim; ++i)
		for ( unsigned int j=0; j<dim; ++j )
			sigma[i][j] = kappa * trace(eps) *  stdTensor_I[i][j] + 2. * mu * deviator(eps)[i][j];
 
\endcode
Finally we declare our desired tangent as the fourth order tensor \a C_Sacado and compute the tangent via
the command \a get_tangent.
\code
	 SymmetricTensor<4,dim> C_Sacado;
	 eps.get_tangent(C_Sacado, sigma);
 
\endcode
We could again compare the herein computed tangent with the analytical tangent from Ex2, but as before
the results are fairly boring, because Sacado hits the analytical tangent exactly --- no surprise for such
simple equations.
 
And that's it. By using the Sacado_wrapper we can achieve everything from Ex2 (besides the equations)
with just four lines of code namely:
- eps.init(eps_d);    // To initialize the Sacado strain tensor
- eps.set_dofs();     // To declare the components of eps as the dofs
- eps.get_tangent(*); // To get the tangent
\code
}
 
 
\endcode
@section Ex4 4. Example: Computing derivatives with respect to a tensor and a scalar
\code
void sacado_test_4 ()
{
	std::cout << "Test 4:" << std::endl;
	const unsigned int dim=3;
 
\endcode
The following declarations are usually input arguments. So you receive the strain tensor \q eps_d,
the damage variable \a phi and the constants \a kappa and \a mu out of doubles.
\code
	SymmetricTensor<2,dim> eps_d;
	eps_d[0][0] = 1;
	eps_d[1][1] = 2;
	eps_d[2][2] = 3;
 
	eps_d[0][1] = 4;
	eps_d[0][2] = 5;
	eps_d[1][2] = 6;
 
	double phi_d = 0.3;
 
\endcode
We don't need these constants in the current example.
double kappa = 5;
double mu = 2;
 
 
We set up our strain tensor as in Ex3B.
\code
	 Sacado_Wrapper::SymTensor<dim> eps;
	 Sacado_Wrapper::SW_double<dim> phi;
 
\endcode
Initialize the strain tensor and the damage variable
\code
	 eps.init(eps_d);
	 phi.init(phi_d);
 
\endcode
Set the dofs, where the argument sets the total nbr of dofs (3 or 6 for the sym. tensor and 1 for the double)
\code
//	  eps.set_dofs(eps.n_independent_components+1/*an additional dof for phi*/);
\endcode

In order to also compute derivatives with respect to the scalar \a phi, we add this scalar to our list
of derivatives. Because we have already defined 3 or 6 dofs our additional dof will be placed at the end
of this list. We set this up with the member variable start_index ...
\code
//	  phi.start_index=eps.n_independent_components;
\endcode
and again using the input argument representing the total number of dofs
\code
//	  phi.set_dofs(eps.n_independent_components+1);
 
\endcode
All of the above 3 lines of code are automatically done by the DoFs_summary class. So, to
set our dofs we just create an instance and call set_dofs with our variables containing the desired dofs.
\code
	 Sacado_Wrapper::DoFs_summary<dim> DoFs_summary;
	 DoFs_summary.set_dofs(eps, phi);
 
 
\endcode
Compute the stress tensor and damage variable \a d (here we just use some arbitrary equations for testing): \n
Let us first declare our output (and auxiliary) variables as Sacado data types.
\code
	  SymmetricTensor<2,dim,fad_double> sigma;
	  fad_double d;
\endcode
@todo It would be nice to use the data types from the Sacado_Wrapper for all the Sacado variables. But
somehow the operators (multiply*, ...) seem to cause conflicts again.
 
The actual computation in the following scope uses the exact same equation as your normal computation e. g. via the data type double.
Hence, you could either directly compute your stress, etc. via the Sacado variables or you define
template functions that contain your equations and are either called templated with double or fad_double.
When using the first option, please consider the computation time that is generally higher for a computation
with fad_double than with normal doubles (own experience in a special case: slower by factor 30).
The second option with templates does not suffer these issues.
\code
	  {
	  for ( unsigned int i=0; i<dim; ++i)
	 	for ( unsigned int j=0; j<dim; ++j )
	 		sigma[i][j] = phi * eps[i][j];
\endcode
ToDo: strangely when phi is a fad_double then the multiplication phi * eps works directly without
having to use the index notation
\code
	  std::cout << "sigma=" << sigma << std::endl;
 
	  d = phi*phi + 25 + trace(eps);
	  std::cout << "d=" << d << std::endl;
	  }
 
 
\endcode
Get the tangents
d_sigma / d_eps: SymmetricTensor with respect to SymmetricTensor
\code
	  SymmetricTensor<4,dim> C_Sacado;
	  eps.get_tangent(C_Sacado, sigma);
	  std::cout << "C_Sacado=" << C_Sacado << std::endl;
 
\endcode
Compute the analytical tangent:
\code
	  SymmetricTensor<4,dim> C_analy;
	  C_analy = phi_d * identity_tensor<dim>();
	  std::cout << "C_analy =" << C_analy << std::endl;
 
\endcode
d_d / d_eps: double with respect to SymmetricTensor
\code
	  SymmetricTensor<2,dim> d_d_d_eps;
	  eps.get_tangent(d_d_d_eps, d);
	  std::cout << "d_d_d_eps=" << d_d_d_eps << std::endl;
 
\endcode
d_sigma / d_phi: SymmetricTensor with respect to double
\code
	  SymmetricTensor<2,dim> d_sigma_d_phi;
	  phi.get_tangent(d_sigma_d_phi, sigma);
 	  std::cout << "d_sigma_d_phi=" << d_sigma_d_phi << std::endl;
	  std::cout << "sigma = d_sigma_d_phi * phi = " << d_sigma_d_phi * phi_d << std::endl;
 
\endcode
d_d / d_phi: double with respect to double
\code
	  double d_d_d_phi;
	  phi.get_tangent(d_d_d_phi, d);
 	  std::cout << "d_d_d_phi=" << d_d_d_phi << std::endl;
 
\endcode
And that's it. By using the Sacado_wrapper we can compute derivatives with respect to
a tensor and a scalar at the same time (besides the equations)
in essence with just the following lines of code namely:
- eps.init(eps_d); phi.init(phi_d);   // To initialize the Sacado strain tensor and scalar damage variable
- DoFs_summary.set_dofs(eps, phi);    // To declare the components of eps and phi as the dofs
- eps.get_tangent(*); // To get tangents with respect to eps
- phi.get_tangent(*); // To get tangents with respect to phi
\code
}
 
 
 
\endcode
@section Ex5 5. Example: Using a vector-valued equation
\code
void sacado_test_5 ()
{
    const unsigned int dim=3;
	std::cout << "Test 5:" << std::endl;
    Tensor<1,dim,fad_double> c;
	fad_double a,b;
    unsigned int n_dofs=2;
	a = 1; b = 2;	// at the point (a,b) = (1,2)
	a.diff(0,2);  // Set a to be dof 0, in a 2-dof system.
	b.diff(1,2);  // Set b to be dof 1, in a 2-dof system.
\endcode
c is now a vector with three components
\code
	c[0] = 2*a+3*b;
    c[1] = 4*a+5*b;
    c[2] = 6*a+7*b;
 
\endcode
Access to the derivatives works as before.
\code
    for(unsigned int i=0;i<dim;++i)
    {
        const fad_double &derivs = c[i]; // Access derivatives
        for(unsigned int j=0;j<n_dofs;++j)
        {
            std::cout << "Derivatives at the point (" << a << "," << b << ") for "
            <<i<<"th component wrt "<<j<<"th direction "<< std::endl;
            std::cout << "dc_i/dxj = " << derivs.fastAccessDx(j) << std::endl;            
        }
    }
}
 
 
 
\endcode
@section Ex6 6. Example: First and second derivatives - Scalar equation
The here shown example was copied from https://github.com/trilinos/Trilinos/blob/master/packages/sacado/example/dfad_dfad_example.cpp
and modified to get a first impression on how we can work with first and second derivatives
\code
void sacado_test_6 ()
{
	std::cout << "Test 6:" << std::endl;
\endcode
Define the variables used in the computation (inputs: a, b; output: c; auxiliaries: *) as doubles
\code
	 double a=1;
	 double b=2;
 
\endcode
Number of independent variables (scalar a and b)
\code
	 int num_dofs = 2;
 
\endcode
Define another data type containing even more Sacado data types
@todo try to merge the fad_double data type with this templated data type
\code
	 typedef Sacado::Fad::DFad<double> DFadType;
	 Sacado::Fad::DFad<DFadType> afad(num_dofs, 0, a);
	 Sacado::Fad::DFad<DFadType> bfad(num_dofs, 1, b);
	 Sacado::Fad::DFad<DFadType> cfad;
 
\endcode
Output the variables: We se that the values of \a a and \a b are set but the derivatives have not yet been fully declared
\code
	 std::cout << "afad=" << afad << std::endl;
	 std::cout << "bfad=" << bfad << std::endl;
	 std::cout << "cfad=" << cfad << std::endl;
 
\endcode
Now we set the "inner" derivatives.
\code
	afad.val() = fad_double(num_dofs, 0, a); // set afad.val() as the first dof and init it with the double a
	bfad.val() = fad_double(num_dofs, 1, b);
 
\endcode
Compute function and derivative with AD
\code
	 cfad = 2*afad + std::cos(afad*bfad);
 
\endcode
After this, we output the variables again and see that some additional derivatives have been declared. Furthermore,
\a cfad is filled with the values and derivatives
\code
	 std::cout << "afad=" << afad << std::endl;
	 std::cout << "bfad=" << bfad << std::endl;
	 std::cout << "cfad=" << cfad << std::endl;
 
\endcode
Extract value and derivatives
\code
	 double c_ad = cfad.val().val();       // r
	 double dcda_ad = cfad.dx(0).val();    // dr/da
	 double dcdb_ad = cfad.dx(1).val();    // dr/db
	 double d2cda2_ad = cfad.dx(0).dx(0);  // d^2r/da^2
	 double d2cdadb_ad = cfad.dx(0).dx(1); // d^2r/dadb
	 double d2cdbda_ad = cfad.dx(1).dx(0); // d^2r/dbda
	 double d2cdb2_ad = cfad.dx(1).dx(1);  // d^2/db^2
 
\endcode
Now we can print the actual double value of c and some of the derivatives:
\code
	 std::cout << "c_ad=" << c_ad << std::endl;
	 std::cout << "Derivatives at the point (" << a << "," << b << ")" << std::endl;
	 std::cout << "dc/da = " << dcda_ad << ", dc/db=" << dcdb_ad << std::endl;
	 std::cout << "d²c/da² = " << d2cda2_ad << ", d²c/db²=" << d2cdb2_ad << std::endl;
	 std::cout << "d²c/dadb = " << d2cdadb_ad << ", d²c/dbda=" << d2cdbda_ad << std::endl;
}
 
 
\endcode
@section Ex7 7. Example: First and second derivatives - Using tensor (The full story)
\code
void sacado_test_7 ()
{
    const unsigned int dim=3;
 
	std::cout << "Test 7:" << std::endl;
 
\endcode
Defining the inputs (material parameters, strain tensor)
\code
	 double lambda=1;
	 double mu=2;
	 SymmetricTensor<2,dim, double> eps;
 
	 eps[0][0] = 1.;
	 eps[1][1] = 2.;
	 eps[2][2] = 3.;
 
	 eps[0][1] = 4.;
	 eps[0][2] = 5.;
	 eps[1][2] = 6.;
 
\endcode
Here we skip the one-field example and right away show the equations for a two-field problem
with \a eps and \a phi.
\code
	 double phi=0.3;
 
\endcode
Setup of the map relating the indices (as before)
\code
	 std::map<unsigned int,std::pair<unsigned int,unsigned int>> std_map_indicies;
 
	 std::pair<unsigned int, unsigned int> tmp_pair;
	 tmp_pair.first=0; tmp_pair.second=0;
	 std_map_indicies[0] = tmp_pair;
 
	 tmp_pair.first=0; tmp_pair.second=1;
	 std_map_indicies[1] = tmp_pair;
 
	 tmp_pair.first=0; tmp_pair.second=2;
	 std_map_indicies[2] = tmp_pair;
 
	 tmp_pair.first=1; tmp_pair.second=1;
	 std_map_indicies[3] = tmp_pair;
 
	 tmp_pair.first=1; tmp_pair.second=2;
	 std_map_indicies[4] = tmp_pair;
 
	 tmp_pair.first=2; tmp_pair.second=2;
	 std_map_indicies[5] = tmp_pair;
 
\endcode
Number of independent variables (6 for the tensor and 1 for the scalar phi)
\code
	 const unsigned int nbr_dofs = 6+1;
 
\endcode
Declaring the special data types containing all derivatives
\code
	 typedef Sacado::Fad::DFad<double> DFadType;
	 SymmetricTensor<2,dim, Sacado::Fad::DFad<DFadType> > eps_fad, eps_fad_squared;
	 Sacado::Fad::DFad<DFadType> phi_fad;
 
 
\endcode
Setting the dofs
\code
	 for ( unsigned int x=0; x<6; ++x )
	 {
	 	unsigned int i=std_map_indicies[x].first;
	 	unsigned int j=std_map_indicies[x].second;
		(eps_fad[i][j]).diff( x, nbr_dofs);	// set up the "inner" derivatives
		(eps_fad[i][j]).val() = fad_double(nbr_dofs, x, eps[i][j]); // set up the "outer" derivatives
	 }
 
	 phi_fad.diff( 6, nbr_dofs );
	 phi_fad.val() = fad_double(nbr_dofs, 6, phi); // set up the "outer" derivatives
 
	 std::cout << "eps_fad=" << eps_fad << std::endl;
	 std::cout << "phi_fad=" << phi_fad << std::endl;
 
\endcode
Compute eps² = eps_ij * eps_jk in index notation
\code
	 for ( unsigned int i=0; i<dim; ++i)
		for ( unsigned int k=0; k<dim; ++k )
			for ( unsigned int j=0; j<dim; ++j )
				if ( i>=k )
					eps_fad_squared[i][k] += eps_fad[i][j] * eps_fad[j][k];
 
\endcode
Compute the strain energy density
\code
	 Sacado::Fad::DFad<DFadType> energy;
	 energy = lambda/2. * trace(eps_fad)*trace(eps_fad) + mu * trace(eps_fad_squared) + 25 * phi_fad * trace(eps_fad);
 
\endcode
Give some insight into the storage of the values and derivatives
\code
	 std::cout << "energy=" << energy << std::endl;
 
\endcode
Compute sigma as \f[ \frac{\partial \Psi}{\partial \boldsymbol{\varepsilon}} \f]
\code
	 SymmetricTensor<2,dim> sigma_Sac;
	 for ( unsigned int x=0; x<6; ++x )
	 {
		unsigned int i=std_map_indicies[x].first;
		unsigned int j=std_map_indicies[x].second;
		if ( i!=j )
			sigma_Sac[i][j] = 0.5 * energy.dx(x).val();
		else
			sigma_Sac[i][j] = energy.dx(x).val();
	 }
	 std::cout << "sigma_Sacado=" << sigma_Sac << std::endl;
 
	 double d_energy_d_phi = energy.dx(6).val();
	 std::cout << "d_energy_d_phi=" << d_energy_d_phi << std::endl;
 
\endcode
Analytical stress tensor:
\code
	 SymmetricTensor<2,dim> sigma;
	 sigma = lambda*trace(eps)*unit_symmetric_tensor<dim>() + 2. * mu * eps;
	 std::cout << "analy. sigma=" << sigma << std::endl;
 
 
\endcode
Sacado-Tangent
\code
	 SymmetricTensor<4,dim> C_Sac;
	 for(unsigned int x=0;x<6;++x)
		for(unsigned int y=0;y<6;++y)
		{
			const unsigned int i=std_map_indicies[y].first;
			const unsigned int j=std_map_indicies[y].second;
			const unsigned int k=std_map_indicies[x].first;
			const unsigned int l=std_map_indicies[x].second;
 
			double deriv = energy.dx(x).dx(y); // Access the derivatives of the (i,j)-th component of \a sigma
 
			if ( k!=l && i!=j )
				C_Sac[i][j][k][l] = 0.25* deriv;
			else if(k!=l)/*Compare to Voigt notation since only SymmetricTensor instead of Tensor*/
			{
				C_Sac[i][j][k][l] = 0.5*deriv;
				C_Sac[i][j][l][k] = 0.5*deriv;
			}
			else
				C_Sac[i][j][k][l] = deriv;
		}
 
	 double d2_energy_d_phi_2 = energy.dx(6).dx(6);
	 std::cout << "d2_energy_d_phi_2=" << d2_energy_d_phi_2 << std::endl;
 
	 SymmetricTensor<2,dim> d2_energy_d_eps_d_phi;
 
	 SymmetricTensor<2,dim, Sacado::Fad::DFad<DFadType> > sigma_Sac_full;
	 for ( unsigned int x=0; x<6; ++x )
	 {
		unsigned int i=std_map_indicies[x].first;
		unsigned int j=std_map_indicies[x].second;
		if ( i!=j )
			sigma_Sac_full[i][j] = 0.5 * energy.dx(x);
		else
			sigma_Sac_full[i][j] = energy.dx(x);
	 }
 
	 std::cout << "sigma_Sac_full=" << sigma_Sac_full << std::endl;
	 d2_energy_d_eps_d_phi[0][0] = sigma_Sac_full[0][0].val().dx(6);
	 d2_energy_d_eps_d_phi[1][1] = sigma_Sac_full[1][1].val().dx(6);
	 d2_energy_d_eps_d_phi[2][2] = sigma_Sac_full[2][2].val().dx(6);
	 d2_energy_d_eps_d_phi[0][1] = sigma_Sac_full[0][1].val().dx(6);
	 d2_energy_d_eps_d_phi[0][2] = sigma_Sac_full[0][2].val().dx(6);
	 d2_energy_d_eps_d_phi[1][2] = sigma_Sac_full[1][2].val().dx(6);
 
	 std::cout << "d2_energy_d_eps_d_phi=" << d2_energy_d_eps_d_phi << std::endl;
 
	 SymmetricTensor<2,dim> d2_energy_d_phi_d_eps;
	 d2_energy_d_phi_d_eps[0][0] = energy.dx(6).dx(0);
	 d2_energy_d_phi_d_eps[0][1] = energy.dx(6).dx(1);
	 d2_energy_d_phi_d_eps[0][2] = energy.dx(6).dx(2);
	 d2_energy_d_phi_d_eps[1][1] = energy.dx(6).dx(3);
	 d2_energy_d_phi_d_eps[1][2] = energy.dx(6).dx(4);
	 d2_energy_d_phi_d_eps[2][2] = energy.dx(6).dx(5);
	 std::cout << "d2_energy_d_phi_d_eps=" << d2_energy_d_phi_d_eps << std::endl;
 
 
\endcode
Analytical tangent
\code
	 SymmetricTensor<4,dim> C_analy;
	 C_analy = lambda * outer_product(unit_symmetric_tensor<dim>(), unit_symmetric_tensor<dim>()) + 2. * mu * identity_tensor<dim>();
 
	double error_Sacado_vs_analy=0;
	for (unsigned int i=0; i<dim; ++i)
		for ( unsigned int j=0; j<dim; ++j)
			for ( unsigned int k=0; k<dim; ++k)
				for ( unsigned int l=0; l<dim; ++l)
					error_Sacado_vs_analy += std::fabs(C_Sac[i][j][k][l] - C_analy[i][j][k][l]);
 
	std::cout << "Numerical error=" << error_Sacado_vs_analy << std::endl;
}
 
 
 
\endcode
@section Ex8 8. Example: First and second derivatives - Using the Wrapper
\code
void sacado_test_8 ()
 
{
    const unsigned int dim=3;
 
	std::cout << "Test 8:" << std::endl;
 
\endcode
Defining the inputs (material parameters, strain tensor)
\code
	 double lambda=1;
	 double mu=2;
	 SymmetricTensor<2,dim, double> eps;
	 double phi = 0.3;
 
	 eps[0][0] = 1.;
	 eps[1][1] = 2.;
	 eps[2][2] = 3.;
 
	 eps[0][1] = 4.;
	 eps[0][2] = 5.;
	 eps[1][2] = 6.;
 
\endcode
Declaring the special data types containing all derivatives
\code
	 typedef Sacado::Fad::DFad<double> DFadType;
 
\endcode
Declare the variables \a eps_fad and \a phi_fad as the special Wrapper data types
\code
	 Sacado_Wrapper::SymTensor2<dim> eps_fad;
	 Sacado_Wrapper::SW_double2<dim> phi_fad;
 
\endcode
Declare the summary data type relating all the dofs and initialising them too
\code
	 Sacado_Wrapper::DoFs_summary<dim> DoFs_summary;
	 DoFs_summary.init_set_dofs(eps_fad, eps, phi_fad, phi);
 
\endcode
The variables are outputted to give some insight into the storage of the values (derivatives still trivial).
\code
	 std::cout << "eps_fad=" << eps_fad << std::endl;
	 std::cout << "phi_fad=" << phi_fad << std::endl;
 
\endcode
Compute eps² = eps_ij * eps_jk in index notation
\code
	 SymmetricTensor<2,dim, Sacado::Fad::DFad<DFadType> > eps_fad_squared;
	 for ( unsigned int i=0; i<dim; ++i)
		for ( unsigned int k=0; k<dim; ++k )
			for ( unsigned int j=0; j<dim; ++j )
				if ( i>=k )
					eps_fad_squared[i][k] += eps_fad[i][j] * eps_fad[j][k];
 
\endcode
Compute the strain energy density
\code
	 Sacado::Fad::DFad<DFadType> energy;
	 energy = lambda/2. * trace(eps_fad)*trace(eps_fad) + mu * trace(eps_fad_squared) + 25 * phi_fad * trace(eps_fad);
 
\endcode
The energy is outputted (formatted by hand) to give some insight into the storage of the values and derivatives. \n
energy=399 [ 17.5 32 40 21.5 48 25.5 150 ] \n
				[ 17.5 [ 5 0 0 1 0 1 25 ] 32 [ 0 8 0 0 0 0 0 ] 40 [ 0 0 8 0 0 0 0 ] \n
				21.5 [ 1 0 0 5 0 1 25 ] 48 [ 0 0 0 0 8 0 0 ] 25.5 [ 1 0 0 1 0 5 25 ] \n
\code
	//	 	 	 	150 [ 25 0 0 25 0 25 0 ] ]
 
	 std::cout << "energy=" << energy << std::endl;
 
\endcode
Compute sigma as \f[ \boldsymbol{\sigma} = \frac{\partial \Psi}{\partial \boldsymbol{\varepsilon}} \f]
\code
	 SymmetricTensor<2,dim> sigma_Sac;
	 eps_fad.get_tangent(sigma_Sac, energy);
	 std::cout << "sigma_Sacado=" << sigma_Sac << std::endl;
 
	 double d_energy_d_phi;
	 phi_fad.get_tangent(d_energy_d_phi, energy);
	 std::cout << "d_energy_d_phi=" << d_energy_d_phi << std::endl;
 
 
\endcode
Analytical stress tensor:
\code
	 SymmetricTensor<2,dim> sigma;
	 sigma = lambda*trace(eps)*unit_symmetric_tensor<dim>() + 2. * mu * eps;
	 std::cout << "analy. sigma=" << sigma << std::endl;
 
 
\endcode
Sacado stress tangent (or eps curvature) as \f[ \frac{\partial^2 \Psi}{\partial \boldsymbol{\varepsilon}^2} \f]
\code
	 SymmetricTensor<4,dim> C_Sac;
	 eps_fad.get_curvature(C_Sac, energy);
 
\endcode
Sacado phi curvature as \f[ \frac{\partial^2 \Psi}{\partial \varphi^2} \f]
\code
	 double d2_energy_d_phi_2;
	 phi_fad.get_curvature(d2_energy_d_phi_2, energy);
	 std::cout << "d2_energy_d_phi_2=" << d2_energy_d_phi_2 << std::endl;
 
\endcode
Sacado derivatives \f[ \frac{\partial^2 \Psi}{\partial \boldsymbol{\varepsilon} \partial \varphi} \f]
\code
	 SymmetricTensor<2,dim> d2_energy_d_eps_d_phi;
	 DoFs_summary.get_curvature(d2_energy_d_eps_d_phi, energy, eps_fad, phi_fad);
	 std::cout << "d2_energy_d_eps_d_phi=" << d2_energy_d_eps_d_phi << std::endl;
 
\endcode
Sacado derivatives \f[ \frac{\partial^2 \Psi}{\partial \varphi \partial \boldsymbol{\varepsilon}} \f]
\code
	 SymmetricTensor<2,dim> d2_energy_d_phi_d_eps;
	 DoFs_summary.get_curvature(d2_energy_d_phi_d_eps, energy, phi_fad, eps_fad);
	 std::cout << "d2_energy_d_phi_d_eps=" << d2_energy_d_phi_d_eps << std::endl;
 
\endcode
When you consider the output: \n
d2_energy_d_eps_d_phi=25 0 0 0 25 0 0 0 25 \n
d2_energy_d_phi_d_eps=25 0 0 0 25 0 0 0 25 \n
in detail you will notice that both second derivatives are identical. This compplies with the Schwarz integrability condition (Symmetry of second derivatives)
(ignoring all limitation and requirements), it holds
\f[ \frac{\partial^2 \Psi}{\partial \boldsymbol{\varepsilon} \partial \varphi} = \frac{\partial^2 \Psi}{\partial \varphi \partial \boldsymbol{\varepsilon}}  \f]
 
Analytical stress tangent
\code
	 SymmetricTensor<4,dim> C_analy;
	 C_analy = lambda * outer_product(unit_symmetric_tensor<dim>(), unit_symmetric_tensor<dim>()) + 2. * mu * identity_tensor<dim>();
 
\endcode
Compute the error for the stress tangent
\code
	 double error_Sacado_vs_analy=0;
	 for (unsigned int i=0; i<dim; ++i)
		for ( unsigned int j=0; j<dim; ++j)
			for ( unsigned int k=0; k<dim; ++k)
				for ( unsigned int l=0; l<dim; ++l)
					error_Sacado_vs_analy += std::fabs(C_Sac[i][j][k][l] - C_analy[i][j][k][l]);
	 std::cout << "Numerical error=" << error_Sacado_vs_analy << std::endl;
}
 
 
 
/*
 * The main function just calls all the examples and puts some space between the outputs.
 */
int main ()
{
	sacado_test_scalar ();
 
	std::cout << std::endl;
 
	sacado_test_2 ();
 
	std::cout << std::endl;
 
	sacado_test_3 ();
 
    std::cout << std::endl;
 
	sacado_test_3B ();
 
    std::cout << std::endl;
 
    sacado_test_4();
 
    std::cout << std::endl;
 
    sacado_test_5();
 
    std::cout << std::endl;
 
    sacado_test_6();
 
    std::cout << std::endl;
 
    sacado_test_7();
 
    std::cout << std::endl;
 
    sacado_test_8();
}
\endcode

@section END The End

Hosted via GitHub according to https://goseeky.wordpress.com/2017/07/22/documentation-101-doxygen-with-github-pages/

@}
*/
