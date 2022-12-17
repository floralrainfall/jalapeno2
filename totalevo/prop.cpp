#include "organism.hpp"

#define DFPROP(y) float TE::SpeciesInfo::avg_##y = 0.f;
DFPROP(pregnancyLength); 
DFPROP(pregnancyEnergy); 
DFPROP(pregnancyChance); 
DFPROP(growthRate); 
DFPROP(birthEnergy);
DFPROP(foodTypePreference);
DFPROP(sightDistance); 
DFPROP(speed); 
DFPROP(scale); 
DFPROP(hungerSearchLimit); 
DFPROP(birthBrainMutations); 
#define RMPROP(y) avg_##y = 0.f;
#define DVPROP(y) avg_##y /= Organism::livingOrganisms->size();
#undef PROP;

#define PROP(y) avg_##y += organism->s.y;
// I know no other way of doing this WITHOUT implementing reflection, which I dont want to do
void TE::SpeciesInfo::CalcAverages()
{
    RMPROP(pregnancyLength); 
    RMPROP(pregnancyEnergy); 
    RMPROP(pregnancyChance); 
    RMPROP(growthRate); 
    RMPROP(birthEnergy);
    RMPROP(foodTypePreference);
    RMPROP(sightDistance); 
    RMPROP(speed); 
    RMPROP(scale); 
    RMPROP(hungerSearchLimit); 
    RMPROP(birthBrainMutations); 
    for(auto&& organism : *Organism::livingOrganisms)
    {
        PROP(pregnancyLength); // length of pregnancy
        PROP(pregnancyEnergy); // amount of energy to give to baby
        PROP(pregnancyChance); // chance every tick for a pregnancy to occour
        PROP(growthRate); // rate it takes to grow to adult hood
        PROP(birthEnergy); // minimum energy to start pregnancy
        PROP(foodTypePreference); // type of food it can digest
        PROP(sightDistance); // sight requires energy
        PROP(speed); // speed requires energy
        PROP(scale); 
        PROP(hungerSearchLimit); 
        PROP(birthBrainMutations); 
    }
    DVPROP(pregnancyLength); 
    DVPROP(pregnancyEnergy); 
    DVPROP(pregnancyChance); 
    DVPROP(growthRate); 
    DVPROP(birthEnergy);
    DVPROP(foodTypePreference);
    DVPROP(sightDistance); 
    DVPROP(speed); 
    DVPROP(scale); 
    DVPROP(hungerSearchLimit); 
    DVPROP(birthBrainMutations); 
}
