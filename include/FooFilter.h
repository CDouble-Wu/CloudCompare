//##########################################################################
//#                                                                        #
//#                          IOMyDescriptorPlugin                              #
//#                                                                        #
//#  This program is free software; you can redistribute it and/or modify  #
//#  it under the terms of the GNU General Public License as published by  #
//#  the Free Software Foundation; version 2 or later of the License.      #
//#                                                                        #
//#  This program is distributed in the hope that it will be useful,       #
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of        #
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the          #
//#  GNU General Public License for more details.                          #
//#                                                                        #
//#          COPYRIGHT: CloudCompare project                               #
//#                                                                        #
//##########################################################################

#pragma once

#include "FileIOFilter.h"
#include<opencv2/opencv.hpp>
//struct CameraParams {
//  int imageWidth = 1280;				// unit:pixel
//  int imageHeight = 720;			// unit:pixel
//  float baseline = 0.12f;				// unit:m
//  float bf_value =0 ;		// (m) the actual size of a pixel
//  float optic_x =0;           // (º) the FOV of X direction
//  float optic_y =0;           // (º) the FOV of Y direction
//};
class RGB {
public:
  char r, g, b;
};
enum {
  R = 2,
  G = 1,
  B = 0
};
class FooFilter : public FileIOFilter
{
public:
  struct CameraParams {
    int imageWidth = 1280;				// unit:pixel
    int imageHeight = 720;			// unit:pixel
    float baseline = 0.12f;				// unit:m
    int  bf_value = 0;		// (m) the actual size of a pixel
    int optic_x = 0;           // (º) the FOV of X direction
    int optic_y = 0;           // (º) the FOV of Y direction
  };
	FooFilter();
    CCVector3d m_globalShift{ 0, 0, 0 };
    bool isValid(bool displayErrors = true) const;
	// Inherited from FileIOFilter
	CC_FILE_ERROR loadFile( const QString &fileName, ccHObject &container, LoadParameters &parameters ) override;
    bool  load_camera_info(const std::string &camera_dir, CameraParams &params);
    cv::Mat  load_image(const std::string &disparity_path, cv::Mat &src,int imageWidth,int imageHeight, std::string &pattern);
	bool canSave( CC_CLASS_ENUM type, bool &multiple, bool &exclusive ) const override;
   char* YCbYCrPlannar2Rgb(const unsigned char* src, uchar* des, int width, int height);
    static RGB Yuv2Rgb(char Y, char U, char V);

};
