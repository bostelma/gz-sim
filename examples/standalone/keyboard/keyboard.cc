/*
 * Copyright (C) 2019 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
*/

/*
  Adapted from https://github.com/ros/ros_tutorials/blob/lunar-devel/turtlesim/tutorials/teleop_turtle_key.cpp
 */


#include <ignition/msgs/twist.pb.h>
#include <ignition/transport/Node.hh>
#include <sdf/sdf.hh>
#include <ignition/common/Time.hh>
#include <ignition/common/Console.hh>
#include <signal.h>
#include <termios.h>
#include <stdio.h>

//#include <fcntl.h>
#include <unistd.h>  // read()

#define KEYCODE_ARR_R 0x43 
#define KEYCODE_ARR_L 0x44
#define KEYCODE_ARR_U 0x41
#define KEYCODE_ARR_D 0x42

#define KEYCODE_Q 0x71

#define KEYCODE_A 0x61
#define KEYCODE_S 0x73
#define KEYCODE_D 0x64
#define KEYCODE_W 0x77


ignition::transport::Node::Publisher cmdVelPub;

/*
ignition::math::Vector3d axisLinear;
ignition::math::Vector3d scaleLinear;

ignition::math::Vector3d axisAngular;
ignition::math::Vector3d scaleAngular;
*/


class TeleopTurtle
{
public:

  TeleopTurtle();
  void keyLoop();

private:

  double linear_, angular_, l_scale_, a_scale_;
};


TeleopTurtle::TeleopTurtle():
  linear_(0),
  angular_(0),
  l_scale_(1.0),
  a_scale_(1.0)
{

}

int kfd = 0;
struct termios cooked, raw;

void quit(int sig)
{
  (void)sig;
  tcsetattr(kfd, TCSANOW, &cooked);
  exit(0);
}


void TeleopTurtle::keyLoop()
{
  char c;
  bool dirty=false;


  // get the console in raw mode                                                              
  tcgetattr(kfd, &cooked);
  memcpy(&raw, &cooked, sizeof(struct termios));
  raw.c_lflag &=~ (ICANON | ECHO);
  // Setting a new line, then end of file                         
  raw.c_cc[VEOL] = 1;
  raw.c_cc[VEOF] = 2;
  tcsetattr(kfd, TCSANOW, &raw);

  puts("Reading from keyboard");
  puts("---------------------------");
  puts("Use arrow keys to move the robot.");


  for(;;)
  {
    // get the next event from the keyboard  
    if(read(kfd, &c, 1) < 0)
    {
      perror("read():");
      exit(-1);
    }

    double linear_ = 0;
    double angular_ = 0;
    fprintf (stderr, "value: 0x%02X\n", c);
  
    switch(c)
    {
      case KEYCODE_ARR_L:
        std::cerr << "LEFT" << std::endl;
        angular_ = 1.0;
        dirty = true;
        break;
      case KEYCODE_ARR_R:
        std::cerr << "RIGHT" << std::endl;
        angular_ = -1.0;
        dirty = true;
        break;
      case KEYCODE_ARR_U:
        std::cerr << "UP" << std::endl;
        linear_ = 1.0;
        dirty = true;
        break;
      case KEYCODE_ARR_D:
        std::cerr << "DOWN" << std::endl;
        linear_ = -1.0;
        dirty = true;
        break;
    }
   
    ignition::msgs::Twist cmdVelMsg;
    cmdVelMsg.mutable_linear()->set_x(l_scale_ * linear_);
    cmdVelMsg.mutable_angular()->set_z(a_scale_ * angular_);

    /*
    cmdVelMsg.mutable_linear()->set_x(axisLinear.X() * scaleLinear.X());
    cmdVelMsg.mutable_linear()->set_y(axisLinear.Y() * scaleLinear.Y());
    cmdVelMsg.mutable_linear()->set_z(axisLinear.Z() * scaleLinear.Z());

    cmdVelMsg.mutable_angular()->set_x(axisAngular.X() * scaleAngular.X());
    cmdVelMsg.mutable_angular()->set_y(axisAngular.Y() * scaleAngular.Y());
    cmdVelMsg.mutable_angular()->set_z(axisAngular.Z() * scaleAngular.Z());
    */

    if(dirty ==true)
    {
      cmdVelPub.Publish(cmdVelMsg);
      dirty=false;
    }
  }

  return;
}


int main(int argc, char** argv)
{
  if (argc != 2)
  {
    std::cerr << "Usage: keyboard <sdf_file>" << std::endl;
    return -1;
  }

  // Get parameters from SDF file
  auto sdf = sdf::readFile(argv[1]);

  if (!sdf)
  {
    std::cerr << "Failed to parse file [" << argv[1] << "]" << std::endl;
    return -1;
  }


  // TODO(louise) This is not a plugin, we need a new SDF tag
  auto plugin = sdf->Root()->GetElement("world")->GetElement("plugin");

  // Set up transport
  ignition::transport::Node node;

  auto twistTopic = plugin->Get<std::string>("twist_topic", "/cmd_vel").first;
  cmdVelPub = node.Advertise<ignition::msgs::Twist>(twistTopic);

  /*
  axisLinear = plugin->Get<ignition::math::Vector3d>("axis_linear",
      ignition::math::Vector3d::UnitX).first;
  scaleLinear = plugin->Get<ignition::math::Vector3d>("scale_linear",
      ignition::math::Vector3d(0.5, 0, 0)).first;

  axisAngular = plugin->Get<ignition::math::Vector3d>("axis_angular",
      ignition::math::Vector3d::Zero).first;
  scaleAngular = plugin->Get<ignition::math::Vector3d>("scale_angular",
      ignition::math::Vector3d(0, 0, 0.5)).first;
  */


  TeleopTurtle teleop_turtle;
  signal(SIGINT, quit);
  teleop_turtle.keyLoop();
  
  return(0);
}
