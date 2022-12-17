/**
LangevinSimulator Version 1.0

Copyright 2020-2022 Christoph Widder and Fabian Glatzel

Christoph Widder <christoph.widder[at]merkur.uni-freiburg.de>
Fabian Glatzel <fabian.glatzel[at]physik.uni-freiburg.de>

This file is part of LangevinSimulator.

LangevinSimulator is free software: you can redistribute it and/or modify it under the terms of
the GNU General Public License as published by the Free Software Foundation, either
version 3 of the License, or (at your option) any later version.

LangevinSimulator is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with LangevinSimulator.
If not, see <https://www.gnu.org/licenses/>.
**/

#include "KernelMethods.hpp"

#include <cstring>
#include <iostream>
#include <gsl/gsl_permutation.h>
#include <gsl/gsl_linalg.h>
#include <filesystem>

#include "InputOutput.hpp"

using namespace std;
using namespace TensorUtils;

tensor<double,4> KernelMethods::calcLowerBlockTriangularInverse(TensorUtils::tensor<double,4> &src)
{
    size_t num_ts = src.shape[0];
    size_t num_obs = src.shape[1];
    tensor<double,3> diag_inverse({num_ts,num_obs,num_obs});
    for(size_t t=0;t<num_ts;t++)
    {
        for(size_t i=0;i<num_obs;i++)
        {
            for(size_t j=0;j<num_obs;j++)
            {
                diag_inverse(t,i,j) = src(t,i,t,j);
            }
        }
    }
    diag_inverse = matInverse(diag_inverse);
    tensor<double,4> inverse({num_ts,num_ts,num_obs,num_obs},0.0);
    for(size_t t=0;t<num_ts;t++)
    {
        for(size_t i=0;i<num_obs;i++)
        {
            for(size_t j=0;j<num_obs;j++)
            {
                inverse(t,t,i,j) = diag_inverse(t,i,j);
            }
        }
    }
    double dummy;
    gsl_matrix * out = gsl_matrix_alloc(num_obs,num_obs);
    src = src.transpose({0,2,3,1});
    for(size_t s=0;s<num_ts;s++)
    {
        for(size_t t=s+1;t<num_ts;t++)
        {
            gsl_matrix_const_view lhs = gsl_matrix_const_view_array(&src(t,s),(t-s)*num_obs,num_obs);
            gsl_matrix_const_view rhs = gsl_matrix_const_view_array(&inverse(s,s),(t-s)*num_obs,num_obs);
            gsl_blas_dgemm(CblasTrans,CblasNoTrans,-1.0,&lhs.matrix,&rhs.matrix,0.0,out);
            for(size_t i=0;i<num_obs;i++)
            {
                for(size_t j=0;j<num_obs;j++)
                {
                    dummy = 0.0;
                    for(size_t k=0;k<num_obs;k++)
                    {
                        dummy += diag_inverse(t,i,k)*gsl_matrix_get(out,k,j);
                    }
                    inverse(s,t,i,j) = dummy;
                }
            }
        }
    }
    gsl_matrix_free(out);
    inverse = inverse.transpose({1,2,0,3});
    src = src.transpose({0,3,1,2});
    return inverse;
}

void KernelMethods::calcDiagInverts(
	gsl_matrix const* corr,
	gsl_matrix **diag_inverts)
{
	size_t const num_obs {diag_inverts[0]->size1};
	size_t const num_ts {corr->size1 / num_obs};
	gsl_permutation *permutation = gsl_permutation_alloc(num_obs);
	gsl_matrix *temp = gsl_matrix_alloc(num_obs, num_obs);
	int signum;

	for(size_t i=0; i!=num_ts; ++i){
		gsl_matrix_const_view view = gsl_matrix_const_submatrix(corr, i*num_obs, i*num_obs, num_obs, num_obs);
		gsl_matrix_memcpy(temp, &(view.matrix));
		gsl_linalg_LU_decomp(temp, permutation, &signum);
		gsl_linalg_LU_invert(temp, permutation, diag_inverts[i]);
	}
	gsl_matrix_free(temp);
	gsl_permutation_free(permutation);
	return;
};

void KernelMethods::calcDerivative(
	gsl_matrix const* mat,
	size_t const num_ts,
	size_t const num_obs,
	gsl_matrix* deriv,
	double const dt,
	bool const firstTime)
{
	double const inv_dt = 1. / dt;
	double const inv_2dt = inv_dt / 2.;
	size_t t1;
	size_t t2;
	int i;
	int j;
	if(firstTime) {
		// Calculate one-sided derivative for first and last row
		for(t2=0; t2!=num_ts; ++t2) {
			i = num_obs*t2;
			gsl_matrix_const_view view_mat0 = gsl_matrix_const_submatrix(mat, 0, i, num_obs, num_obs);
			gsl_matrix_const_view view_mat1 = gsl_matrix_const_submatrix(mat, 1 * num_obs , i, num_obs, num_obs);
			gsl_matrix_view view_deriv = gsl_matrix_submatrix(deriv, 0, i, num_obs, num_obs);
			gsl_matrix_memcpy(&view_deriv.matrix, &view_mat1.matrix);
			gsl_matrix_sub(&view_deriv.matrix, &view_mat0.matrix);
			gsl_matrix_scale(&view_deriv.matrix, inv_dt);

			gsl_matrix_const_view view_mat2 = gsl_matrix_const_submatrix(mat, (num_ts - 2) * num_obs, i, num_obs, num_obs);
			gsl_matrix_const_view view_mat3 = gsl_matrix_const_submatrix(mat, (num_ts - 1) * num_obs, i, num_obs, num_obs);
			view_deriv = gsl_matrix_submatrix(deriv, (num_ts - 1) * num_obs, i, num_obs, num_obs);
			gsl_matrix_memcpy(&view_deriv.matrix, &view_mat3.matrix);
			gsl_matrix_sub(&view_deriv.matrix, &view_mat2.matrix);
			gsl_matrix_scale(&view_deriv.matrix, inv_dt);
		}
		// Calculate center-space derivatives for other rows
		for(t1=1; t1!=num_ts-1; ++t1) {
			j = num_obs*t1;
			for(t2=0; t2!=num_ts; ++t2) {
				i = num_obs*t2;
				gsl_matrix_const_view view_mat0 = gsl_matrix_const_submatrix(mat, j-num_obs, i, num_obs, num_obs);
				gsl_matrix_const_view view_mat1 = gsl_matrix_const_submatrix(mat, j+num_obs, i, num_obs, num_obs);
				gsl_matrix_view view_deriv = gsl_matrix_submatrix(deriv, j, i, num_obs, num_obs);
				gsl_matrix_memcpy(&view_deriv.matrix, &view_mat1.matrix);
				gsl_matrix_sub(&view_deriv.matrix, &view_mat0.matrix);
				gsl_matrix_scale(&view_deriv.matrix, inv_2dt);
			}
		}
	} else {
		// Calculate one-sided derivative for first and last column
		for(t1=0; t1!=num_ts; ++t1) {
			i = num_obs*t1;
			gsl_matrix_const_view view_mat0 = gsl_matrix_const_submatrix(mat, i, 0, num_obs, num_obs);
			gsl_matrix_const_view view_mat1 = gsl_matrix_const_submatrix(mat, i, 1 * num_obs, num_obs, num_obs);
			gsl_matrix_view view_deriv = gsl_matrix_submatrix(deriv, i, 0, num_obs, num_obs);
			gsl_matrix_memcpy(&view_deriv.matrix, &view_mat1.matrix);
			gsl_matrix_sub(&view_deriv.matrix, &view_mat0.matrix);
			gsl_matrix_scale(&view_deriv.matrix, inv_dt);

			gsl_matrix_const_view view_mat2 = gsl_matrix_const_submatrix(mat, i, (num_ts - 2) * num_obs, num_obs, num_obs);
			gsl_matrix_const_view view_mat3 = gsl_matrix_const_submatrix(mat, i, (num_ts - 1) * num_obs, num_obs, num_obs);
			view_deriv = gsl_matrix_submatrix(deriv, i, (num_ts - 1) * num_obs, num_obs, num_obs);
			gsl_matrix_memcpy(&view_deriv.matrix, &view_mat3.matrix);
			gsl_matrix_sub(&view_deriv.matrix, &view_mat2.matrix);
			gsl_matrix_scale(&view_deriv.matrix, inv_dt);
		}
		// Calculate center-space derivatives for other columns
		for(t1=0; t1!=num_ts; ++t1) {
			j = num_obs*t1;
			for(t2=1; t2!=num_ts-1; ++t2) {
				i = num_obs*t2;
				gsl_matrix_const_view view_mat0 = gsl_matrix_const_submatrix(mat, j, i-num_obs, num_obs, num_obs);
				gsl_matrix_const_view view_mat1 = gsl_matrix_const_submatrix(mat, j, i+num_obs, num_obs, num_obs);
				gsl_matrix_view view_deriv = gsl_matrix_submatrix(deriv, j, i, num_obs, num_obs);
				gsl_matrix_memcpy(&view_deriv.matrix, &view_mat1.matrix);
				gsl_matrix_sub(&view_deriv.matrix, &view_mat0.matrix);
				gsl_matrix_scale(&view_deriv.matrix, inv_2dt);
			}
		}
	}
	return;
};


void KernelMethods::calcS0(
		gsl_matrix const* corr,
		gsl_matrix* temp,
		gsl_matrix** diag_inverts,
		double const dt,
		gsl_matrix* S0)
{
	size_t t1;
	size_t t2;
	int i;
	int j;
	size_t const num_obs {diag_inverts[0]->size1};
	size_t const num_ts {corr->size1 / num_obs};
	// false -> derivative with respect to second time coordinate
	KernelMethods::calcDerivative(corr, num_ts, num_obs, temp, dt, false);
	for(t1=0; t1!=num_ts; ++t1) {
		i = num_obs*t1;
		for(t2=0; t2!=num_ts; ++t2) {
			j = num_obs*t2;
			gsl_matrix_view view_mat0 = gsl_matrix_submatrix(temp, i, j, num_obs, num_obs);
			gsl_matrix_view view_mat1 = gsl_matrix_submatrix(S0, i, j, num_obs, num_obs);
			gsl_blas_dgemm(
				CblasNoTrans, CblasNoTrans, 1.0,
				&view_mat0.matrix, diag_inverts[t2], 0.0, &view_mat1.matrix);
		}
	}
};


void KernelMethods::calcj(
	gsl_matrix const* corr,
	gsl_matrix* temp,
	gsl_matrix** diag_inverts,
	const double dt,
	gsl_matrix* j)
{
	size_t t1;
	size_t t2;
	int i;
	int k;
	size_t const num_obs {diag_inverts[0]->size1};
	size_t const num_ts {corr->size1 / num_obs};
	for(t2=0; t2!=num_ts; ++t2) {
		k = num_obs*t2;
		gsl_matrix_const_view view_mat3 = gsl_matrix_const_submatrix(corr, k, k, num_obs, num_obs);
		for(t1=0; t1!=num_ts; ++t1) {
			i = num_obs*t1;

			gsl_matrix_const_view view_mat0 = gsl_matrix_const_submatrix(corr, i, k, num_obs, num_obs);
			gsl_matrix_view view_mat1 = gsl_matrix_submatrix(j, i, k, num_obs, num_obs);

			gsl_matrix_memcpy(&view_mat1.matrix, &view_mat0.matrix);
			gsl_matrix_scale(&view_mat1.matrix, -1.);
			gsl_matrix_add(&view_mat1.matrix, &view_mat3.matrix);
		}
	}
	KernelMethods::calcDerivative(j, num_ts, num_obs, temp, dt, false);
	for(t1=0; t1!=num_ts; ++t1) {
		i = num_obs*t1;
		for(t2=0; t2!=num_ts; ++t2) {
			k = num_obs*t2;
			gsl_matrix_view view_mat0 = gsl_matrix_submatrix(temp, i, k, num_obs, num_obs);
			gsl_matrix_view view_mat1 = gsl_matrix_submatrix(j, i, k, num_obs, num_obs);
			gsl_blas_dgemm(
				CblasNoTrans, CblasNoTrans, 1.0,
				&view_mat0.matrix, diag_inverts[t2], 0.0, &view_mat1.matrix);
		}
	}
};


void KernelMethods::splitIntoTriangular(
	gsl_matrix const* mat,
	int const num_ts,
	int const num_obs,
	gsl_matrix* lower,
	gsl_matrix* upper,
	bool const flipLower)
{
	int t1;
	int t2;
	int i;
	int j;

	if(!flipLower){
		for(t1=0; t1!=num_ts; ++t1) {
			i = t1 * num_obs;
			// t1 > t2
			for(t2=0; t2!=t1; ++t2) {
				j = t2 * num_obs;
				gsl_matrix_const_view view_mat = gsl_matrix_const_submatrix(mat, i, j, num_obs, num_obs);
				gsl_matrix_view view_upper = gsl_matrix_submatrix(upper, i, j, num_obs, num_obs);
				gsl_matrix_view view_lower = gsl_matrix_submatrix(lower, i, j, num_obs, num_obs);

				gsl_matrix_memcpy(&view_lower.matrix, &view_mat.matrix);
				gsl_matrix_set_zero(&view_upper.matrix);
			}
			// t1 == t2
			j = t1 * num_obs;
			gsl_matrix_const_view view_mat = gsl_matrix_const_submatrix(mat, i, j, num_obs, num_obs);
			gsl_matrix_view view_upper = gsl_matrix_submatrix(upper, i, j, num_obs, num_obs);
			gsl_matrix_view view_lower = gsl_matrix_submatrix(lower, i, j, num_obs, num_obs);

			gsl_matrix_memcpy(&view_lower.matrix, &view_mat.matrix);
			gsl_matrix_memcpy(&view_upper.matrix, &view_lower.matrix);
			gsl_matrix_scale(&view_lower.matrix, 0.5);
			gsl_matrix_scale(&view_upper.matrix, 0.5);
			// t1 < t2
			for(t2=t1+1; t2!=num_ts; ++t2) {
				j = t2 * num_obs;
				gsl_matrix_const_view view_mat = gsl_matrix_const_submatrix(mat, i, j, num_obs, num_obs);
				gsl_matrix_view view_upper = gsl_matrix_submatrix(upper, i, j, num_obs, num_obs);
				gsl_matrix_view view_lower = gsl_matrix_submatrix(lower, i, j, num_obs, num_obs);

				gsl_matrix_memcpy(&view_upper.matrix, &view_mat.matrix);
				gsl_matrix_set_zero(&view_lower.matrix);
			}
		}
	} else {
		for(t1=0; t1!=num_ts; ++t1) {
			i = t1 * num_obs;
			// t1 > t2
			for(t2=0; t2!=t1; ++t2) {
				j = t2 * num_obs;
				gsl_matrix_const_view view_mat = gsl_matrix_const_submatrix(mat, i, j, num_obs, num_obs);
				gsl_matrix_view view_upper = gsl_matrix_submatrix(upper, i, j, num_obs, num_obs);
				gsl_matrix_view view_lower = gsl_matrix_submatrix(lower, i, j, num_obs, num_obs);

				gsl_matrix_memcpy(&view_lower.matrix, &view_mat.matrix);
				gsl_matrix_scale(&view_lower.matrix, -1);
				gsl_matrix_set_zero(&view_upper.matrix);
			}
			// t1 == t2
			j = t1 * num_obs;
			gsl_matrix_const_view view_mat = gsl_matrix_const_submatrix(mat, i, j, num_obs, num_obs);
			gsl_matrix_view view_upper = gsl_matrix_submatrix(upper, i, j, num_obs, num_obs);
			gsl_matrix_view view_lower = gsl_matrix_submatrix(lower, i, j, num_obs, num_obs);

			gsl_matrix_memcpy(&view_lower.matrix, &view_mat.matrix);
			gsl_matrix_memcpy(&view_upper.matrix, &view_lower.matrix);
			gsl_matrix_scale(&view_lower.matrix, 0.5);
			gsl_matrix_scale(&view_upper.matrix, 0.5);
			gsl_matrix_scale(&view_lower.matrix, -1);
			// t1 < t2
			for(t2=t1+1; t2!=num_ts; ++t2) {
				j = t2 * num_obs;
				gsl_matrix_const_view view_mat = gsl_matrix_const_submatrix(mat, i, j, num_obs, num_obs);
				gsl_matrix_view view_upper = gsl_matrix_submatrix(upper, i, j, num_obs, num_obs);
				gsl_matrix_view view_lower = gsl_matrix_submatrix(lower, i, j, num_obs, num_obs);

				gsl_matrix_memcpy(&view_upper.matrix, &view_mat.matrix);
				gsl_matrix_set_zero(&view_lower.matrix);
			}
		}
	}
};


void KernelMethods::calcJ(
	gsl_matrix* S0,
	gsl_matrix* j,
	gsl_matrix* temp1,
	gsl_matrix* temp2,
	gsl_matrix* J,
	int const num_ts,
	int const num_obs,
	double const dt)
{
	int i;
	gsl_matrix_scale(S0, dt);
	KernelMethods::splitIntoTriangular(S0, num_ts, num_obs, temp1, temp2, true);
	// Add one to diagonal elements
	for(i=0; i!=num_ts*num_obs; ++i) {
		gsl_matrix_set(temp1, i, i, gsl_matrix_get(temp1, i, i) + 1.0);
		gsl_matrix_set(temp2, i, i, gsl_matrix_get(temp2, i, i) + 1.0);
	}

	if(num_obs>1)
    {
        tensor<double,4> buffer({size_t(num_ts),size_t(num_obs),size_t(num_ts),size_t(num_obs)});
        buffer << *temp1->data;
        calcLowerBlockTriangularInverse(buffer) >> *J->data;   // J is now S_lower!!!
        buffer << *temp2->data;
        buffer = buffer.transpose({2,3,0,1});
        calcLowerBlockTriangularInverse(buffer).transpose({2,3,0,1}) >> *temp1->data;   // temp1 is now S_upper!!!
    }
    else
    {
        gsl_matrix_set_identity(J);
        gsl_blas_dtrsm(CblasLeft, CblasLower, CblasNoTrans, CblasNonUnit, 1.0, temp1, J);
        gsl_matrix_set_identity(temp1);
        gsl_blas_dtrsm(CblasLeft, CblasUpper, CblasNoTrans, CblasNonUnit, 1.0, temp2, temp1);
    }

	KernelMethods::splitIntoTriangular(j, num_ts, num_obs, S0, temp2, false);  // S0 is now j_lower, temp2 is now j_upper!!!

	gsl_blas_dgemm(
		CblasNoTrans, CblasNoTrans, 1.0,
		S0, J, 0.0, j);  // j is now J_lower!!!
	gsl_blas_dgemm(
		CblasNoTrans, CblasNoTrans, 1.0,
		temp2, temp1, 0.0, J);  // J is now J_upper!!!

	gsl_matrix_add(J, j);  // J is now J!!!

};


void KernelMethods::getMemoryKernel(
    gsl_matrix* K,
    gsl_matrix* corr,
    size_t const num_ts,
    size_t const num_obs,
    double const dt)
{
    size_t num_tot = num_ts * num_obs;
    gsl_matrix* S0 = gsl_matrix_alloc(num_tot, num_tot);
    gsl_matrix* j = gsl_matrix_alloc(num_tot, num_tot);
    gsl_matrix* J = gsl_matrix_alloc(num_tot, num_tot);
    gsl_matrix **diag_inverts = new gsl_matrix*[num_ts];
    for(size_t t1=0; t1!=num_ts; ++t1) {
        diag_inverts[t1] = gsl_matrix_alloc(num_obs, num_obs);
    }

    KernelMethods::calcDiagInverts(corr, diag_inverts);
    KernelMethods::calcS0(corr, K, diag_inverts, dt, S0);
    KernelMethods::calcj(corr, K, diag_inverts, dt, j);
    // K and corr are used as auxilliary matrices and anything but J is destroyed
    KernelMethods::calcJ(S0, j, K, corr, J, num_ts, num_obs, dt);
    KernelMethods::calcDerivative(J, num_ts, num_obs, K, dt, true);

    gsl_matrix_free(S0);
    gsl_matrix_free(j);
    gsl_matrix_free(J);
    for(size_t t1=0; t1!=num_ts; ++t1) {
        gsl_matrix_free(diag_inverts[t1]);
    }
    delete[] diag_inverts;
}

tensor<double,3> KernelMethods::getMemoryKernel(tensor<double,3> &correlation, double dt)
{
    size_t shape0 = correlation.shape[0];
    size_t num_ts = (shape0+1)/2;
    size_t num_obs = correlation.shape[1];

    tensor<double,2> inverse({num_obs,num_obs});
    gsl_matrix *lu = gsl_matrix_alloc(num_obs, num_obs);
    gsl_permutation *permutation = gsl_permutation_alloc(num_obs);
    int signum;
    gsl_matrix *inv = gsl_matrix_alloc(num_obs, num_obs);
    for(size_t i=0; i<num_obs; i++)
    {
        for(size_t j=0; j<num_obs; j++)
        {
            gsl_matrix_set(lu,i,j,correlation(num_ts-1,i,j));
        }
    }
    gsl_linalg_LU_decomp(lu, permutation, &signum);
    gsl_linalg_LU_invert(lu, permutation, inv);
    inverse << *inv->data;
    gsl_permutation_free(permutation);
    gsl_matrix_free(lu);
    gsl_matrix_free(inv);

    tensor<double,3> diff;
    diff=diffFront(correlation,dt);
    tensor<double, 3> j0({shape0,num_obs,num_obs},0.0);
    for(size_t t=0; t<shape0;t++)
    {
        for(size_t i=0; i<num_obs; i++)
        {
            for(size_t j=0; j<num_obs; j++)
            {
                for(size_t k=0; k<num_obs; k++)
                {
                    j0(t,i,j) += diff(t,i,k)*inverse(k,j);
                }
            }
        }
    }
    diff.clear();

    tensor<double,3> S_lower({num_ts,num_obs,num_obs});
    tensor<double,3> S_upper({num_ts,num_obs,num_obs});
    S_lower << j0(num_ts-1);
    S_upper << j0(0);
    S_lower *= +1*dt; // S = -dt*S0
    S_upper *= -1*dt; // S = +dt*S0
    for(size_t i=0; i<num_obs; i++)
    {
        for(size_t j=0; j<num_obs; j++)
        {
            S_lower(0,i,j) *= 0.5;
            S_upper(num_ts-1,i,j) *= 0.5;
        }
    }
    for(size_t i=0; i<num_obs; i++)
    {
        S_lower(0,i,i) += 1.0; // add identity
        S_upper(num_ts-1,i,i) += 1.0; // add identity
    } // S = 1-\pm*dt*S_0

    lu = gsl_matrix_alloc(num_obs, num_obs);
    permutation = gsl_permutation_alloc(num_obs);
    inv = gsl_matrix_alloc(num_obs, num_obs);

    tensor<double,2> inverse_lower({num_obs,num_obs});
    for(size_t i=0; i<num_obs; i++)
    {
        for(size_t j=0; j<num_obs; j++)
        {
            gsl_matrix_set(lu,i,j,S_lower(0,i,j));
        }
    }
    gsl_linalg_LU_decomp(lu, permutation, &signum);
    gsl_linalg_LU_invert(lu, permutation, inv);
    inverse_lower << *inv->data;

    tensor<double,2> inverse_upper({num_obs,num_obs});
    for(size_t i=0; i<num_obs; i++)
    {
        for(size_t j=0; j<num_obs; j++)
        {
            gsl_matrix_set(lu,i,j,S_upper(num_ts-1,i,j));
        }
    }
    gsl_linalg_LU_decomp(lu, permutation, &signum);
    gsl_linalg_LU_invert(lu, permutation, inv);
    inverse_upper << *inv->data;

    gsl_permutation_free(permutation);
    gsl_matrix_free(lu);
    gsl_matrix_free(inv);

    tensor<double, 3> J({shape0,num_obs,num_obs},0.0);
    tensor<double,2> buffer({num_obs,num_obs});
    for(size_t tau=0;tau<num_ts;tau++)
    {
        buffer.init(0.0);
        for(size_t r=1;r<=tau;r++)
        {
            for(size_t i=0;i<num_obs;i++)
            {
                for(size_t j=0;j<num_obs;j++)
                {
                    for(size_t k=0;k<num_obs;k++)
                    {
                        buffer(i,j) += J(num_ts-1+tau-r,i,k)*S_lower(r,k,j); // RUN-TIME CRITICAL
                    }
                }
            }
        }
        for(size_t i=0;i<num_obs;i++)
        {
            for(size_t j=0;j<num_obs;j++)
            {
                J(num_ts-1+tau,i,j) = 0.0;
                for(size_t k=0;k<num_obs;k++)
                {
                    J(num_ts-1+tau,i,j) += (j0(num_ts-1+tau,i,k)-buffer(i,k))*inverse_lower(k,j);
                }
            }
        }
    }
    S_lower.clear();
    for(size_t tau=0;tau<num_ts;tau++)
    {
        buffer.init(0.0);
        for(size_t r=1;r<=tau;r++)
        {
            for(size_t i=0;i<num_obs;i++)
            {
                for(size_t j=0;j<num_obs;j++)
                {
                    for(size_t k=0;k<num_obs;k++)
                    {
                        buffer(i,j) += J(num_ts-1-(tau-r),i,k)*S_upper(num_ts-1-r,k,j); // RUN-TIME CRITICAL
                    }
                }
            }
        }
        for(size_t i=0;i<num_obs;i++)
        {
            for(size_t j=0;j<num_obs;j++)
            {
                J(num_ts-1-tau,i,j) = 0.0;
                for(size_t k=0;k<num_obs;k++)
                {
                    J(num_ts-1-tau,i,j) += (j0(num_ts-1-tau,i,k)-buffer(i,k))*inverse_upper(k,j);
                }
            }
        }
    }
    S_upper.clear();
    j0.clear();

    tensor<double, 3> K;
    K=diffFront(J,dt);

    return K;
}

tensor<double,3> KernelMethods::diffTrajectories(tensor<double,3> &trajectories, double dt, bool darboux_sum)
{
    size_t num_traj = trajectories.shape[0];
    size_t num_ts = trajectories.shape[1];
    size_t num_obs   = trajectories.shape[2];
    tensor<double,3> diff(trajectories.shape,0.0);
    if(darboux_sum)
    {
        double dummy = 1.0/dt;
        for(size_t i=0; i<num_traj; i++)
        {
            for(size_t j=0; j+1<num_ts; j++)
            {
                for(size_t k=0; k<num_obs; k++)
                {
                    diff(i,j,k) = (trajectories(i,j+1,k)-trajectories(i,j,k))*dummy;
                }
            }
        }
    }
    else
    {
        for(size_t i=0; i<num_traj; i++)
        {
            for(size_t k=0; k<num_obs; k++)
            {
                diff(i,0,k) = (trajectories(i,1,k)-trajectories(i,0,k))/dt;
            }
            for(size_t j=1; j+1<num_ts; j++)
            {
                for(size_t k=0; k<num_obs; k++)
                {
                    diff(i,j,k) = (trajectories(i,j+1,k)-trajectories(i,j-1,k))/(2*dt);
                }
            }
            for(size_t k=0; k<num_obs; k++)
            {
                diff(i,num_ts-1,k) = (trajectories(i,num_ts-1,k)-trajectories(i,num_ts-2,k))/dt;
            }
        }
    }
    return diff;
};

tensor<double,3> KernelMethods::diffFront(tensor<double,3> &corr_diag, double dt)
{
    size_t num_ts = corr_diag.shape[0];
    size_t num_obs   = corr_diag.shape[1];
    tensor<double,3> diff(corr_diag.shape);
    double dummy = 0.5/dt;
    for(size_t k=0; k<num_obs; k++)
    {
        for(size_t l=0; l<num_obs; l++)
        {
            diff(0,k,l) = (corr_diag(1,k,l)-corr_diag(0,k,l))*2*dummy;
        }
    }
    for(size_t t=1; t+1<num_ts; t++)
    {
        for(size_t k=0; k<num_obs; k++)
        {
            for(size_t l=0; l<num_obs; l++)
            {
                diff(t,k,l) = (corr_diag(t+1,k,l)-corr_diag(t-1,k,l))*dummy;
            }
        }
    }
    for(size_t k=0; k<num_obs; k++)
    {
        for(size_t l=0; l<num_obs; l++)
        {
            diff(num_ts-1,k,l) = (corr_diag(num_ts-1,k,l)-corr_diag(num_ts-2,k,l))*2*dummy;
        }
    }
    return diff;
};

tensor<double,3> KernelMethods::matInverse(tensor<double,3> &mat)
{
    size_t num_obs = mat.shape[1];
    tensor<double,3> result(mat.shape);

    gsl_matrix *lu = gsl_matrix_alloc(num_obs, num_obs);
    gsl_permutation *permutation = gsl_permutation_alloc(num_obs);
    int signum;
    gsl_matrix *inv = gsl_matrix_alloc(num_obs, num_obs);
    for(size_t n=0; n<result.shape[0]; n++)
    {
        // get LU decomposition
        for(size_t i=0; i<num_obs; i++)
        {
            for(size_t j=0; j<num_obs; j++)
            {
                gsl_matrix_set(lu,i,j,mat(n,i,j));
            }
        }
        gsl_linalg_LU_decomp(lu, permutation, &signum);

        // get inverse
        gsl_linalg_LU_invert(lu, permutation, inv);

        for(size_t i=0; i<num_obs; i++)
        {
            for(size_t j=0; j<num_obs; j++)
            {
                result(n,i,j) = gsl_matrix_get(inv,i,j);
            }
        }
    }

    gsl_permutation_free(permutation);
    gsl_matrix_free(lu);
    gsl_matrix_free(inv);

    return result;
}

tensor<double,2> KernelMethods::getDrift(tensor<double,3> &correlation, double dt)
{
    size_t num_obs = correlation.shape[1];
    size_t num_ts = (correlation.shape[0]-1)/2;

    // diff
    tensor<double,2> diff({num_obs,num_obs});
    for(size_t k=0;k<num_obs;k++)
    {
        for(size_t l=0;l<num_obs;l++)
        {
            diff(k,l)=(correlation(num_ts,k,l)-correlation(num_ts-2,l,k))/(2*dt);
        }
    }

    // invert
    tensor<double,2> inverse({num_obs,num_obs});
    gsl_matrix *lu = gsl_matrix_alloc(num_obs, num_obs);
    gsl_permutation *permutation = gsl_permutation_alloc(num_obs);
    int signum;
    gsl_matrix *inv = gsl_matrix_alloc(num_obs, num_obs);
    for(size_t i=0; i<num_obs; i++)
    {
        for(size_t j=0; j<num_obs; j++)
        {
            gsl_matrix_set(lu,i,j,correlation(num_ts-1,i,j));
        }
    }
    gsl_linalg_LU_decomp(lu, permutation, &signum);
    gsl_linalg_LU_invert(lu, permutation, inv);
    inverse << *inv->data;
    gsl_permutation_free(permutation);
    gsl_matrix_free(lu);
    gsl_matrix_free(inv);

    tensor<double,2> drift({num_obs,num_obs},0.0);
    for(size_t i=0; i<num_obs; i++)
    {
        for(size_t j=0; j<num_obs; j++)
        {
            for(size_t k=0; k< num_obs; k++)
            {
                drift(i,j) += diff(i,k)*inverse(k,j);
            }
        }
    }
    return drift;
}

tensor<double,3> KernelMethods::getDrift(tensor<double,4> &correlation, double dt)
{
    size_t num_ts = correlation.shape[0];
    size_t num_obs = correlation.shape[1];
    tensor<double,3> diff_diag({num_ts,num_obs,num_obs});
    tensor<double,3> diag_inverse(diff_diag.shape);
    for(size_t t=0;t<num_ts;t++)
    {
        for(size_t k=0;k<num_obs;k++)
        {
            for(size_t l=0;l<num_obs;l++)
            {
                diff_diag(t,k,l)=correlation(t,k,t,l);
            }
        }
    }
    diag_inverse=diff_diag;
    diff_diag=diffFront(diff_diag,dt);
    diag_inverse=matInverse(diag_inverse);

    tensor<double,3> drift(diff_diag.shape);
    double dummy;
    for(size_t t=0; t<num_ts; t++)
    {
        for(size_t i=0; i<num_obs; i++)
        {
            for(size_t j=0; j<num_obs; j++)
            {
                dummy = 0.0;
                for(size_t k=0; k< num_obs; k++)
                {
                    dummy += diff_diag(t,i,k)*diag_inverse(t,k,j);
                }
                drift(t,i,j) = dummy;
            }
        }
    }
    return drift;
}

tensor<double,3> KernelMethods::getFluctuatingForce(
    tensor<double,4> &kernel,
    tensor<double,3> &drift,
    tensor<double,3> &trajectories,
    tensor<double,1> &times,
    bool darboux_sum)
{
    double dt = times[1]-times[0];

    size_t num_traj = trajectories.shape[0];
    size_t num_ts = trajectories.shape[1];
    size_t num_obs = trajectories.shape[2];

    // get dA/dt
    tensor<double,3> diff_traj = diffTrajectories(trajectories, dt, darboux_sum);

    // subtract drift * A
    double dummy;
    for(size_t n=0; n<num_traj; n++)
    {
        for(size_t t=0; t<num_ts; t++)
        {
            for(size_t i=0; i <num_obs; i++)
            {
                dummy = 0.0;
                for(size_t k=0; k<num_obs; k++)
                {
                    dummy += drift(t,i,k)*trajectories(n,t,k);  // = dA/dt - drift*A
                }
                diff_traj(n,t,i) -= dummy;
            }
        }
    }
    drift.clear();

    // subtract memory part
    tensor<double,3> buffer;
    if(darboux_sum)
    {
        trajectories=trajectories.transpose({1,2,0});
        kernel=kernel.transpose({0,1,3,2});
        buffer.alloc({num_ts,num_obs,num_traj},0.0);
        gsl_matrix * out = gsl_matrix_alloc(num_obs,num_traj);
        for(size_t t=1;t<num_ts;t++)
        {
            gsl_matrix_const_view kernel_t = gsl_matrix_const_view_array(&kernel(t),t*num_obs,num_obs);
            gsl_matrix_const_view traj = gsl_matrix_const_view_array(&trajectories[0],t*num_obs,num_traj);
            gsl_blas_dgemm(CblasTrans,CblasNoTrans,1.0,&kernel_t.matrix,&traj.matrix,0.0,out);
            memcpy(&buffer(t),out->data,num_obs*num_traj*sizeof(double));
        }
        gsl_matrix_free(out);
        buffer=buffer.transpose({2,0,1});
        buffer *=dt;
        trajectories = trajectories.transpose({2,0,1});
        diff_traj -= buffer; // = dA/dt-drift*A-SUM_t2 K(.,t2)*A(t2)
    }
    else
    {
        buffer.alloc({num_traj,num_ts,num_obs});
        const size_t k_incr_0 = kernel.incr[0];
        const size_t k_incr_1 = kernel.incr[1];
        const size_t k_incr_2 = kernel.incr[2];
        const size_t traj_incr_0 = trajectories.incr[0];
        const size_t traj_incr_1 = trajectories.incr[1];

        for(size_t n=0; n<num_traj; n++)
        {
            for(size_t t1=0; t1<num_ts; t1++)
            {
                for(size_t i=0; i<num_obs;i++)
                {
                    dummy = 0.0;
                    size_t t2=0;
                    while(t2+1<t1)
                    {
                        for(size_t k=0; k<num_obs; k++)
                        {
                            dummy += kernel[k_incr_0*t1+k_incr_1*t2+k_incr_2*i+k]*trajectories[traj_incr_0*n+traj_incr_1*t2+k];
                            dummy += 4.0*kernel[k_incr_0*t1+k_incr_1*(t2+1)+k_incr_2*i+k]*trajectories[traj_incr_0*n+traj_incr_1*(t2+1)+k];
                            dummy += kernel[k_incr_0*t1+k_incr_1*(t2+2)+k_incr_2*i+k]*trajectories[traj_incr_0*n+traj_incr_1*(t2+2)+k];
                        }
                        t2+=2;
                    }
                    if(t2+1 == t1) // Trapezoidal rule for last time-interval
                    {
                        for(size_t k=0; k<num_obs; k++)
                        {
                            dummy += 1.5*kernel[k_incr_0*t1+k_incr_1*t2+k_incr_2*i+k]*trajectories[traj_incr_0*n+traj_incr_1*t2+k];
                            dummy += 1.5*kernel[k_incr_0*t1+k_incr_1*(t2+1)+k_incr_2*i+k]*trajectories[traj_incr_0*n+traj_incr_1*(t2+1)+k];
                        }
                    }
                    buffer(n,t1,i) = dummy;
                }
            }
        }
        buffer *= dt/3.0;
        diff_traj -= buffer; // = dA/dt-drift*A-SUM_t2 K(.,t2)*A(t2)
    }

    return diff_traj;
}

void KernelMethods::writeCovarianceMatrix(tensor<double,3> &ff, string out_folder)
{
    size_t num_traj = ff.shape[0];
    size_t num_ts = ff.shape[1];
    size_t num_obs = ff.shape[2];

    tensor<double> ff_average;
    ff_average = ff.contract({-1,2,3});
    ff_average *= 1.0/num_traj;
    cout << "Write mean values of fluctuating forces: " << out_folder+"/"+"ff_average.f64" << endl;
    ff_average.write("ff_average.f64",out_folder);

    tensor<double> ff_cov;
    ff_cov = ff;
    for(size_t n=0; n<num_traj; n++)
    {
        ff_cov.substract(ff_average, {n});
    }

    gsl_matrix * in_buffer = gsl_matrix_alloc(num_traj,num_ts*num_obs);
    ff_cov *= sqrt(1.0/(num_traj-1));
    ff_cov >> *in_buffer->data;
    ff_cov.clear();

    gsl_matrix * out_buffer = gsl_matrix_alloc(num_ts*num_obs,num_ts*num_obs);

    gsl_blas_dgemm(CblasTrans, CblasNoTrans, 1.0, in_buffer, in_buffer, 0.0, out_buffer);
    gsl_matrix_free(in_buffer);

    ff_cov.alloc({num_ts,num_obs,num_ts,num_obs});
    ff_cov << *out_buffer->data;
    gsl_matrix_free(out_buffer);
    cout << "Write covariance matrix of fluctuating forces: " << out_folder+"/"+"ff_cov.f64" << endl;
    ff_cov.write("ff_cov.f64", out_folder);
}

void KernelMethods::writeExtendedCovarianceMatrix(TensorUtils::tensor<double,3> &traj, TensorUtils::tensor<double,3> &ff, std::string out_folder)
{
    size_t num_traj = ff.shape[0];
    size_t num_ts = ff.shape[1];
    size_t num_obs = ff.shape[2];

    num_ts++;

    tensor<double> ff_cov({num_traj,num_ts,num_obs});
    for(size_t n=0; n<num_traj; n++)
    {
        for(size_t o=0; o<num_obs; o++)
        {
            ff_cov(n,0,o) = traj(n,0,o);
            for(size_t t=1; t<num_ts; t++)
            {
                ff_cov(n,t,o) = ff(n,t-1,o);
            }
        }
    }

    tensor<double> ff_average;
    ff_average = ff_cov.contract({-1,2,3});
    ff_average *= 1.0/num_traj;
    cout << "Write mean of initial values and fluctuating forces: " << out_folder+"/"+"ff_average.f64" << endl;
    ff_average.write("ff_average.f64",out_folder);

    for(size_t n=0; n<num_traj; n++)
    {
        ff_cov.substract(ff_average, {n});
    }

    gsl_matrix * in_buffer = gsl_matrix_alloc(num_traj,num_ts*num_obs);
    ff_cov *= sqrt(1.0/(num_traj-1));
    ff_cov >> *in_buffer->data;
    ff_cov.clear();

    gsl_matrix * out_buffer = gsl_matrix_alloc(num_ts*num_obs,num_ts*num_obs);

    gsl_blas_dgemm(CblasTrans, CblasNoTrans, 1.0, in_buffer, in_buffer, 0.0, out_buffer);
    gsl_matrix_free(in_buffer);

    ff_cov.alloc({num_ts,num_obs,num_ts,num_obs});
    ff_cov << *out_buffer->data;
    gsl_matrix_free(out_buffer);
    cout << "Write covariance matrix of initial values and fluctuating forces: " << out_folder+"/"+"ff_cov.f64" << endl;
    ff_cov.write("ff_cov.f64", out_folder);
}

