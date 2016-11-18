#ifndef IMAGE_UNDISTORT_IMAGE_UNDISTORT_NODELET_H
#define IMAGE_UNDISTORT_IMAGE_UNDISTORT_NODELET_H

#include <stdio.h>
#include <Eigen/Eigen>

#include <image_transport/image_transport.h>
#include <nodelet/nodelet.h>
#include <ros/callback_queue.h>
#include <ros/ros.h>
#include <tf/transform_broadcaster.h>
#include <tf_conversions/tf_eigen.h>

#include <image_undistort/undistorter.h>

#include <cv_bridge/cv_bridge.h>
#include <sensor_msgs/CameraInfo.h>
#include <sensor_msgs/Image.h>

namespace image_undistort {

// Default values

// queue size
constexpr int kQueueSize = 100;
// true to load input cam_info from ros parameters, false to get it from a
// cam_info topic
constexpr bool kDefaultInputCameraInfoFromROSParams = false;
// source of output camera information, the options are as follows-
//  "auto_generated": (default), automatically generated based on the input, the
//    focal length is the average of fx and fy of the input, the center point is
//    in the center of the image, R=I and only x-translation is preserved.
//    Resolution is set to the largest area that contains no empty pixels. The
//    size of the output can also be modified with the "scale" parameter
//  "match_input": intrinsic values are taken from the input camera parameters
//  "ros_params": values are loaded from ros parameters, usually stored in an
//    xml file. The file format matches that generated by Kalibr.
//  "camera_info": the output parameters are found through subscribing to a
//    camera_info ros topic, (default topic is output/cam_info).
const std::string kDefaultOutputCameraInfoSource = "auto_generated";
// namespace to use when loading input camera parameters from ros params
const std::string kDefaultInputCameraNamespace = "input_camera";
// namespace to use when loading output camera parameters from ros params
const std::string kDefaultOutputCameraNamespace = "output_camera";
// true to process images, false if you only wish to generate a cam_info topic
// from ros params (the image topic must still be subscribed to so that the
// cam_info message is published at the correct times).
constexpr bool kDefaultProcessImage = true;
// true to undistort, false if you only wish to modify the intrinsics
constexpr bool kDefaultUndistortImage = true;
// downsamples output rate if <= 1, every frame is processed.
constexpr int kDefaultProcessEveryNthFrame = 1;
// converts the output image to the given format, set to the empty string "" to
// copy input type. Consult cv_bridge documentation for valid strings
const std::string kDefaultOutputImageType = "";
// if the output camera source is set to "auto_generated", the output focal
// length will be multiplied by this value. This has the effect of
// resizing the image by this scale factor.
constexpr double kDefaultScale = 1.0;
// if a tf between the input and output frame should be created
constexpr bool kDefaultPublishTF = true;
// name of output image frame
const std::string kDefaultOutputFrame = "output_camera";
// if radtan distortion should be called radtan (ASL standard) or plumb_bob (ros
// standard)
constexpr bool kDefaultRenameRadtanPlumbBob = false;

class ImageUndistort {
 public:
  ImageUndistort(const ros::NodeHandle& nh_, const ros::NodeHandle& nh_private_);

 private:

  void imageMsgToCvMat(const sensor_msgs::ImageConstPtr& image_msg,
                       cv::Mat* image);

  void updateCameraInfo(const sensor_msgs::CameraInfo& camera_info);

  bool loadCameraParameters(const bool is_input,
                            sensor_msgs::CameraInfo* loaded_camera_info,
                            std::string* image_topic);

  void imageCallback(const sensor_msgs::ImageConstPtr& image_msg_in);

  void cameraCallback(const sensor_msgs::ImageConstPtr& image_msg,
                      const sensor_msgs::CameraInfoConstPtr& cam_info);

  void cameraInfoCallback(const sensor_msgs::CameraInfoConstPtr& camera_info);

  // nodes
  ros::NodeHandle nh_;
  ros::NodeHandle nh_private_;
  image_transport::ImageTransport it_;

  // subscribers
  image_transport::Subscriber image_sub_;         // input image
  image_transport::CameraSubscriber camera_sub_;  // input image and camera_info
  ros::Subscriber camera_info_sub_;               // output camera_info

  // publishers
  image_transport::CameraPublisher camera_pub_;  // output image and camera_info
  image_transport::Publisher image_pub_;         // output image
  ros::Publisher camera_info_pub_;               // output camera_info

  // undistorter
  std::shared_ptr<Undistorter> undistorter_ptr_;

  // tf broadcaster
  tf::TransformBroadcaster br;

  // camera info
  std::shared_ptr<CameraParametersPair> camera_parameters_pair_ptr_;

  enum class OutputInfoSource {
    AUTO_GENERATED,
    MATCH_INPUT,
    ROS_PARAMS,
    CAMERA_INFO
  };

  // other variables
  int queue_size_;
  bool process_image_;
  OutputInfoSource output_camera_info_source_;
  int process_every_nth_frame_;
  std::string output_image_type_;
  double scale_;
  bool publish_tf_;
  std::string output_frame_;
  bool rename_radtan_plumb_bob_;

  int frame_counter_;
};
}

#endif
