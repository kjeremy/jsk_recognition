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

#include "jsk_perception/roi_to_rect.h"
#include <boost/assign.hpp>
#include <jsk_topic_tools/log_utils.h>

namespace jsk_perception
{
  void ROIToRect::onInit()
  {
    DiagnosticNodelet::onInit();
    pub_ = advertise<geometry_msgs::PolygonStamped>(
      *pnh_, "output", 1);
    onInitPostProcess();
  }

  void ROIToRect::subscribe()
  {
    sub_ = pnh_->subscribe("input", 1, &ROIToRect::convert, this);
    ros::V_string names = boost::assign::list_of("~input");
    jsk_topic_tools::warnNoRemap(names);
  }

  void ROIToRect::unsubscribe()
  {
    sub_.shutdown();
  }

  void ROIToRect::convert(
    const sensor_msgs::CameraInfo::ConstPtr& roi_msg)
  {
    vital_checker_->poke();
    geometry_msgs::PolygonStamped rect;
    rect.header = roi_msg->header;
    geometry_msgs::Point32 top_left_pt, top_right_pt, bottom_left_pt, bottom_right_pt;
    top_left_pt.x = roi_msg->roi.x_offset;
    top_left_pt.y = roi_msg->roi.y_offset;
    top_right_pt.x = roi_msg->roi.x_offset + roi_msg->roi.width;
    top_right_pt.y = roi_msg->roi.y_offset;
    bottom_left_pt.x = roi_msg->roi.x_offset;
    bottom_left_pt.y = roi_msg->roi.y_offset + roi_msg->roi.height;
    bottom_right_pt.x = roi_msg->roi.x_offset + roi_msg->roi.width;
    bottom_right_pt.y = roi_msg->roi.y_offset + roi_msg->roi.height;
    rect.polygon.points.push_back(top_left_pt);
    rect.polygon.points.push_back(bottom_left_pt);
    rect.polygon.points.push_back(bottom_right_pt);
    rect.polygon.points.push_back(top_right_pt);
    pub_.publish(rect);
  }
}

#include <pluginlib/class_list_macros.h>
PLUGINLIB_EXPORT_CLASS (jsk_perception::ROIToRect, nodelet::Nodelet);
