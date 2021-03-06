#include <uWS/uWS.h>
#include <iostream>
#include "json.hpp"
#include "PID.h"
#include <math.h>

// for convenience
using json = nlohmann::json;

// For converting back and forth between radians and degrees.
constexpr double pi() { return M_PI; }
double deg2rad(double x) { return x * pi() / 180; }
double rad2deg(double x) { return x * 180 / pi(); }

// Checks if the SocketIO event has JSON data.
// If there is data the JSON object in string format will be returned,
// else the empty string "" will be returned.
std::string hasData(std::string s) {
  auto found_null = s.find("null");
  auto b1 = s.find_first_of("[");
  auto b2 = s.find_last_of("]");
  if (found_null != std::string::npos) {
    return "";
  }
  else if (b1 != std::string::npos && b2 != std::string::npos) {
    return s.substr(b1, b2 - b1 + 1);
  }
  return "";
}

int main()
{
  uWS::Hub h;

  PID pid;
  PID pid_throttle;
  // TODO: Initialize the pid variable.
  //pid.Init(0.1, 0.001, 0.5); can go through the complete track with minor oscalation
  //pid.Init(0.1, 0.001, 1); imporved a little bit
  //pid.Init(0.1, 0.001, 5); a lot of sudden changes of steer value
  //pid.Init(0.3, 0.001, 5); more sudden changes of steer value
  //pid.Init(0.3, 0.001, 2); still have sudden changes of steer value
  pid.Init(0.12, 0.0001, 1.2 ); 
  pid_throttle.Init(0.3 , 0.0, 0.02);   

  int window_size=3;
  double steer_value_history[window_size];
  int count = 0;
  h.onMessage([&pid, &pid_throttle, &window_size, &steer_value_history,&count](uWS::WebSocket<uWS::SERVER> ws, char *data, size_t length, uWS::OpCode opCode) {
    // "42" at the start of the message means there's a websocket message event.
    // The 4 signifies a websocket message
    // The 2 signifies a websocket event
    if (length && length > 2 && data[0] == '4' && data[1] == '2')
    {
      auto s = hasData(std::string(data).substr(0, length));
      if (s != "") {
        auto j = json::parse(s);
        std::string event = j[0].get<std::string>();
        if (event == "telemetry") {
          // j[1] is the data JSON object
          double cte = std::stod(j[1]["cte"].get<std::string>());
          double speed = std::stod(j[1]["speed"].get<std::string>());
          double angle = std::stod(j[1]["steering_angle"].get<std::string>());
          double steer_value;
          double throttle;
          /*
          * TODO: Calcuate steering value here, remember the steering value is
          * [-1, 1].
          * NOTE: Feel free to play around with the throttle and speed. Maybe use
          * another PID controller to control the speed!
          */
          pid.UpdateParameters(speed);
          pid.UpdateError(cte); 
          pid_throttle.UpdateError(fabs(cte));

          steer_value = -pid.TotalError(); 
          steer_value = std::max(std::min(1.0, steer_value), -1.0);
          steer_value_history[count]=steer_value;
          
          throttle = -pid_throttle.TotalError(); 
          throttle = std::min(throttle+0.7, 0.3);

          double steer_value_sum = 0;
          for( int i = 0; i < window_size; i++ ) {
            if(count == i){
              steer_value_sum += (window_size+1)* steer_value_history[i]; 
            }
            else
            {
              steer_value_sum += steer_value_history[i]; 
            }
          }
          steer_value = steer_value_sum /  (2*window_size);
          count = (count+1)%window_size;
          
          // DEBUG
          std::cout << "CTE: " << cte 
                    << " speed: "<< speed
                    << " angle: "<< angle
                    << " Steering Value: " << steer_value 
                    << " Steering Value History: " << steer_value_history[0] << std::endl
                    << " d_error: " << pid.d_error*pid.Kd<<std::endl
                    << " p_error: " << pid.p_error*pid.Kp<<std::endl
                    << " i_error: " << pid.i_error*pid.Ki<<std::endl;


          json msgJson;
          msgJson["steering_angle"]  = steer_value;
          msgJson["throttle"] = throttle;
          auto msg = "42[\"steer\"," + msgJson.dump() + "]";
          std::cout << msg << std::endl<< std::endl;
          ws.send(msg.data(), msg.length(), uWS::OpCode::TEXT);
        }
      } else {
        // Manual driving
        std::string msg = "42[\"manual\",{}]";
        ws.send(msg.data(), msg.length(), uWS::OpCode::TEXT);
      }
    }
  });

  // We don't need this since we're not using HTTP but if it's removed the program
  // doesn't compile :-(
  h.onHttpRequest([](uWS::HttpResponse *res, uWS::HttpRequest req, char *data, size_t, size_t) {
    const std::string s = "<h1>Hello world!</h1>";
    if (req.getUrl().valueLength == 1)
    {
      res->end(s.data(), s.length());
    }
    else
    {
      // i guess this should be done more gracefully?
      res->end(nullptr, 0);
    }
  });

  h.onConnection([&h](uWS::WebSocket<uWS::SERVER> ws, uWS::HttpRequest req) {
    std::cout << "Connected!!!" << std::endl;
  });

  h.onDisconnection([&h](uWS::WebSocket<uWS::SERVER> ws, int code, char *message, size_t length) {
    ws.close();
    std::cout << "Disconnected" << std::endl;
  });

  int port = 4567;
  if (h.listen(port))
  {
    std::cout << "Listening to port " << port << std::endl;
  }
  else
  {
    std::cerr << "Failed to listen to port" << std::endl;
    return -1;
  }
  h.run();
}
