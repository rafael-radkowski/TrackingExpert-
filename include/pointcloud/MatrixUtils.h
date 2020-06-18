#pragma once

#pragma once
/*
class MatrixUtils

Rafael Radkowski
Iowa State University 
Feb 2018
rafael@iastate.edu
MIT License
---------------------------------------------------------------
Last edited:

June 11, 2020, RR
- Added a function to convert a 4x4 eigen matrix into a glm matrix. 
*/

#include <iostream>
#include <string>



// GLM include files
#define GLM_FORCE_INLINE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>  // transformation
#include <glm/gtx/quaternion.hpp> // quaternions

// Eigen
#include <Eigen/Dense>

// local
#include "Types.h"

using namespace std;

namespace texpert
{

	class MatrixUtils
	{
	public:



		/*
		Converts an Eigen::AAffine3f transformation to a glm4 matrix
		Matrices are in column major order.
		@param matrix - affine matrix of type Affine3f (4x4).
		@return glm mat4 matrix. 
		*/
		static glm::mat4 Affine3f2Mat4(Eigen::Affine3f& matrix);



		/*
		Convert an Einge4f matrix to an glm::mat4 matrix. 
		*/
		static glm::mat4 Matrix4f2Mat4(Eigen::Matrix4f& matrix);


		/*!
		Convert the ICP->Rt() result into a glm::mat4 matrix
		@param matrix - a 4x4 matrix with the pose of the object.
		@return - a glm 4x4 matrix with the translation of the reference object. 
		*/
		static glm::mat4 ICPRt3Mat4(Eigen::Matrix4f& matrix);



		/*
		Print an affine3 Eigen matrix.
		@param matrix - the matrix in column-major order
		*/
		static void PrintAffine3f(Eigen::Affine3f& matrix);


		/*
		Print an a glm::mat4  matrix.
		@param matrix - the matrix in column-major order
		*/
		static void PrintGlm4(glm::mat4& matrix);


		static void PrintMatrix4f(Eigen::Matrix4f& matrix);

	private:


	};



};