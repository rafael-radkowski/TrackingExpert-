//

// stl
#include <iostream>
#include <string>
#include <vector>

// local
#include "ArgParser.h"
#include "TrackingExpertDemo.h"
#include "TrackingExpertParams.h"
using namespace texpert;


TEParams populateParams(Arguments params)
{
	TEParams p;

	p.angle_step = params.fd_angle_step;
	p.camera_sampling_offset = params.camera_sampling_offset;
	p.cluster_rot_threshold = params.fd_cluster_rot_th;
	p.cluster_trans_threshold = params.fd_cluster_trans_th;
	p.curvature_search_radius = params.fd_curvature_radius;
	p.icp_num_max_iterations = (float)params.icp_max_iterations;
	p.icp_outlier_reject_angle = params.icp_outlier_ang_th;
	p.icp_outlier_reject_distance = params.icp_outlier_dist_th;
	p.icp_termination_dist = params.icp_min_rms;
	p.sampling_grid_size = params.sampling_grid_size;
	p.verbose = params.verbose;
	p.filter_enabled = params.filter_enabled;
	p.filter_sigmaI = params.filter_sigmaI;
	p.filter_sigmaS = params.filter_sigmaS;
	p.filter_kernel = params.filter_kernel;
	p.curvature_multiplier = 4.0;
	p.input_mkv = params.scene_file;

	return p;
}


void displayKeyboard(void) {

	std::cout << "\nKeyboard layout:\n--------------------------------------------------------------------------" << std::endl;

	std::cout << " ESC\tExit the application." << std::endl;
	/*std::cout << " 1\tDisplay matching curvatures per point." << std::endl;
	std::cout << " 2\tDisplay matching curvatures patches per point." << std::endl;
	std::cout << " q\tMove to the next matching point." << std::endl;
	std::cout << " w\tMove to the previous matching point." << std::endl;
	std::cout << " 3\tDisplay pose clusters." << std::endl;
	std::cout << " a\tMove to the next pose cluster." << std::endl;
	std::cout << " s\tMove to the previous pose cluster." << std::endl;
	std::cout << " 4\tDisplay curvature histogram values." << std::endl;*/

	std::cout << "\n--------------------------------------------------------------------------" << std::endl;
}

// -scene ../data/Azure_Kinect_model_1_2020-06-19_10-37-18_pc.obj -model ../data/stanford_bunny_pc.obj -verbose
// -scene ../data/stanford_bunny_pc.obj -model ../data/stanford_bunny_pc.obj -verbose
// -scene C:/Users/Tyler/Documents/TrackingExpertPlus/data/stanford_bunny_pc.obj -model C:/Users/Tyler/Documents/TrackingExpertPlus/data/stanford_bunny_pc.obj -verbose
// -cam AzureKinect -model C:/Users/Tyler/Documents/TrackingExpertPlus/data/stanford_bunny_pc.obj -verbose



int main(int argc, char** argv) {
		
	std::cout << "TrackingExpertPlus Example Application" << std::endl;
	std::cout << "V0.5-20210802, Aug. 2021\n" << std::endl;
	std::cout << "--------------------------------------------------------------------------" << std::endl;
	std::cout << "The demo demonstrates the use of TrackingExpert+" << std::endl;
	std::cout << "It was prepared to track one object given a 3D point cloud model as input.\n" << std::endl;

	std::cout << "Rafael Radkowski" << std::endl;
	std::cout << "radkowski.dev@gmail.com" << std::endl;
	std::cout << "MIT License" << std::endl;
	std::cout << "--------------------------------------------------------------------------\n" << std::endl;

	displayKeyboard(); 

	// Parse the command line arguments
	Arguments params = ArgParser::Parse(argc, argv);

	// Set the capture device
	CaptureDeviceType type = CaptureDeviceType::None;
	if (params.camera_type.compare("AzureKinect") == 0) type = KinectAzure;
	else if (params.camera_type.compare("AzureKinectMKV") == 0) type = AzureKinectMKV;

	
	// Start the demo
	TrackingExpertDemo* demo = new TrackingExpertDemo();
	demo->setParams(populateParams( params));
	demo->setVerbose(params.verbose);
	demo->setSourceCamera(type);
	demo->loadReferenceModel(params.model_file, "model");


#ifdef _WITH_REGISTRATION
	// for debugging
	demo->resetReferenceModel();
#endif

	// Blocking function. 
	demo->run();

	delete demo;

	return 1;
}