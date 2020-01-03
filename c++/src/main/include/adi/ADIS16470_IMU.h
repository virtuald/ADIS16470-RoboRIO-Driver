/*----------------------------------------------------------------------------*/
/* Copyright (c) 2016-2018 FIRST. All Rights Reserved.                        */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in the root directory of */
/* the project.                                                               */
/*                                                                            */
/* Modified by Juan Chong - juan.chong@analog.com                             */
/*----------------------------------------------------------------------------*/

#pragma once

#include <atomic>
#include <cstdint>
#include <memory>
#include <thread>

#include <frc/DigitalOutput.h>
#include <frc/DigitalSource.h>
#include <frc/DigitalInput.h>
#include <frc/GyroBase.h>
#include <frc/SPI.h>
#include <frc/smartdashboard/SendableBuilder.h>
#include <wpi/mutex.h>
#include <wpi/condition_variable.h>

namespace frc {

/* ADIS16470 Calibration Time Enum Class */
enum class ADIS16470CalibrationTime { 
  _32ms = 0,
  _64ms = 1,
  _128ms = 2,
  _256ms = 3,
  _512ms = 4,
  _1s = 5,
  _2s = 6,
  _4s = 7,
  _8s = 8,
  _16s = 9,
  _32s = 10,
  _64s = 11
};

/* ADIS16470 Register Map Declaration */
static constexpr uint8_t FLASH_CNT      =   0x00;  //Flash memory write count
static constexpr uint8_t DIAG_STAT      =   0x02;  //Diagnostic and operational status
static constexpr uint8_t X_GYRO_LOW     =   0x04;  //X-axis gyroscope output, lower word
static constexpr uint8_t X_GYRO_OUT     =   0x06;  //X-axis gyroscope output, upper word
static constexpr uint8_t Y_GYRO_LOW     =   0x08;  //Y-axis gyroscope output, lower word
static constexpr uint8_t Y_GYRO_OUT     =  	0x0A;  //Y-axis gyroscope output, upper word
static constexpr uint8_t Z_GYRO_LOW     = 	0x0C;  //Z-axis gyroscope output, lower word
static constexpr uint8_t Z_GYRO_OUT     =   0x0E;  //Z-axis gyroscope output, upper word
static constexpr uint8_t X_ACCL_LOW     =   0x10;  //X-axis accelerometer output, lower word
static constexpr uint8_t X_ACCL_OUT     =   0x12;  //X-axis accelerometer output, upper word
static constexpr uint8_t Y_ACCL_LOW     =   0x14;  //Y-axis accelerometer output, lower word
static constexpr uint8_t Y_ACCL_OUT     =   0x16;  //Y-axis accelerometer output, upper word
static constexpr uint8_t Z_ACCL_LOW     =   0x18;  //Z-axis accelerometer output, lower word
static constexpr uint8_t Z_ACCL_OUT     =   0x1A;  //Z-axis accelerometer output, upper word
static constexpr uint8_t TEMP_OUT       =   0x1C;  //Temperature output (internal, not calibrated)
static constexpr uint8_t TIME_STAMP     =   0x1E;  //PPS mode time stamp
static constexpr uint8_t X_DELTANG_LOW  =   0x24;  //X-axis delta angle output, lower word
static constexpr uint8_t X_DELTANG_OUT  =   0x26;  //X-axis delta angle output, upper word
static constexpr uint8_t Y_DELTANG_LOW  =   0x28;  //Y-axis delta angle output, lower word
static constexpr uint8_t Y_DELTANG_OUT  =   0x2A;  //Y-axis delta angle output, upper word
static constexpr uint8_t Z_DELTANG_LOW  =   0x2C;  //Z-axis delta angle output, lower word
static constexpr uint8_t Z_DELTANG_OUT  =   0x2E;  //Z-axis delta angle output, upper word
static constexpr uint8_t X_DELTVEL_LOW  =   0x30;  //X-axis delta velocity output, lower word
static constexpr uint8_t X_DELTVEL_OUT  =   0x32;  //X-axis delta velocity output, upper word
static constexpr uint8_t Y_DELTVEL_LOW  =   0x34;  //Y-axis delta velocity output, lower word
static constexpr uint8_t Y_DELTVEL_OUT  =   0x36;  //Y-axis delta velocity output, upper word
static constexpr uint8_t Z_DELTVEL_LOW  =   0x38;  //Z-axis delta velocity output, lower word
static constexpr uint8_t Z_DELTVEL_OUT  =   0x3A;  //Z-axis delta velocity output, upper word
static constexpr uint8_t XG_BIAS_LOW    =   0x40;  //X-axis gyroscope bias offset correction, lower word
static constexpr uint8_t XG_BIAS_HIGH   =   0x42;  //X-axis gyroscope bias offset correction, upper word
static constexpr uint8_t YG_BIAS_LOW    =   0x44;  //Y-axis gyroscope bias offset correction, lower word
static constexpr uint8_t YG_BIAS_HIGH 	=   0x46;  //Y-axis gyroscope bias offset correction, upper word
static constexpr uint8_t ZG_BIAS_LOW    =   0x48;  //Z-axis gyroscope bias offset correction, lower word
static constexpr uint8_t ZG_BIAS_HIGH   =   0x4A;  //Z-axis gyroscope bias offset correction, upper word
static constexpr uint8_t XA_BIAS_LOW    =   0x4C;  //X-axis accelerometer bias offset correction, lower word
static constexpr uint8_t XA_BIAS_HIGH   =   0x4E;  //X-axis accelerometer bias offset correction, upper word
static constexpr uint8_t YA_BIAS_LOW    =   0x50;  //Y-axis accelerometer bias offset correction, lower word
static constexpr uint8_t YA_BIAS_HIGH   =   0x52;  //Y-axis accelerometer bias offset correction, upper word
static constexpr uint8_t ZA_BIAS_LOW    =   0x54;  //Z-axis accelerometer bias offset correction, lower word
static constexpr uint8_t ZA_BIAS_HIGH   =   0x56;  //Z-axis accelerometer bias offset correction, upper word
static constexpr uint8_t FILT_CTRL      =   0x5C;  //Filter control
static constexpr uint8_t MSC_CTRL       =   0x60;  //Miscellaneous control
static constexpr uint8_t UP_SCALE       =   0x62;  //Clock scale factor, PPS mode
static constexpr uint8_t DEC_RATE       =   0x64;  //Decimation rate control (output data rate)
static constexpr uint8_t NULL_CNFG      =   0x66;  //Auto-null configuration control
static constexpr uint8_t GLOB_CMD       =   0x68;  //Global commands
static constexpr uint8_t FIRM_REV       =   0x6C;  //Firmware revision
static constexpr uint8_t FIRM_DM        =   0x6E;  //Firmware revision date, month and day
static constexpr uint8_t FIRM_Y         =   0x70;  //Firmware revision date, year
static constexpr uint8_t PROD_ID        =   0x72;  //Product identification 
static constexpr uint8_t SERIAL_NUM     =   0x74;  //Serial number (relative to assembly lot)
static constexpr uint8_t USER_SCR1      =   0x76;  //User scratch register 1 
static constexpr uint8_t USER_SCR2      =   0x78;  //User scratch register 2 
static constexpr uint8_t USER_SCR3      =   0x7A;  //User scratch register 3 
static constexpr uint8_t FLSHCNT_LOW    =   0x7C;  //Flash update count, lower word 
static constexpr uint8_t FLSHCNT_HIGH   =   0x7E;  //Flash update count, upper word 

/* ADIS16470 Auto SPI Data Packet */
static constexpr uint8_t m_autospi_packet [14] = {
X_DELTANG_OUT, 
FLASH_CNT, 
X_DELTANG_LOW, 
FLASH_CNT, 
Y_DELTANG_OUT, 
FLASH_CNT, 
Y_DELTANG_LOW, 
FLASH_CNT, 
Z_DELTANG_OUT, 
FLASH_CNT, 
Z_DELTANG_LOW, 
FLASH_CNT, 
PROD_ID, 
FLASH_CNT
};

/**
 * Use DMA SPI to read rate and acceleration data from the ADIS16470 IMU and return the
 * robot's heading relative to a starting position and instant measurements
 *
 * The ADIS16470 gyro angle outputs track the robot's heading based on the starting position. As
 * the robot rotates the new heading is computed by integrating the rate of rotation returned by 
 * the IMU. When the class is instantiated, a short calibration routine is performed where the 
 * IMU samples the gyros while at rest to determine the initial offset. This is subtracted from 
 * each sample to determine the heading.
 *
 * This class is for the ADIS16470 IMU connected via the primary SPI port available on the RoboRIO.
 */

class ADIS16470_IMU : public GyroBase {
 public:

 enum IMUAxis { kX, kY, kZ };

  /**
  * IMU constructor on onboard SPI CS0.
  */
  ADIS16470_IMU();

  /**
   * IMU constructor on the specified SPI port.
   * 
   * @param yaw_axis Selects the "default" axis to use for GetAngle() and GetRate()
   * @param port The SPI port where the IMU is connected.
   */
  explicit ADIS16470_IMU(IMUAxis yaw_axis, SPI::Port port, ADIS16470CalibrationTime cal_time);

  ~ADIS16470_IMU();

  ADIS16470_IMU(ADIS16470_IMU&&) = default;
  ADIS16470_IMU& operator=(ADIS16470_IMU&&) = default;

  void Calibrate() override;
  bool Reconfigure(ADIS16470CalibrationTime new_cal_time);

  /**
   * Reset the gyro.
   *
   * Resets the gyro accumulations to a heading of zero. This can be used if 
   * there is significant drift in the gyro and it needs to be recalibrated
   * after running.
   */
  void Reset() override;

  /**
   * Return the actual angle in degrees that the robot is currently facing.
   *
   * The angle is based on the current accumulator value corrected by
   * offset calibration and built-in IMU calibration. The angle is continuous, 
   * that is it will continue from 360->361 degrees. This allows algorithms 
   * that wouldn't want to see a discontinuity in the gyro output as it sweeps 
   * from 360 to 0 on the second time around. The axis returned by this 
   * function is adjusted fased on the configured yaw_axis.
   *
   * @return the current heading of the robot in degrees. This heading is based
   *         on integration of the returned rate from the gyro.
   */
  double GetAngle() const override;

  /**
   * Return the IMU X-axis integrated angle in degrees.
   *
   * The angle is based on the current accumulator value corrected by
   * offset calibration and built-in IMU calibration. The angle is continuous, 
   * that is it will continue from 360->361 degrees. This allows algorithms 
   * that wouldn't want to see a discontinuity in the gyro output as it sweeps 
   * from 360 to 0 on the second time around. 
   *
   * @return the current accumulated value of the X-axis in degrees. 
   */
  double GetAngleX() const;

  /**
   * Return the IMU Y-axis integrated angle in degrees.
   *
   * The angle is based on the current accumulator value corrected by
   * offset calibration and built-in IMU calibration. The angle is continuous, 
   * that is it will continue from 360->361 degrees. This allows algorithms 
   * that wouldn't want to see a discontinuity in the gyro output as it sweeps 
   * from 360 to 0 on the second time around. 
   *
   * @return the current accumulated value of the Y-axis in degrees. 
   */
  double GetAngleY() const;

  /**
   * Return the IMU Z-axis integrated angle in degrees.
   *
   * The angle is based on the current accumulator value corrected by
   * offset calibration and built-in IMU calibration. The angle is continuous, 
   * that is it will continue from 360->361 degrees. This allows algorithms 
   * that wouldn't want to see a discontinuity in the gyro output as it sweeps 
   * from 360 to 0 on the second time around. 
   *
   * @return the current accumulated value of the Z-axis in degrees. 
   */
  double GetAngleZ() const;

  double GetRate() const;

  void InitSendable(SendableBuilder& builder) override;

 private:

  bool SwitchToStandardSPI();
  bool SwitchToAutoSPI();

  // IMU yaw axis
  IMUAxis m_yaw_axis;

  uint16_t ReadRegister(uint8_t reg);
  void WriteRegister(uint8_t reg, uint16_t val);
  void Acquire();

  // Integrated gyro values
  double m_integ_gyro_x = 0.0;
  double m_integ_gyro_y = 0.0;
  double m_integ_gyro_z = 0.0;

  std::atomic_bool m_freed;
  SPI::Port m_spi_port;
  uint16_t m_calibration_time;
  SPI *m_spi = nullptr;
  DigitalInput *m_auto_interrupt;
  
  std::thread m_acquire_task;

  mutable wpi::mutex m_mutex;

};

} //namespace frc