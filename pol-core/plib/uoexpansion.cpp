#include "uoexpansion.h"

namespace Pol {
    namespace Plib {

        char *ExpansionNames[numExpansions] = {
            "T2A", "LBR", "AOS",
            "SE", "ML", "KR",
            "SA", "HSA"
        };
        B9Feature ExpansionFlags[numExpansions] = {
            B9Feature::T2A, B9Feature::LBR, B9Feature::AOS,
            B9Feature::SE, B9Feature::ML, B9Feature::KRFaces,
            B9Feature::SA, B9Feature::HSA
        };

        const char* getExpansionName(ExpansionVersion x) {
            if (x > ExpansionVersion::LastVersion)
                return "";

            return ExpansionNames[static_cast<u8>(x)];
        }

    }
}