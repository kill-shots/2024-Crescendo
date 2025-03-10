// Copyright (c) 2023-2024 CurtinFRC
// Open Source Software, you can modify it according to the terms
// of the MIT License at the root of this project

#pragma once

#include <frc/Compressor.h>
#include <frc/DoubleSolenoid.h>
#include <frc/XboxController.h>
#include <frc/system/plant/DCMotor.h>
#include <units/angle.h>
#include <units/length.h>

#include <string>

#include <ctre/phoenix6/CANcoder.hpp>
#include <ctre/phoenix6/Pigeon2.hpp>
#include <ctre/phoenix6/TalonFX.hpp>

#include "Wombat.h"
#include "Intake.h"
#include "IntakeBehaviour.h"

struct RobotMap {

  struct Controllers {
    frc::XboxController driver = frc::XboxController(0);
    frc::XboxController codriver = frc::XboxController(1);
  };

  struct IntakeSystem {

    rev::CANSparkMax intakeMotor{99, rev::CANSparkMax::MotorType::kBrushless};
    wom::CANSparkMaxEncoder intakeEncoder{&intakeMotor, 0.1_m};
    frc::DigitalInput intakeSensor {0};
    frc::DigitalInput magSensor {0};
    frc::DigitalInput shooterSensor {0};

    wom::Gearbox IntakeGearbox { 
      &intakeMotor,
      &intakeEncoder,
      frc::DCMotor::NEO(1)
    };

    IntakeConfig config {
      IntakeGearbox,
      &intakeSensor,
      &magSensor,
      &shooterSensor
    };
  }; IntakeSystem intakeSystem;

  Controllers controllers;

  struct SwerveBase {
    ctre::phoenix6::hardware::CANcoder frontLeftCancoder{19};
    ctre::phoenix6::hardware::CANcoder frontRightCancoder{17};
    ctre::phoenix6::hardware::CANcoder backLeftCancoder{16};
    ctre::phoenix6::hardware::CANcoder backRightCancoder{18};

    ctre::phoenix6::hardware::Pigeon2* gyro = new ctre::phoenix6::hardware::Pigeon2(20, "Drivebase");
    wpi::array<ctre::phoenix6::hardware::TalonFX*, 4> turnMotors{
        new ctre::phoenix6::hardware::TalonFX(7, "Drivebase"),
        new ctre::phoenix6::hardware::TalonFX(5, "Drivebase"),
        new ctre::phoenix6::hardware::TalonFX(1, "Drivebase"),
        new ctre::phoenix6::hardware::TalonFX(3, "Drivebase")};
    wpi::array<ctre::phoenix6::hardware::TalonFX*, 4> driveMotors{
        new ctre::phoenix6::hardware::TalonFX(9, "Drivebase"),
        new ctre::phoenix6::hardware::TalonFX(6, "Drivebase"),
        new ctre::phoenix6::hardware::TalonFX(2, "Drivebase"),
        new ctre::phoenix6::hardware::TalonFX(4, "Drivebase")};

    wpi::array<wom::SwerveModuleConfig, 4> moduleConfigs{
        wom::SwerveModuleConfig{
            frc::Translation2d(10.761_in, 9.455_in),
            wom::Gearbox{driveMotors[0], new wom::TalonFXEncoder(driveMotors[0], 0.0445_m, 6.75),
                         frc::DCMotor::Falcon500(1).WithReduction(6.75)},
            wom::Gearbox{turnMotors[0], new wom::CanEncoder(19, 0.0445_m, 4096, 12.8),
                         frc::DCMotor::Falcon500(1).WithReduction(12.8)},
            &frontLeftCancoder, 4_in / 2},
        wom::SwerveModuleConfig{
            frc::Translation2d(10.761_in, -9.455_in),
            wom::Gearbox{driveMotors[1], new wom::TalonFXEncoder(driveMotors[1], 0.0445_m, 6.75),
                         frc::DCMotor::Falcon500(1).WithReduction(6.75)},
            wom::Gearbox{turnMotors[1], new wom::CanEncoder(17, 0.0445_m, 4096, 12.8),
                         frc::DCMotor::Falcon500(1).WithReduction(12.8)},
            &frontRightCancoder, 4_in / 2},
        wom::SwerveModuleConfig{
            frc::Translation2d(-10.761_in, 9.455_in),
            wom::Gearbox{driveMotors[2], new wom::TalonFXEncoder(driveMotors[2], 0.0445_m, 6.75),
                         frc::DCMotor::Falcon500(1).WithReduction(6.75)},
            wom::Gearbox{turnMotors[2], new wom::CanEncoder(16, 0.0445_m, 4096, 12.8),
                         frc::DCMotor::Falcon500(1).WithReduction(12.8)},
            &backRightCancoder, 4_in / 2},
        wom::SwerveModuleConfig{
            frc::Translation2d(-10.761_in, -9.455_in),
            wom::Gearbox{driveMotors[3], new wom::TalonFXEncoder(driveMotors[3], 0.0445_m, 6.75),
                         frc::DCMotor::Falcon500(1).WithReduction(6.75)},
            wom::Gearbox{turnMotors[3], new wom::CanEncoder(18, 0.0445_m, 4096, 12.8),
                         frc::DCMotor::Falcon500(1).WithReduction(12.8)},
            &backLeftCancoder, 4_in / 2},
    };

    wom::SwerveModule::angle_pid_conf_t anglePID{
        "/drivetrain/pid/angle/config", 2_V / 360_deg, 0.0_V / (100_deg * 1_s),
        0_V / (100_deg / 1_s),          1_deg,         0.5_deg / 2_s};
    wom::SwerveModule::velocity_pid_conf_t velocityPID{
        "/drivetrain/pid/velocity/config",
    };
    wom::SwerveDriveConfig::pose_angle_conf_t poseAnglePID{
        "/drivetrain/pid/pose/angle/config",
        180_deg / 1_s / 45_deg,
        wom::SwerveDriveConfig::pose_angle_conf_t::ki_t{0.1},
        0_deg / 1_deg,
        10_deg,
        10_deg / 1_s};
    wom::SwerveDriveConfig::pose_position_conf_t posePositionPID{
        "/drivetrain/pid/pose/position/config",
        3_mps / 1_m,
        wom::SwerveDriveConfig::pose_position_conf_t::ki_t{0.15},
        0_m / 1_m,
        20_cm,
        10_cm / 1_s,
        10_cm};

    wom::SwerveDriveConfig config{"/drivetrain",
                                  anglePID,
                                  velocityPID,
                                  moduleConfigs, 
                                  gyro,
                                  poseAnglePID,
                                  posePositionPID,
                                  60_kg, 
                                  {0.1, 0.1, 0.1},
                                  {0.9, 0.9, 0.9}};

  };
  SwerveBase swerveBase;
};