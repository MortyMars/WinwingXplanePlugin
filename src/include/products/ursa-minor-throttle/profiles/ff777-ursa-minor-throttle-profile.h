#ifndef FF777_URSA_MINOR_THROTTLE_PROFILE_H
#define FF777_URSA_MINOR_THROTTLE_PROFILE_H

#include "ursa-minor-throttle-aircraft-profile.h"

#include <string>

class FF777UrsaMinorThrottleProfile : public UrsaMinorThrottleAircraftProfile {
    private:
        int lastVibration = 0;
        float lastGForce = 0.0f;
        std::string trimText;

    public:
        FF777UrsaMinorThrottleProfile(ProductUrsaMinorThrottle *product);
        ~FF777UrsaMinorThrottleProfile();

        static bool IsEligible();

        void update() override;

        const std::unordered_map<uint16_t, UrsaMinorThrottleButtonDef> &buttonDefs() const override;
        void buttonPressed(const UrsaMinorThrottleButtonDef *button, XPLMCommandPhase phase) override;

        void updateDisplays() override;
};

#endif
