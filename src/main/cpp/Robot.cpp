// Copyright (c) 2023-2024 CurtinFRC
// Open Source Software, you can modify it according to the terms
// of the MIT License at the root of this project

#include "Robot.h"

#include <frc/TimedRobot.h>
#include <frc/Timer.h>
#include <frc/controller/RamseteController.h>
#include <frc/kinematics/DifferentialDriveKinematics.h>
#include <networktables/DoubleTopic.h>
#include <networktables/NetworkTable.h>
#include <networktables/NetworkTableInstance.h>
#include <units/acceleration.h>
#include <units/angle.h>
#include <units/length.h>
#include <units/time.h>
#include <units/velocity.h>
#include <units/voltage.h>

#include "behaviour/HasBehaviour.h"

#include "Auto.h"

static units::second_t lastPeriodic;

void Robot::RobotInit() {
  shooter = new Shooter(robotmap.shooterSystem.config);
  wom::BehaviourScheduler::GetInstance()->Register(shooter);
  shooter->SetDefaultBehaviour(
      [this]() { return wom::make<ShooterManualControl>(shooter, &robotmap.controllers.codriver); });

  sched = wom::BehaviourScheduler::GetInstance();
  m_chooser.SetDefaultOption("kTaxi", "kTaxi");

  for (auto &option : autoOptions) {
    m_chooser.AddOption(option, option);
  }

  frc::SmartDashboard::PutData("Auto Modes", &m_chooser);

  // m_path_chooser.SetDefaultOption("Path1", "paths/output/Path1.wpilib.json");

  // m_path_chooser.AddOption("Path1", "paths/output/Path1.wpilib.json");
  // m_path_chooser.AddOption("Path2", "paths/output/Path2.wpilib.json");

  // frc::SmartDashboard::PutData("Path Selector", &m_path_chooser);

  // frc::SmartDashboard::PutData("Field", &m_field);

  timer = frc::Timer();

  // robotmap.swerveBase.gyro->Reset();

  // _swerveDrive = new wom::SwerveDrive(robotmap.swerveBase.config, frc::Pose2d());
  // wom::BehaviourScheduler::GetInstance()->Register(_swerveDrive);
  // _swerveDrive->SetDefaultBehaviour(
  //     [this]() { return wom::make<wom::ManualDrivebase>(_swerveDrive, &robotmap.controllers.driver); });
  _swerveDrive = new wom::SwerveDrive(robotmap.swerveBase.config, frc::Pose2d());
  wom::BehaviourScheduler::GetInstance()->Register(_swerveDrive);
  _swerveDrive->SetDefaultBehaviour(
      [this]() { return wom::make<wom::ManualDrivebase>(_swerveDrive, &robotmap.controllers.driver); });

  // m_driveSim = new wom::TempSimSwerveDrive(&simulation_timer, &m_field);
  // m_driveSim = wom::TempSimSwerveDrive();

  alphaArm = new AlphaArm(robotmap.alphaArmSystem.config);
  wom::BehaviourScheduler::GetInstance()->Register(alphaArm);
  alphaArm->SetDefaultBehaviour(
      [this]() { return wom::make<AlphaArmManualControl>(alphaArm, &robotmap.controllers.codriver); });

  robotmap.swerveBase.moduleConfigs[0].turnMotor.encoder->SetEncoderOffset(0_rad);
  robotmap.swerveBase.moduleConfigs[1].turnMotor.encoder->SetEncoderOffset(0_rad);
  robotmap.swerveBase.moduleConfigs[2].turnMotor.encoder->SetEncoderOffset(0_rad);
  robotmap.swerveBase.moduleConfigs[3].turnMotor.encoder->SetEncoderOffset(0_rad);

  // frontLeft = new ctre::phoenix6::hardware::TalonFX(7, "Drivebase");  // front left
  // frontRight = new ctre::phoenix6::hardware::TalonFX(2, "Drivebase");   // front right
  // backLeft = new ctre::phoenix6::hardware::TalonFX(6, "Drivebase");   // back left
  // backRight = new ctre::phoenix6::hardware::TalonFX(4, "Drivebase");  // back right
  // frontLeft = new ctre::phoenix6::hardware::TalonFX(9, "Drivebase");   // front left
  // frontRight = new ctre::phoenix6::hardware::TalonFX(1, "Drivebase");   // front right
  // backLeft = new ctre::phoenix6::hardware::TalonFX(5, "Drivebase");   // back left
  // backRight = new ctre::phoenix6::hardware::TalonFX(3, "Drivebase");
  lastPeriodic = wom::now();

  intake = new Intake(robotmap.intakeSystem.config);
  wom::BehaviourScheduler::GetInstance()->Register(intake);
  intake->SetDefaultBehaviour(
      [this]() { return wom::make<IntakeManualControl>(intake, robotmap.controllers.codriver); });
}

void Robot::RobotPeriodic() {
  auto dt = wom::now() - lastPeriodic;
  lastPeriodic = wom::now();

  loop.Poll();
  wom::BehaviourScheduler::GetInstance()->Tick();
  shooter->OnUpdate(dt);
  sched->Tick();

  robotmap.swerveTable.swerveDriveTable->GetEntry("frontLeftEncoder")
      .SetDouble(robotmap.swerveBase.moduleConfigs[0].turnMotor.encoder->GetEncoderPosition().value());
  robotmap.swerveTable.swerveDriveTable->GetEntry("frontRightEncoder")
      .SetDouble(robotmap.swerveBase.moduleConfigs[1].turnMotor.encoder->GetEncoderPosition().value());
  robotmap.swerveTable.swerveDriveTable->GetEntry("backLeftEncoder")
      .SetDouble(robotmap.swerveBase.moduleConfigs[2].turnMotor.encoder->GetEncoderPosition().value());
  robotmap.swerveTable.swerveDriveTable->GetEntry("backRightEncoder")
      .SetDouble(robotmap.swerveBase.moduleConfigs[3].turnMotor.encoder->GetEncoderPosition().value());

  _swerveDrive->OnUpdate(dt);
  alphaArm->OnUpdate(dt);
  intake->OnUpdate(dt);

}

void Robot::AutonomousInit() {
  loop.Clear();
  sched->InterruptAll();

  m_autoSelected = m_chooser.GetSelected();
  fmt::print("Auto selected: {}\n", m_autoSelected);
  if (m_autoSelected == "Taxi") {
    sched->Schedule(autos::Taxi(_swerveDrive, &timer, &field, shooter, intake, alphaArm));
  } else if (m_autoSelected == "Auto Test") {
    sched->Schedule(autos::AutoTest(_swerveDrive, &timer, &field, shooter, intake, alphaArm));
  } else if (m_autoSelected == "Quadruple Close") {
    sched->Schedule(autos::QuadrupleClose(_swerveDrive, &timer, &field, shooter, intake, alphaArm));
  } else if (m_autoSelected == "Quadruple Far") {
    sched->Schedule(autos::QuadrupleFar(_swerveDrive, &timer, &field, shooter, intake, alphaArm));
  } else if (m_autoSelected == "Quadruple Close Double Far") {
    sched->Schedule(autos::QuadrupleCloseDoubleFar(_swerveDrive, &timer, &field, shooter, intake, alphaArm));
  } else if (m_autoSelected == "Quadruple Close Single Far") {
    sched->Schedule(autos::QuadrupleCloseSingleFar(_swerveDrive, &timer, &field, shooter, intake, alphaArm));
  }
}
void Robot::AutonomousPeriodic() {}

void Robot::TeleopInit() {
  loop.Clear();
  wom::BehaviourScheduler* sched = wom::BehaviourScheduler::GetInstance();
  sched->InterruptAll();

  // frontLeft->SetVoltage(4_V);
  // frontRight->SetVoltage(4_V);
  // backLeft->SetVoltage(4_V);
  // backRight->SetVoltage(4_V);
  loop.Clear();
  wom::BehaviourScheduler* scheduler = wom::BehaviourScheduler::GetInstance();
  scheduler->InterruptAll();
}

void Robot::TeleopPeriodic() {}

void Robot::DisabledInit() {}
void Robot::DisabledPeriodic() {}

void Robot::TestInit() {}
void Robot::TestPeriodic() {}
