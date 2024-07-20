// Copyright (c) FIRST and other WPILib contributors.
// Open Source Software; you can modify and/or share it under the terms of
// the WPILib BSD license file in the root directory of this project.

#include "RobotContainer.h"


using namespace generalConstants;
RobotContainer::RobotContainer() {

  // Initialize all of your commands and subsystems here
  pathplanner::NamedCommands::registerCommand("spinup", std::move(spinup(&m_intakeshooter, 115.5).ToPtr()));
  pathplanner::NamedCommands::registerCommand("fire", std::move(fire(&m_intakeshooter).ToPtr()));
  pathplanner::NamedCommands::registerCommand("intakeActivate", std::move(intakeActivate(&m_intakeshooter).ToPtr()));
  pathplanner::NamedCommands::registerCommand("intakeRetract", std::move(intakeRetract(&m_intakeshooter).ToPtr()));
  pathplanner::NamedCommands::registerCommand("rapidFire", std::move(rapidFire(&m_intakeshooter).ToPtr()));
  // Configure the button bindings
  ConfigureButtonBindings();

  // $ CONTROLLER INPUTS FOR SWERVE DRIVE BELOW
  m_drivetrain.SetDefaultCommand(Drive(
    &m_drivetrain,
    [this] { return (MathFunctions::joystickCurve((m_controllerMain.GetLeftX()), controllerConstants::kControllerCurve)); },
    [this] { return (MathFunctions::joystickCurve((m_controllerMain.GetLeftY()), controllerConstants::kControllerCurve)); },
    [this] { return (m_controllerMain.GetRawAxis(4)); }));
  

    m_chooser.SetDefaultOption("DoNothing", "DoNothing");
    m_chooser.AddOption("2NoteCenter", "2NoteCenter");
    m_chooser.AddOption("2NoteAmpSide", "2NoteAmpSide");
    m_chooser.AddOption("3NoteAmpSide", "3NoteAmpSide");
    m_chooser.AddOption("3NoteCenter", "3NoteCenter");
    m_chooser.AddOption("4Note", "4Note");
    m_chooser.AddOption("Backup", "Backup");
    m_chooser.AddOption("Preload", "Preload");
    m_chooser.AddOption("PreloadBackupCenter", "PreloadBackupCenter");
    m_chooser.AddOption("New Auto", "New Auto");

    frc::SmartDashboard::PutData(&m_chooser);
    
    // Start Timer
    timer.Start();
  
}

// All the button commands are set in this function
void RobotContainer::ConfigureButtonBindings() {

  // Zeroing for swervedrive command
  m_controllerMain.Start().OnTrue(m_drivetrain.resetGyro());

  // Slow button for swerve (whenever left OR right bumper is held down), slows swerve to slow value
  (m_controllerMain.RightBumper() || m_controllerMain.LeftBumper()).WhileTrue(m_drivetrain.slowDown());

  // Align
  m_controllerMain.B().WhileTrue(m_drivetrain.Align(&m_vision));
  m_controllerMain.A().WhileTrue(m_drivetrain.Run([this]{m_drivetrain.xStance();}));

  // ShooterIntake buttons
  m_controllerOperator.LeftBumper().OnTrue(intakeActivate(&m_intakeshooter).ToPtr()); // kA
  m_controllerOperator.B().OnTrue(AutoAngle(&m_intakeshooter, &m_vision).ToPtr()); // spinup for far speaker shot (7 feet from speaker) !!!!96.6 (111.5)
  m_controllerOperator.X().OnTrue(spinup(&m_intakeshooter, intakeConstants::kIntakeSpeakerAngle).ToPtr()); // spinup for near speaker shot (right at speaker) Y !!!!!110.0
  m_controllerOperator.RightBumper().OnTrue(fire(&m_intakeshooter).ToPtr());
  m_controllerOperator.A().OnTrue(intakeRetract(&m_intakeshooter).ToPtr()); //leftbumper
  m_controllerOperator.Y().OnTrue(scoreAmp(&m_intakeshooter).ToPtr());
  m_controllerOperator.Start().OnTrue(m_intakeshooter.zeroOTF());
  
	// Climber Buttons
	m_controllerOperator.LeftStick().OnTrue(m_climber.climberExtend());
	m_controllerOperator.RightStick().OnTrue(m_climber.climberRetract());

	// Climber Zero Maintinence Buttons
	m_controllerOperator.LeftTrigger().WhileTrue(m_climber.leftClimbZero());
	m_controllerOperator.RightTrigger().WhileTrue(m_climber.rightClimbZero());
}

frc2::CommandPtr RobotContainer::GetAutonomousCommand() {
  using namespace pathplanner;
  return m_chooser.GetSelected() == "DoNothing" ? frc2::WaitCommand(15_s).ToPtr() : PathPlannerAuto(m_chooser.GetSelected()).ToPtr();
}


