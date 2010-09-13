/* LONManager.h - Defines prototypes for all communications with the LON nodes and stores data retrieved from the LON network.
 *
 * By: Michael Lindemuth
 */

/* CTD sensor data */
typedef  struct {
    float       Conductivity;
    float       Temperature;
    float       Depth;
    float       Velocity;
}CTD_SENSOR_DATA;

CTD_SENSOR_DATA getCTDSensorData();
