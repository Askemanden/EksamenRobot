const double L1 = 10;
const double L2 = 10;
const double L3 = 10;
const double sensor_position_distance = 10;


double sensor_reading_distance = 0;
double sensor_angle_rad = 0;

double base_angle_rad = 0;

double theta1 = 0;
double theta2 = 0;
double theta3 = 0;

struct Vector2{
  double u;
  double v;
}

struct Arm{
  Vector2 targetUV;
  double theta1;
  double theta2;
  double theta3;
  double base_angle;
}

void calculateBaseAngle(Arm *self, )
void calculateArmAnglesUV(Arm *self, double x, double y, double z){

}