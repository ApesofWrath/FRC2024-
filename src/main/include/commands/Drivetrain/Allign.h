#pragma once

#include <frc2/command/Command.h>
#include <frc2/command/CommandHelper.h>
#include <Constants.h>

#include <subsystems/vision.h>
#include <subsystems/drivetrain.h>

class Allign : public frc2::CommandHelper<frc2::Command, Allign> {
    public:
    explicit Allign(drivetrain* m_drivetrain, vision* m_vision);

    void Initialize() override;

    void Execute() override;

    bool IsFinished() override;

    private:
    drivetrain* m_drivetrain;
    vision* m_vision;
};