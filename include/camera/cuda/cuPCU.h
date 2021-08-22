#pragma once
/*
class cuPCU

The class cuPCU (point cloud utils) provides apis too transfer a depth image into
a point cloud and to calculate normal vector for each point in this point clouud.
The input is a pointer to the image memory. The output a vector with point cloud points. 

The output in  vector<float3>& points and vector<float3>& normals are index aligned 
and provide one point per image pixel, even if the pixel data is invalid. Each invalid point
will result in a vector p = {0, 0, 0}.
Thus, the vector index is related to the pixel position.

	I(depth, i) -> P(i) = {p0, p1, p2, ...., pN } as float3 with pi = {x,y,z}
	I(depth, i) -> N(i) = {n0, n1, n2, ...., nN } as float3 with pi = {x,y,z}

Allocate memory first, before creating a point cloud with CreatePointCloud();
The memory is managed in the class cuDevMem. Memory can be allocated by invoking AllocateDeviceMemory()

Features:
	- Converts a depth image into an array of points and normal vectors per points. 

Rafael Radkowski
Iowa State University
rafael@iastate.edu
September 27, 2017
MIT License
---------------------------------------------------------------
*/

// stl
#include <stdio.h>
#include <iostream>
#include <vector>
#include <algorithm>

// cuda
#include "cuda_runtime.h"

// OpenCV
#include <opencv2/highgui.hpp>
#include <opencv2/core.hpp>



using namespace std;


namespace texpert
{

class cuPCU
{
public:



	/*
	Create a point cloud from a depth image with all points
	@param src_image_ptr - a pointer to the image of size [wdith x height x channels ] stored as an array of type unsigned short which stores the depth values as
						A(i) = {d0, d1, d2, ..., dN}
	@param width - the width of the image in pixels
	@param height - the height of the image in pixels
	@param channels - the number of channels. A depth image should have only 1 channel.
	@param focal_legnth - the focal length of the camera in pixel
	@param step_size - for normal vector calculations. The interger specifies how many steps a neighbor sould be away no obtain a vector for normal vector calculation.
					Minimum step_size is 1.
	@param points - a vector A(i) = {p0, p1, p2, ..., pN} with all points p_i = {px, py, pz} as float3.
	@param normals - a vector A(i) = {n0, n1, n2, ..., nN} with all normal vectors n_i = {nx, ny, nz} as float3
	@param to_host - if true, the device normals and points are copied back to the host. If false, this is skipped and the  data in points and normals remains empty.
					NOTE, COPYING DATA REQUIRES A SIGNIFICANT AMOUNT OF TIME AND SHOULD ONLY BE EXECUTED AT THE VERY LAST STEP
	*/
	static int CreatePointCloud(unsigned short* src_image_ptr, int width, int height, int chanels, float focal_length_x, float focal_length_y, float cx, float cy, int step_size, vector<float3>& points, vector<float3>& normals, bool to_host = true);

	/*
	Init the device memory. The device memory can be re-used. So no need to always create new memory.
	@param width - the width of the image in pixels
	@param height - the height of the image in pixels
	@param channels - the number of channels. A depth image should have only 1 channel.
	*/
	static void AllocateDeviceMemory(int width, int height, int chanels);


	/*
	Free all device memory
	*/
	static void FreeDeviceMemory(void);

	/*
	Copy the depth image from the device and return it as an openCV mat.
	Image memory needs to be intialized since the function writes the image into the memory.
	Use:
	cv::Mat output_points = cv::Mat::zeros(height, width, CV_32FC3);
	The image is returns all values as grid image I(u,v) with each component storing I(ui,vi) = {px, py, pz} the components of a particular point
	I(u,v) = {p0_x, p0_y, p0_z, p1_x, p1_y, p1_z, p2_x, p2_y, p2_z, ......, p0_N, p0_N, p0_N }
	@param depth_image - reference to storage for the depth image as OpenCV Mat of type CV_32FC3.
	*/
	static void GetDepthImage(cv::Mat& depth_image);

	/*
	Copy the normal vectors encoded into a rgb image from the device and return it as an openCV mat.
	Image memory needs to be intialized since the function writes the image into the memory.
	Use:
	cv::Mat output_normals = cv::Mat::zeros(height, width, CV_32FC3);
	The image is returned all normal vector values organized as grid image I(u,v) with each component storing I(ui,vi) = {nx, ny, nz} the normal vecctor components of a particular point
	I(u,v) = {n0_x, n0_y, n0_z, n1_x, n1_y, n1_z, n2_x, n2_y, n2_z, ......, n0_N, n0_N, n0_N }
	@param depth_image - reference to the normal vector image as OpenCV Mat of type CV_32FC3
	*/
	static void GetNormalImage(cv::Mat& depth_image);


private:


};




class cuSample
{
public:

	/*
	Create a sample pattern to uniformly remove points from the point set
	@param width - the width of the image
	@param height - the height of the image
	@param sampling_steps - the number of pixels the pattern should step over in each frame
	*/
	static void CreateUniformSamplePattern(int width, int height, int sampling_steps);



	/*
	Create a sample pattern to randomly remove points from the point set
	@param width - the width of the image
	@param height - the height of the image
	@param max_points - the max. number of points
	@param percentage - percentage of points to be used. 
	Note, either max_points or percentage can be used. One of them must be set to -1;
	*/
	static void CreateRandomSamplePattern(int width, int height, int max_points, float percentage = -1.0);


	/*
	Create a point cloud from a depth image. The point set is uniformly sampled. 

	NOTE, use the function CreateSamplePattern() to create the pattern for sampling

	@param src_image_ptr - a pointer to the image of size [wdith x height x channels ] stored as an array of type unsigned short which stores the depth values as
	A(i) = {d0, d1, d2, ..., dN}
	@param width - the width of the image in pixels
	@param height - the height of the image in pixels
	@param channels - the number of channels. A depth image should have only 1 channel.
	@param focal_legnth - the focal length of the camera in pixel
	@param normal_radius - for normal vector calculations. The interger specifies how many steps a neighbor sould be away no obtain a vector for normal vector calculation.
	Minimum normal_radius is 1.
	@param cp_enabled - cutting plane enabled if true;
	@param points - a vector A(i) = {p0, p1, p2, ..., pN} with all points p_i = {px, py, pz} as float3.
	@param normals - a vector A(i) = {n0, n1, n2, ..., nN} with all normal vectors n_i = {nx, ny, nz} as float3
	@param to_host - if true, the device normals and points are copied back to the host. If false, this is skipped and the  data in points and normals remains empty.
	NOTE, COPYING DATA REQUIRES A SIGNIFICANT AMOUNT OF TIME AND SHOULD ONLY BE EXECUTED AT THE VERY LAST STEP
	*/
	static void UniformSampling(unsigned short* src_image_ptr, int width, int height, float focal_length_x, float focal_length_y, float cx, float cy, int normal_radius, bool cp_enabled,  vector<float3>& points, vector<float3>& normals, bool to_host = true);



	/*
	Create a point cloud from a depth image. The point set is randomly sampled.
	NOTE, use the function CreateRandomSamplePattern() to create the pattern for sampling

	@param src_image_ptr - a pointer to the image of size [wdith x height x channels ] stored as an array of type unsigned short which stores the depth values as
	A(i) = {d0, d1, d2, ..., dN}
	@param width - the width of the image in pixels
	@param height - the height of the image in pixels
	@param channels - the number of channels. A depth image should have only 1 channel.
	@param focal_legnth - the focal length of the camera in pixel
	@param normal_radius - for normal vector calculations. The interger specifies how many steps a neighbor sould be away no obtain a vector for normal vector calculation.
	Minimum normal_radius is 1.
	@param points - a vector A(i) = {p0, p1, p2, ..., pN} with all points p_i = {px, py, pz} as float3.
	@param normals - a vector A(i) = {n0, n1, n2, ..., nN} with all normal vectors n_i = {nx, ny, nz} as float3
	@param to_host - if true, the device normals and points are copied back to the host. If false, this is skipped and the  data in points and normals remains empty.
	NOTE, COPYING DATA REQUIRES A SIGNIFICANT AMOUNT OF TIME AND SHOULD ONLY BE EXECUTED AT THE VERY LAST STEP
	*/ 
	static void cuSample::RandomSampling(unsigned short* src_image_ptr, int width, int height, float focal_length, int normal_radius, bool cp_enabled, vector<float3>& points, vector<float3>& normals, bool to_host=true);



	/*
	Set parameters for a cutting plane that removes points from the point set.
	The plane is defined by A * x + B * y + C * z = D
	// where a point gets removed if D - current_D > CP_THRESHOLD
	*/
	static void SetCuttingPlaneParams(float a, float b, float c, float d, float threshold);
};


} //namespace tacuda


