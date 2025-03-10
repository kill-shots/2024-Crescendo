#include "IntakeBehaviour.h"
#include <frc/XboxController.h>

IntakeManualControl::IntakeManualControl(Intake *intake, frc::XboxController &codriver) : _intake(intake), _codriver(codriver) {
  Controls(intake);
}

void IntakeManualControl::OnTick(units::second_t dt) {

  if (_codriver.GetBButtonPressed()) {
    if(_rawControl == true) {
      _rawControl = false;
    } else {
      _rawControl = true;
    }
  }

  if (_rawControl) {
    _intake->setState(IntakeState::kRaw);
    _intake->setRaw(_codriver.GetLeftY() * 5_V);
  } else {
    if (_codriver.GetYButtonPressed()) {
      _intake->setState(IntakeState::kIntake);
    }
    if (_codriver.GetAButtonPressed()) {
      _intake->setState(IntakeState::kPass);
    }
  }
}

IntakeAutoControl::IntakeAutoControl(Intake *intake) : _intake(intake) {}

void IntakeAutoControl::OnTick(units::second_t dt) {
  if (_intake->GetConfig().intakeSensor->Get() == 1) {
    _intake->setState(IntakeState::kPass);
  } else if (_intake->GetConfig().magSensor->Get() == 0) {
    _intake->setState(IntakeState::kIdle);
  }
}