### _Visit [DRIVE FOLDER](https://drive.google.com/drive/u/0/folders/1Xs8jZ2kVp76sEvjYTNtu95YPYwZKQTcp) for build pics & videos_

### _Will fix readme later when it's not midterm szn_

Initial experimentation with microros system

Works thru agent, stable communication to full ROS2 Jazzy install achieved

Currently has 3 topics:
/goose_calls -> esp32 sends a string "HONK" periodically

/joystick_x_pub -> esp32 reads data from peripheral joystick and sends x coordinate

/led_control -> ROS2 install sends string commands to esp32, which triggers an LED accordingly.
