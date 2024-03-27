#include "swerveModule.h"
#include <frc/smartdashboard/SmartDashboard.h>
#include <frc/MathUtil.h>
#include <iostream>
#include <numbers>
#include <thread>
#include <chrono>
#include <frc/geometry/Rotation2d.h>
using namespace drivetrainConstants::calculations;
using namespace generalConstants;

// Constructor for creating and configuring the swerve module motors
swerveModule::swerveModule(const double module[]) 
    : m_motorDrive(module[0], rev::CANSparkMax::MotorType::kBrushless),
    m_motorTurn(module[1], rev::CANSparkMax::MotorType::kBrushless),
    m_encoderTurn(module[2]) {

    // Resets the swerve module motors and encoders to factory settings
    m_motorDrive.RestoreFactoryDefaults();
    m_motorTurn.RestoreFactoryDefaults();

    // Sets both the drive motor and the turn motor to be inverted
    m_motorDrive.SetInverted(true);
    m_motorTurn.SetInverted(true);

    // Sets the idle mode of the swerve module motors to brake (Motors brake when not doing anything)
    m_motorDrive.SetIdleMode(rev::CANSparkMax::IdleMode::kBrake);
    m_motorTurn.SetIdleMode(rev::CANSparkMax::IdleMode::kBrake);

    // Sets current limits for the swerve module motors
    m_motorDrive.SetSmartCurrentLimit(40.0);
    m_motorTurn.SetSmartCurrentLimit(20.0);

    // Burn all settings
    m_motorDrive.BurnFlash();
    m_motorTurn.BurnFlash();

    // Sets the feedback device of the drive motor to the built in motor encoder and the feedback device of the turn motor to the external encoder
    m_driveController.SetFeedbackDevice(m_encoderDrive);
    m_turnController.SetFeedbackDevice(m_encoderTurnIntegrated);

    m_driveController.SetP(0.3);
    m_driveController.SetI(0);
    m_driveController.SetD(0.2);
    m_driveController.SetFF(1.0/4.6);
    m_driveController.SetOutputRange(-1.0F, 1.0F);

    m_turnController.SetP(0.015); //0.55
    m_turnController.SetI(0.0);
    m_turnController.SetD(0.001); //0.3
    m_turnController.SetFF(0.0);
    m_turnController.SetOutputRange(-1.0F, 1.0F);

    // Velocity values for the external turn encoder and the built in drive encoder
    m_encoderTurnIntegrated.SetPositionConversionFactor(2.0 * std::numbers::pi * (kFinalTurnRatio));
    m_encoderTurnIntegrated.SetVelocityConversionFactor((2.0 * std::numbers::pi * (kFinalTurnRatio)) / 60.0);

    m_encoderDrive.SetPositionConversionFactor((kWheelDiameter.value() / 2.0) * 2.0 * std::numbers::pi * (kFinalDriveRatio));
    m_encoderDrive.SetVelocityConversionFactor((kWheelDiameter.value() / 2.0) * (2.0 * std::numbers::pi * (kFinalDriveRatio)) / 60.0);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    m_motorDrive.BurnFlash();
    m_motorTurn.BurnFlash();
}

// Gets the position of the swerve module drive motor and turn motor
frc::SwerveModulePosition swerveModule::GetPosition() {
    return {units::meter_t{m_encoderDrive.GetPosition()}, frc::Rotation2d(frc::AngleModulus(units::degree_t{m_encoderTurn.GetAbsolutePosition().GetValueAsDouble() * kRotationsToDegrees}))};
}

frc::SwerveModuleState swerveModule::GetState() {
    return {units::meters_per_second_t{m_encoderDrive.GetVelocity()}, frc::Rotation2d(frc::AngleModulus(units::degree_t{m_encoderTurn.GetAbsolutePosition().GetValueAsDouble() * kRotationsToDegrees}))};
}

// Applies the wanted speed and direction to the turn and drive motors
void swerveModule::SetDesiredState(const frc::SwerveModuleState& referenceState) {
    const auto state = frc::SwerveModuleState::Optimize(
        referenceState,units::degree_t(m_encoderTurn.GetAbsolutePosition().GetValueAsDouble() * kRotationsToDegrees));
    //This returns the position in +-Cancoder units counting forever, as opposed to absolulte -180 to +180 deg.

    const auto targetWheelSpeed{state.speed};
    m_targetAngle = state.angle.Degrees().value();
    const double turnOutput = m_targetAngle;

    units::radians_per_second_t targetMotorSpeed{
        (targetWheelSpeed * units::radian_t(2 * std::numbers::pi))
        / kWheelCircumference};
    m_driveController.SetReference(targetWheelSpeed.value(), rev::CANSparkMax::ControlType::kVelocity);
    m_encoderTurnIntegrated.SetPosition(m_encoderTurn.GetAbsolutePosition().GetValueAsDouble() * kRotationsToDegrees);
    m_turnController.SetReference(turnOutput, rev::CANSparkMax::ControlType::kPosition);

    frc::SmartDashboard::PutNumber("Target Wheel Speed" + std::to_string(m_motorTurn.GetDeviceId()), targetWheelSpeed.value());
    frc::SmartDashboard::PutNumber("Target Wheel Speed" + std::to_string(m_motorTurn.GetDeviceId()), targetWheelSpeed.value());
    frc::SmartDashboard::PutNumber("Target Motor Speed", targetMotorSpeed.value());
}

// Calculates difference between desired angle and current angle for swerve modules and other values
frc::SwerveModuleState swerveModule::CustomOptimize(const frc::SwerveModuleState& desiredState,
                                                    const frc::Rotation2d& currentAngle) {
    auto modulusAngle{frc::AngleModulus(currentAngle.Degrees())};
    auto optAngle = desiredState.angle;
    auto optSpeed = desiredState.speed;
    
    auto difference = optAngle.Degrees() - modulusAngle;
    frc::SmartDashboard::PutNumber("Difference", difference.value());

    if (difference >= 270_deg) {
        difference = difference - 360_deg;
    } else if (difference <= -270_deg) {
        difference = difference + 360_deg;
    }

    if (units::math::abs(difference) > 90_deg) {
        optSpeed = -desiredState.speed;
        if (difference > 0_deg) {
            difference = difference - 180_deg;
        } else {
            difference = difference + 180_deg;
        }
    }
    optAngle = currentAngle.Degrees() + difference;

    frc::SmartDashboard::PutNumber("Neo Encoder Vel " + std::to_string(m_motorTurn.GetDeviceId()), m_encoderDrive.GetVelocity());

    return {optSpeed, optAngle};
}

// Calculates possible values to use in SmartDashboard
double swerveModule::DashboardInfo(const DataType& type) {
    switch(type) {
        case DataType::kCurrentAngle :
            return {units::degree_t(frc::AngleModulus(units::degree_t(m_encoderTurn.GetAbsolutePosition().GetValueAsDouble() * kRotationsToDegrees))).value()};
        case DataType::kTargetAngle :
            return {m_targetAngle};
        default :
            throw std::invalid_argument("Invalid DashboardInfo DataType");
    }
}