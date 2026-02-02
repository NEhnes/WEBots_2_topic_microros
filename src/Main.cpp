#include <Arduino.h>
#include <micro_ros_platformio.h>

#include <rcl/rcl.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>

#include <std_msgs/msg/int32.h>
#include <std_msgs/msg/string.h>
#include <string.h>

#if !defined(MICRO_ROS_TRANSPORT_ARDUINO_SERIAL)
#error This example is only avaliable for Arduino framework with serial transport.
#endif

// for both publishers:
rclc_executor_t executor;
rclc_support_t support;
rcl_allocator_t allocator;
rcl_node_t node;
rcl_publisher_t esp32_pub;

// joystick publisher shi:
std_msgs__msg__Int32 joystick_msg;
rcl_timer_t joystick_timer;
#define VRX_PIN 32
#define VRY_PIN 33

// goose publisher shi:
rcl_publisher_t goose_publisher;
std_msgs__msg__String goose_msg;
rcl_timer_t goose_timer;

// subscriber shi:
rcl_subscription_t subscriber;
std_msgs__msg__String received_msg;

#define RCCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){error_loop();}}
#define RCSOFTCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){}}

#define LED_PIN 18

// Error handle loop
void error_loop() {
  while(1) {
    delay(100);
  }
}

void joystick_timer_callback(rcl_timer_t * joystick_timer, int64_t last_call_time) {
  RCLC_UNUSED(last_call_time);
  if (joystick_timer != NULL) {
    joystick_msg.data = analogRead(VRX_PIN) - 2096; // relative to center
    RCSOFTCHECK(rcl_publish(&esp32_pub, &joystick_msg, NULL));
  }
}

void goose_timer_callback(rcl_timer_t * timer, int64_t last_call_time) {
  RCLC_UNUSED(last_call_time);
  if (timer != NULL) {

    const char* goose_sound = "HONK";

    strcpy(goose_msg.data.data, goose_sound); 
    goose_msg.data.size = strlen(goose_msg.data.data);
    
    RCSOFTCHECK(rcl_publish(&goose_publisher, &goose_msg, NULL));
  }
}

void subscriber_callback(const void * msgin) {
  const std_msgs__msg__String * msg = (const std_msgs__msg__String *)msgin;
  if(strcmp(msg->data.data, "ON") == 0) {
    digitalWrite(LED_PIN, HIGH);
  } else if(strcmp(msg->data.data, "OFF") == 0) {
    digitalWrite(LED_PIN, LOW);
  }
}

void setup() {
  pinMode(LED_PIN, OUTPUT);
  // wait 4s bcz agent bullshit
  delay(1000);
  digitalWrite(LED_PIN, HIGH);
  delay(1000);
  digitalWrite(LED_PIN, LOW);
  delay(1000);
  digitalWrite(LED_PIN, HIGH);
  delay(1000);
  digitalWrite(LED_PIN, LOW);
  // Configure serial transport
  Serial.begin(115200);
  set_microros_serial_transports(Serial);
  delay(2000);

  // create allocator
  allocator = rcl_get_default_allocator();

  // create init_options
  RCCHECK(rclc_support_init(&support, 0, NULL, &allocator));

  // create node
  RCCHECK(rclc_node_init_default(&node, "esp32_node", "", &support));

  // create publisher
  RCCHECK(rclc_publisher_init_default(
    &esp32_pub,
    &node,
    ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32),
    "joystick_x_pub"));

  // create goose_publisher for mating_calls topic
  RCCHECK(rclc_publisher_init_default(
    &goose_publisher,
    &node,
    ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, String),
    "goose_calls"));

  // create timer
  const unsigned int joystick_timer_timeout = 50;
  RCCHECK(rclc_timer_init_default(
    &joystick_timer,
    &support,
    RCL_MS_TO_NS(joystick_timer_timeout),
    joystick_timer_callback)); // removed NULL 5th param

  // create goose_timer with 400ms interval
  const unsigned int goose_timer_timeout = 400;
  RCCHECK(rclc_timer_init_default(
    &goose_timer,
    &support,
    RCL_MS_TO_NS(goose_timer_timeout),
    goose_timer_callback)); // removed NULL 5th param

  // create subscriber
  RCCHECK(rclc_subscription_init_default(
    &subscriber,
    &node,
    ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, String),
    "led_control"));

  // allocate goose_msg
  goose_msg.data.capacity = 10; // enough for "HONK" and null terminator
  goose_msg.data.data = (char *)malloc(goose_msg.data.capacity * sizeof(char));
  goose_msg.data.size = 0;
  goose_msg.data.data[0] = '\0'; // initialize as empty string

  // allocate received_msg
  received_msg.data.capacity = 10; // enough for "ON", "OFF" and null terminator
  received_msg.data.data = (char *)malloc(received_msg.data.capacity * sizeof(char));
  received_msg.data.size = 0;
  received_msg.data.data[0] = '\0'; // initialize as empty string

  // initialize joystick_msg
  joystick_msg.data = 0;

  // create executor
  RCCHECK(rclc_executor_init(&executor, &support.context, 5, &allocator)); // Changed to 2 handles
  RCCHECK(rclc_executor_add_timer(&executor, &joystick_timer));
  RCCHECK(rclc_executor_add_timer(&executor, &goose_timer));
  RCCHECK(rclc_executor_add_subscription(
    &executor,
    &subscriber,
    &received_msg,
    &subscriber_callback,
    ON_NEW_DATA));

  // define joystick input
  pinMode(VRX_PIN, INPUT);
  pinMode(VRY_PIN, INPUT);
}

void loop() {
  delay(100);
  RCSOFTCHECK(rclc_executor_spin_some(&executor, RCL_MS_TO_NS(100)));
}
