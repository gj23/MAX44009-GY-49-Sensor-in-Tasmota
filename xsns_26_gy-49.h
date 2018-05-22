#ifdef USE_I2C
#ifdef USE_GY49
/*****
****************************************************************************************\
 * MAX44009 (GY-49) - Ambient Light Intensity
 *
 * I2C Address: 0x4A
\*********************************************************************************************/
#define GY49_ADDR1         0x4A

#define GY49_CONTINUOUS_HIGH_RES_MODE 0x03 

uint8_t gy49_address = GY49_ADDR1;
uint8_t gy49_type = 0;
byte a, b;
int c, d;

uint16_t GY49ReadLux()
{

  Wire.requestFrom(gy49_address, (uint8_t)2);
  byte msb = Wire.read();
  byte lsb = Wire.read();
  a = msb;
  b = lsb;

  int exponent = (msb & 0xF0) >> 4;
  int mantissa = ((msb & 0x0F) << 4) | (lsb & 0x0F);
  float luminance = pow(2, exponent) * mantissa * 0.045;
  c = exponent;
  d = mantissa;

  return luminance;
}

/********************************************************************************************/

void GY49Detect()
{
  if (gy49_type) {
    return;
  }

    Wire.beginTransmission(gy49_address);
    Wire.write(GY49_CONTINUOUS_HIGH_RES_MODE);
    if (!Wire.endTransmission()) {
      gy49_type = 1;
      snprintf_P(log_data, sizeof(log_data), S_LOG_I2C_FOUND_AT, "GY-49", gy49_address);
      AddLog(LOG_LEVEL_DEBUG);
  }
}

#ifdef USE_WEBSERVER
const char HTTP_SNS_GY49[] PROGMEM =
  "%s{s}GY-49 " D_ILLUMINANCE "{m}%d " D_UNIT_LUX "{e}";  // {s} = <tr><th>, {m} = </th><td>, {e} = </td></tr>
#endif  // USE_WEBSERVER

void GY49Show(boolean json)
{
  if (gy49_type) {
    uint16_t illuminance = GY49ReadLux();

    if (json) {
      snprintf_P(mqtt_data, sizeof(mqtt_data), PSTR("%s,\"GY-49\":{\"" D_JSON_ILLUMINANCE "\":%d}"), mqtt_data, illuminance);
      
#ifdef USE_DOMOTICZ
      if (0 == tele_period) DomoticzSensor(DZ_ILLUMINANCE, illuminance);
#endif  // USE_DOMOTICZ
#ifdef USE_WEBSERVER
    } else {
      snprintf_P(mqtt_data, sizeof(mqtt_data), HTTP_SNS_GY49, mqtt_data, illuminance);
#endif  // USE_WEBSERVER
    }
  }
}

/*********************************************************************************************\
 * Interface
\*********************************************************************************************/

#define XSNS_26

boolean Xsns26(byte function)
{
  boolean result = false;

  if (i2c_flg) {
    switch (function) {
      case FUNC_PREP_BEFORE_TELEPERIOD:
        GY49Detect();
        break;
      case FUNC_JSON_APPEND:
        GY49Show(1);
        break;
#ifdef USE_WEBSERVER
      case FUNC_WEB_APPEND:
        GY49Show(0);
        break;
#endif  // USE_WEBSERVER
    }
  }
  return result;
}

#endif // USE_GY49
#endif //USE_I2C
