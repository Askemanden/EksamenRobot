/**
* @brief $L_1$
*/
#define L1 (10);

/**
* @brief $L_2$
*/
#define L2 (10);

/**
* @brief $L_3$
*/
#define L3 (10);

#define STEP_DIST (1);

#define GRAB_HEIGHT (3);

/**
* @struct Vector2
* @brief A 2d vector in the uv plane of the arm.
* @var Vector2::u U-coordinate (x-equivalent) of the vector.
* @var Vector2::v v-coordinate (y-equivalent) of the vector.
*/
struct Vector2 {
  double u;
  double v;
}


/**
* @struct Arm
* @brief Contains angles of the arm. All angles are in radians.
* @var Arm::theta1 Angle at the bottom of the arm. $\theta_1$.
* @var Arm::theta2 Elbow angle of the arm. $\theta_2$.
* @var Arm::theta3 Wrist angle of the arm. Should always result in 90 deg negative angle with x-axist at the point of rotation. $\theta_3$.
* @var Arm::base_angle Arm angle with the x-axis. Rotates the UV plane around Z. $\theta_O$
* @var Arm::last_targetUV Last target of the Arm from which to calculate the next target in the path of the hand.
*/
struct Arm {
  Vector2 final_targetUV;
  double theta1;
  double theta2;
  double theta3;
  double base_angle;
  double targetUV;
}

Vector2
vectorAdd(Vector2 self, Vector2 other) {
  return { .u = self.u + other.y, .v = self.v + other.v };
}

/**
* @brief Calculates base_angle and writes it to self.
* @param self The arm for which to calculate data.
* @param sensor_position_distance Distance of sensor from origin. $d_s$
* @param sensor_reading_distance The reading of the sensor in centimeters. $L_s$
* @param sensor_angle_rad Angle in the sensors servo. $\theta_s$
*/
int calculateBaseAngle(Arm *self, double sensor_position_distance, double sensor_reading_distance, double sensor_angle_rad) {
  /*$d_s*/
  double target_distance = sqrt(sq(sensor_position_distance) + sq(sensor_reading_distance) - 2 * sensor_position_distance * sensor_distance_reading * cos(sensor_angle_rad));
  double sin_theta_O = sin(sensor_angle_rad) * sensor_distance_reading / target_distance;
  double cos_theta_O = (sq(target_distance) + sq(sensor_position_distance) - sq(sensor_reading_distance)) / (2 * target_distance * sensor_position_distance);
  self->base_angle = atan2(sin_theta_O, cos_theta_O);
  self->final_targetUV = target_distance
  return 0;
}

/**
* @brief Finds the next UV coordinate in the path of self and writes to self on targetUV.
* @param self The arm to which to write
*/
int calculateNextTargetUV(Arm *self) {
  Vector2 final_targetUV = self->final_targetUV;

  Vector2 previous_target = self->targetUV;

  double safe_v = final_targetUV.v + GRAB_HEIGHT;

  if (abs(previous_target.u - final_targetUV.u) < 0.01) {
    self->targetUV = vectorAdd(previous_target, (Vector2){ .u = 0, .v = -STEP_DIST });
  }

  else if (previous_target.v < safe_v) {
    self->targetUV = vectorAdd(previous_target, (Vector2){ .u = 0, .v = STEP_DIST });
  }

  else if (previous_target.u < final_targetUV.u) {
    self->targetUV = vectorAdd(previous_target, (Vector2){ .u = STEP_DIST, .v = 0 });
  } else {
    self->targetUV = vectorAdd(previous_target, (Vector2){ .u = -STEP_DIST, .v = 0 });
  }

  if (abs(self->targetUV.u - final_targetUV.u) < STEP_DIST) {
    self->targetUV.u = final_targetUV.u;
  }
  if (abs(self->targetUV.v - final_targetUV.v) < STEP_DIST) {
    self->targetUV.v = final_targetUV.v;
  }

  return 0;
}


/**
* @brief Calculates the angles ($\theta_1 \, \theta_2 \, \theta_3$) of self and write to self.
* @param self The arm to which to write.
*/
int calculateArmAngles(Arm *self) {
  Vector2 final_targetUV = self->final_targetUV;
  Vector2 target = self->targetUV;
  self->theta2 = acos((sq(target.u) + sq(target.v) - sq(L1) - sq(L2)) / (2 * L1 * L2));
  double k1 = L1 + L2 * cos(self->theta2);
  double k2 = L2 * sin(self->theta2);
  self->theta1 = atan2(target.v, target.u) - atan2(k2, k1);
  self->theta3 = 2 * PI - self->theta1 - self->theta2 - PI / 2;
  return 0;
}
