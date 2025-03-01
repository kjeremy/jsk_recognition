// -*- mode: c++ -*-
/*********************************************************************
 * Software License Agreement (BSD License)
 *
 *  Copyright (c) 2015, JSK Lab
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of the JSK Lab nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *********************************************************************/

#include <jsk_perception/multiply_mask_image.h>
#include <boost/assign.hpp>
#include <jsk_topic_tools/log_utils.h>
#include <opencv2/opencv.hpp>
#include <cv_bridge/cv_bridge.h>
#include <sensor_msgs/image_encodings.h>

namespace jsk_perception
{
  void MultiplyMaskImage::onInit()
  {
    DiagnosticNodelet::onInit();
    pnh_->param("approximate_sync", approximate_sync_, false);
    pnh_->param("queue_size", queue_size_, 100);
    pub_ = advertise<sensor_msgs::Image>(
      *pnh_, "output", 1);
    onInitPostProcess();
  }

  MultiplyMaskImage::~MultiplyMaskImage() {
    // message_filters::Synchronizer needs to be called reset
    // before message_filters::Subscriber is freed.
    // Calling reset fixes the following error on shutdown of the nodelet:
    // terminate called after throwing an instance of
    // 'boost::exception_detail::clone_impl<boost::exception_detail::error_info_injector<boost::lock_error> >'
    //     what():  boost: mutex lock failed in pthread_mutex_lock: Invalid argument
    // Also see https://github.com/ros/ros_comm/issues/720 .
    sync_.reset();
    async_.reset();
  }

  void MultiplyMaskImage::subscribe()
  {
    sub_src1_.subscribe(*pnh_, "input/src1", 1);
    sub_src2_.subscribe(*pnh_, "input/src2", 1);
    if (approximate_sync_) {
      async_ = boost::make_shared<message_filters::Synchronizer<ApproxSyncPolicy> >(queue_size_);
      async_->connectInput(sub_src1_, sub_src2_);
      async_->registerCallback(boost::bind(&MultiplyMaskImage::multiply, this, _1, _2));
    }
    else {
      sync_ = boost::make_shared<message_filters::Synchronizer<SyncPolicy> >(queue_size_);
      sync_->connectInput(sub_src1_, sub_src2_);
      sync_->registerCallback(boost::bind(&MultiplyMaskImage::multiply, this, _1, _2));
    }
    ros::V_string names = boost::assign::list_of("~input/src1")("~input/src2");
    jsk_topic_tools::warnNoRemap(names);
  }

  void MultiplyMaskImage::unsubscribe()
  {
    sub_src1_.unsubscribe();
    sub_src2_.unsubscribe();
  }

  void MultiplyMaskImage::multiply(
    const sensor_msgs::Image::ConstPtr& src1_msg,
    const sensor_msgs::Image::ConstPtr& src2_msg)
  {
    cv::Mat src1 = cv_bridge::toCvShare(
      src1_msg, src1_msg->encoding)->image;
    cv::Mat src2 = cv_bridge::toCvShare(
      src2_msg, src2_msg->encoding)->image;
    cv::Mat result;
    cv::bitwise_and(src1, src2, result);
    pub_.publish(
      cv_bridge::CvImage(src1_msg->header,
                         sensor_msgs::image_encodings::MONO8,
                         result).toImageMsg());
  }
  
}


#include <pluginlib/class_list_macros.h>
PLUGINLIB_EXPORT_CLASS (jsk_perception::MultiplyMaskImage, nodelet::Nodelet);
