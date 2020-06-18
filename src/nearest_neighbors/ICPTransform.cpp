#include "ICPTransform.h"

#include <cassert>

using namespace texpert; 

/*
Computes the difference between two set of points
@param points0 - a vector with Eigen::Vec3 points with each [x, y, z, w]
@param points1 - a vector with Eigen::VEc3 points
@return the rotation and translation difference between those points as
4x4 matri in homogenous coordinate frames.
*/
Matrix4f ICPTransform::Compute(vector<Vector3f>& points0, vector<Vector3f>& points1, Pose initial_pose)
{

	Matrix4f result = Matrix4f::Identity();
	Matrix4f overall = Matrix4f::Identity();

	vector<Vector3f> in0, in1;
	// copy the points
	std::copy(points0.begin(), points0.end(), back_inserter(in0));
	std::copy(points1.begin(), points1.end(), back_inserter(in1));

	float rms = 100000000.0;


	//Matrix4f M = initial_pose.t.matrix();
	Matrix3f R = initial_pose.t.rotation();
	Vector3f t = initial_pose.t.translation();
	//for (auto & e : in0) e = ( R * e) + t;
	for_each(in0.begin(), in0.end(), [&](Vector3f& p){p = (R * p) + t;});


	// the loop expects that both vectors are already index aligned. 
	int itr = 0;
	for (int i = 0; i < 10; i++)
	{
		//  Calculate the rotation delta
		Matrix3f R = CalcRotationArun(in0, in1);

		Vector3f t = CalculateTranslation(in0, in1);

		Matrix3f R_inv = R;// Matrix3f::Identity();

		// maintaining row-major
		result(0) = R_inv(0);
		result(1) = R_inv(1);
		result(2) = R_inv(2);

		result(4) = R_inv(3);
		result(5) = R_inv(4);
		result(6) = R_inv(5);

		result(8) = R_inv(6);
		result(9) = R_inv(7);
		result(10) = R_inv(8);

		result(12) = t.x();
		result(13) = t.y();
		result(14) = t.z();

		// update the points
		/// TODO: performance teste. Which function is faster for_each vs. std::transform
		//for (auto & e : in0) e = (R * e) + t;
		for_each(in0.begin(), in0.end(), [&](Vector3f& p){p = (R * p) + t;});
	
		overall =  result * overall;

		itr++;
		rms = CheckRMS(in0, in1);

		cout << "RMS: " << rms << " at " << itr << "." << endl;

		if (rms < 0.00001) break;

		
	}
	
	cout << "ICP RMS: " << rms << " with " << itr << " interations." << endl;

	return overall;
}

/*
Calculate the rotation using the method from Arun et al. 1987
Arun, K., Huang, T.S., and Blostein, S.D., 1987. �Least - squares fitting of two 3 - d point sets�.
IEEE Transactions on Pattern Analysis and Machine Intel - ligence, PAMI - 9(5), Sept, pp. 698�700

@param pVec0 - first point array, each element needs to be a vector4 [[x, y, z, w], [x, y, z, w], ...]
@param pVec1 - second point array, each element needs to be a vector4 [[x, y, z, w], [x, y, z, w], ...]
@param return: 4x4 matrix with the delta rotation
*/
//static 
Matrix3f ICPTransform::CalcRotationArun(vector<Vector3f>& pVec0, vector<Vector3f>& pVec1)
{

	assert(pVec0.size() == pVec1.size()); // size of both vectors must be equal

	Matrix3f W = Matrix3f::Zero(3, 3);
	int count = 0;

	// mean values
	// requires C++14
	Vector3f avg0 = accumulate(pVec0.begin(), pVec0.end(), Vector3f(0, 0, 0), [currIndex = 0U, lastIndex = pVec0.size()](Vector3f x, Vector3f y) mutable { return x + y; }) / pVec0.size();
	Vector3f avg1 = accumulate(pVec1.begin(), pVec1.end(), Vector3f(0, 0, 0), [currIndex = 0U, lastIndex = pVec1.size()](Vector3f x, Vector3f y) mutable { return x + y; }) / pVec1.size();

	size_t size = pVec0.size();
	for (auto i=0; i < size; i++)
	{
		Vector3f p0 = pVec0[i] - avg0;
		Vector3f p1 = pVec1[i] - avg1;

		// x-cov matrix
		W = W + p0 *  p1.transpose() ;
	}
		
	float d = W.determinant();
		
	//Singular value decompositon
	JacobiSVD<Matrix3f> svd(W, ComputeFullU | ComputeFullV);
	Matrix3f R = (svd.matrixV() * svd.matrixU().transpose());



	float det = R.determinant();
	if (det < 0.0) {
	//	_cprintf("Singular exception \n");
		R = Matrix3f::Identity();
	}
	//std::cout << det << std::endl; 
	//R = Matrix3f::Identity();
	return R;
}


/*
Calculates the translation delta between both point sets as mean.
@param pVec0 - first point array, each element needs to be a vector4 [[x, y, z, w], [x, y, z, w], ...]
@param pVec1 - second point array, each element needs to be a vector4 [[x, y, z, w], [x, y, z, w], ...]
@param return: Vec 3 with the delta translation
*/
//static 
Vector3f ICPTransform::CalculateTranslation(vector<Vector3f>& pVec0, vector<Vector3f>& pVec1)
{
	// mean values
	// requires C++14
	Vector3f avg0 = accumulate(pVec0.begin(), pVec0.end(), Vector3f(0, 0, 0), [currIndex = 0U, lastIndex = pVec0.size()](Vector3f x, Vector3f y) mutable { return x + y; }) / pVec0.size();
	Vector3f avg1 = accumulate(pVec1.begin(), pVec1.end(), Vector3f(0, 0, 0), [currIndex = 0U, lastIndex = pVec1.size()](Vector3f x, Vector3f y) mutable { return x + y; }) / pVec1.size();


//#define AVG_NAIVE
#ifdef AVG_NAIVE
	Vector3f avg0n(0.0,0.0,0.0);
	Vector3f avg1n(0.0,0.0,0.0);

	if (pVec0.size() != pVec1.size()) {
		cout << "[ICPTransform] - Error: vectors with different size (1)." << endl;
	}

	for (int i = 0; i < pVec0.size(); i++) {
		avg0n += pVec0[i];
		avg1n += pVec1[i];
	}

	avg0n /= pVec0.size();
	avg1n /= pVec0.size();

	cout << "T0 avg: " << avg0 << " == " << avg0n << endl;
	cout << "T1 avg: " << avg1 << " == " << avg1n << endl;

	if ((avg0 - avg0n).norm() > 0.0001) {
			cout << "[ERROR] T0 avg  " << endl;
	}
	if ((avg1 - avg1n).norm() > 0.0001) {
			cout << "[ERROR] T1 avg " << endl;
	}
	Vector3f delta_n = avg1n - avg0n;
	Vector3f delta = avg1 - avg0;
	if ((delta_n - delta).norm() > 0.0001) {
			cout << "[ERROR] Delta error " << endl;
	}

#endif

	return avg1 - avg0;
}


/*
Check the root mean square error between the two points sets.
Do not forget to translate the reference point set before checking.
@param pVec0 - first point array, each element needs to be a vector4 [[x, y, z, w], [x, y, z, w], ...]
@param pVec1 - second point array, each element needs to be a vector4 [[x, y, z, w], [x, y, z, w], ...]
@param return: float values representing the RMS
*/
//static 
float ICPTransform::CheckRMS(vector<Vector3f>& pVec0, vector<Vector3f>& pVec1)
{
	Vector3f avg0 = accumulate(pVec0.begin(), pVec0.end(), Vector3f(0, 0, 0), [currIndex = 0U, lastIndex = pVec0.size()](Vector3f x, Vector3f y) mutable { return x + y; }) / pVec0.size();
	Vector3f avg1 = accumulate(pVec1.begin(), pVec1.end(), Vector3f(0, 0, 0), [currIndex = 0U, lastIndex = pVec1.size()](Vector3f x, Vector3f y) mutable { return x + y; }) / pVec1.size();

	//cout << "A0 -> " << avg0 << "\tA1 -> " << avg1 << endl;

	float sum2 = (avg1.transpose() * avg1);
	sum2 -= avg0.transpose() * avg0;
	return sqrt(abs(sum2));
}



Vector3f ICPTransform::CalculateCentroid(vector<Vector3f>& pVec0)
{
	Vector3f avg0 = accumulate(pVec0.begin(), pVec0.end(), Vector3f(0, 0, 0), [currIndex = 0U, lastIndex = pVec0.size()](Vector3f x, Vector3f y) mutable { return x + y; }) / pVec0.size();
	return avg0;
}