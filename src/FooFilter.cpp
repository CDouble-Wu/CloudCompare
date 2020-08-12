//##########################################################################
//#                                                                        #
//#                           IOMyDescriptorPlugin                              #
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

#include <QString>
#include "FooFilter.h"
#include <filesystem>
#include<fstream>
#include <vector>
#include <iostream>
#include <string.h>
//qCC_db
#include <ccGenericPointCloud.h>
#include <ccPointCloud.h>
#include <ccProgressDialog.h>
#include <ReferenceCloud.h>
//CC
#include <ScalarField.h>
#include<QTextStream>
#include <QFileDialog>
#include <ccGLWindow.h>

namespace fs = std::experimental::filesystem;

FooFilter::FooFilter()
	: FileIOFilter( {
					"dat Filter",
					DEFAULT_PRIORITY,	// priority
					QStringList{ "dat", "txt" },
					"dat",
					QStringList{ "dat file (*.dat)", "Text file (*.txt)" },
					QStringList(),
					Import
					} )
{
}


CC_FILE_ERROR FooFilter::loadFile(const QString &fileName, ccHObject &container, LoadParameters &parameters)
{
  Q_UNUSED(container);
  Q_UNUSED(parameters);;
  float disp;
  double x, y, z;
  int based=0;
  cv::Mat src,des;
  bool is_png = true;
  std::vector<std::vector<double>>colors;
  colors.reserve(1280 * 720);
  QFile file(fileName); 
  std::string pattern;
  std::ifstream ifs;
  CameraParams params;
  double cPointB = 0;
  double cPointG = 0;
  double cPointR = 0;
  int count = 0;
   //std::string disparity_file(R"(\\192.168.100.252\pic\≤‚ ‘◊È◊®”√ÕººØ_S2&infinite2\’œ∞≠ŒÔºÏ≤‚∑∂Œß≤‚ ‘ÕººØ\∞◊ÃÏÀ≥π‚\–°∆˚≥µºÏ≤‚∑∂Œß≤‚ ‘ÕººØ\h2m\z15m\disparity_images\disparity_5_00000001589603849590_W1280_Speed_0.dat)");
  std::string disparity_file = std::string((const char *)fileName.toLocal8Bit());
  int found = disparity_file.find("disparity_images");
  std::string all_file = disparity_file;
  all_file.erase(found, all_file.size() - found);
  bool camera_params =load_camera_info(all_file, params);
  if (camera_params)
  { 
    des = load_image(disparity_file, src, params.imageWidth , params.imageHeight, pattern);
    if (pattern==".raw")
    {
      is_png = false;
      if (src.channels()==1)
      {
        cv::cvtColor(src, src, cv::COLOR_GRAY2BGR);
      }
    }
    ifs.open(disparity_file, std::ios::binary);
    int buffer_size = params.imageWidth * params.imageHeight * 2;
    unsigned short  *buffer = new unsigned short[params.imageWidth * params.imageHeight];
    ifs.read((char *)buffer, buffer_size);
    ccPointCloud* cloud = new ccPointCloud();
    int count = 0;
    ccColor::Rgb cols;
    std::vector<double> RGB;
    std::vector<float> para;
    CCVector3 P;
    double * point = new double[3];
    for (int row = 0; row < params.imageHeight; row++) {
      based = row * params.imageWidth;
      auto pix = src.ptr<cv::Vec3b>(row);
      for (int col = 0; col < params.imageWidth;col++)
      {
        disp = float(buffer[based + col]) / 32.0;
        if (disp > 2)
        {
          x = (params.baseline * (col - params.optic_x))/ disp;
          y = -(params.baseline * (row - params.optic_y) )/ disp;
          z = -(params.bf_value / disp);
         if ((-80.0 < z )&&( z< 80.0) && (-25 < x )&&(x< 25))
          {
           point[0] =x;
           point[1] =y;
           point[2] =z;
             P = CCVector3::fromArray(point);
             cloud->addPoint(P);
             cPointB = pix [col][0];
             cPointG = pix [col][1];
             cPointR = pix [col][2];
             RGB.push_back(cPointR);
             RGB.push_back(cPointG);
             RGB.push_back(cPointB);
             colors.push_back(RGB);
             RGB.clear();
          }
        }
      }
    } 
    delete[]point;
    cloud->setPointSize(0);
    unsigned counts = cloud->size();
    cloud->setColor(ccColor::white);
    for (int t=0;t<counts;t++)
    {
      cols.r = colors[t][0];
      cols.g = colors[t][1];
      cols.b = colors[t][2];
      cloud->setPointColor(t, cols);
    }
    cloud->setVisible(true);
    cloud->showColors(true);
    container.addChild(cloud);
    ccLog::Print(QStringLiteral("[Camera Params] The file has succeed input"));
    return CC_FERR_NO_ERROR;

  }
  else
  {
    ccLog::Print(QStringLiteral("[Camera Params] The file has not input "));
    return CC_FERR_NO_LOAD;
  }
  
}
bool FooFilter::canSave( CC_CLASS_ENUM type, bool &multiple, bool &exclusive ) const
{
	Q_UNUSED( type );
	Q_UNUSED( multiple );
	Q_UNUSED( exclusive );

	// ... can we save this?
	return false;
}



bool FooFilter::load_camera_info(const std::string &image_dir, CameraParams &params) {
  for (auto &p : fs::directory_iterator(image_dir)) {
    /*if (fs::is_directory(p) && p.path().filename() == "adas_params") {*/
    if (fs::is_directory(p) && p.path().filename() == "adas_params") {
      float mm2m = 1000.f;
      size_t base_opt_x;
      cv::FileStorage  fs(p.path().string() + "\\cameraInstallParam.yml", cv::FileStorage::READ);
      fs.release();
      fs.open(p.path().string() + "\\calibData.yml", cv::FileStorage::READ);
      if (!fs.isOpened()) {
        std::cout << "Cannot open file " << p.path().string() + "\\calibData.yml" << std::endl;
        return false;
      }
      std::string file_version = (std::string)fs["FileVersion"];
      if (file_version == "1.0") {
        base_opt_x = (float)fs["Calib"]["data"][42];
        params.optic_y = (float)fs["Calib"]["data"][43];
        params.baseline = std::abs((float)fs["Calib"]["data"][22] / mm2m);
      }
      else {
        params.baseline = std::abs((float)fs["Calib"]["data"][46] / mm2m);
        base_opt_x = ((float)fs["Calib"]["data"][39] + (float)fs["Calib"]["data"][41]) / 2.0f;
        params.optic_y = ((float)fs["Calib"]["data"][40] + (float)fs["Calib"]["data"][42]) / 2.0f;
        fs.release();
      }
      fs.open(p.path().string() + "\\depthData.yml", cv::FileStorage::READ);
      if (!fs.isOpened()) {
        std::cout << "Cannot open file " << p.path().string() + "\\depthData.yml" << std::endl;
        return false;
      }
      params.bf_value = (float)fs["DepthCalib"]["data"][1];
      float delta = (float)fs["DepthCalib"]["data"][4] + (float)fs["DepthCalib"]["data"][0];
      params.optic_x = base_opt_x - std::floor(delta);
      params.baseline = std::abs((float)fs["DepthCalib"]["data"][2]);
      fs.release();
      return true;
    }
  }
    if (std::experimental::filesystem::exists(image_dir + "..\\adas_params")) {
     // std::string a = image_dir + "..\\";
      if (load_camera_info(image_dir + "..\\", params)) {
          return true;
    }
  }
  return false;
}

cv::Mat FooFilter::load_image(const std::string &disparity_path, cv::Mat &src, int imageWidth, int imageHeight, std::string &pattern) {
  cv::Mat img;
  std::string image_path = disparity_path;
  int found = image_path.find("disparity_images");
  image_path.replace(found, 16, "left_images");
  found = image_path.find("disparity_5_");
  image_path.replace(found, 12, "LRemap_");
  image_path = image_path.substr(0, image_path.length() - 4) + ".png";
  img = cv::imread(image_path);
  if (img.empty())
  {
    int type = image_path.find("png");
    image_path.replace(type, image_path.size(), "raw");
  }
  fs::path p(image_path);
 pattern = p.extension().string();
  std::ifstream ifs,ifd;
    if (pattern == ".png") {
      src = cv::imread(image_path);
      return src;
    }
    else if (pattern == ".raw") {
      cv::Mat image1;
      cv::Mat src1;
      ifs.open(image_path, std::ios::binary);
      ifd.open(image_path, std::ios::binary);
      ifd.seekg(0, std::ios_base::end);
      std::streampos sp = ifd.tellg();
      long length = (long)sp;
      if (length == 921600)
      {
        image1.create(imageWidth, imageHeight, CV_8U);
        if (!ifs.is_open()) {
          std::cout << "Cannot open file " << image_path << std::endl;
          return image1;
        }
        ifs.read((char *)image1.data, imageWidth * imageHeight);
        cv::Mat src2(imageHeight, imageWidth, CV_8U, image1.data);
        src = src2.clone();
        ifs.close();
      }if (length == 1843200)
      {
        image1.create( imageHeight*2,imageWidth, CV_8UC1);
        src1.create(imageHeight, imageWidth, CV_8UC3);
        if (!ifs.is_open()) {
          std::cout << "Cannot open file " << image_path << std::endl;
          return image1;
        }
        ifs.read((char *)image1.data, imageWidth * imageHeight*2);
        uchar* data = image1.ptr<uchar>(0);
        uchar* data1 = src1.ptr<uchar>(0);
        YCbYCrPlannar2Rgb(data, data1, imageHeight, imageWidth);
        cv::Mat src2(imageHeight, imageWidth, CV_8UC3, data1);
        src = src2.clone();
        ifs.close();
      }
      else
      {
        ccLog::Print(QStringLiteral("[color Params] The file has succeed input"));
      }
     
    } 
  return src;
}

 char* FooFilter::YCbYCrPlannar2Rgb(const unsigned char* src,  uchar* des, int width, int height)
{
  const unsigned char *CbBase = src + width * height;
  const unsigned char *CrBase = CbBase + width * height / 2;
  for (int i = 0; i < width*height; i += 2) {
    RGB tmp = Yuv2Rgb(src [i], CbBase [i / 2], CrBase [i / 2]);
    des [i * 3 + R] = tmp.r;
    des [i * 3 + G] = tmp.g;
    des [i * 3 + B] = tmp.b;
    tmp = Yuv2Rgb(src [i + 1], CbBase [i / 2], CrBase [i / 2]);
    des [(i + 1) * 3 + R] = tmp.r;
    des [(i + 1) * 3 + G] = tmp.g;
    des [(i + 1) * 3 + B] = tmp.b;
  }

  return (char*)des;
}


RGB FooFilter::Yuv2Rgb(char Y, char U, char V)
{
  RGB rgb;
  int r = (int)((Y & 0xff) + 1.4075 * ((V & 0xff) - 128));
  int g = (int)((Y & 0xff) - 0.3455 * ((U & 0xff) - 128) - 0.7169*((V & 0xff) - 128));
  int b = (int)((Y & 0xff) + 1.779 * ((U & 0xff) - 128));
  rgb.r = (r < 0 ? 0 : r>255 ? 255 : r);
  rgb.g = (g < 0 ? 0 : g>255 ? 255 : g);
  rgb.b = (b < 0 ? 0 : b>255 ? 255 : b);
  return rgb;
}

