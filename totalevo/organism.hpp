#ifndef ORGANISM_HPP
#define ORGANISM_HPP

#include <scene.hpp>
#include <set>
#include <glm/glm.hpp>
#include <list>
#include "brain.hpp"

#define TE_ORGANISM_TAG 0x554926178
#define TE_FOOD_TAG     0x586917190
#define TE_ATEFOOD_TAG  0x558619719
#define CLIP(x) x = std::max(-1.f, std::min(x, 1.f))
#define CLAMP(x,u,l) x = std::max(l, std::min(x, u))

namespace TE
{
    enum OrganismStatus
    {
        IDLE,
        SEEKING,
        FOUND,
        DEAD,
    };

    enum FoodType
    {
        VEGETABLE,
        MEAT,
    };

#define PROP(x) float x; \
                static float avg_##x;
    struct SpeciesInfo
    {
        Brain* speciesBrain;

        static void CalcAverages();

        void Mutate(); // randomly mutate a trait
        void Init();
        void Deinit();
        void Clip();

        // growth = growthRate * time + pregnancyLength

        PROP(pregnancyLength); // length of pregnancy
        PROP(pregnancyEnergy); // amount of energy to give to baby
        PROP(pregnancyChance); // chance every tick for a pregnancy to occour
        PROP(growthRate); // rate it takes to grow to adult hood
        PROP(birthEnergy); // minimum energy to start pregnancy
        PROP(foodTypePreference); // type of food it can digest
        PROP(sightDistance); // sight requires energy   
        PROP(speed); // speed requires energy
        PROP(scale); // scale of thing, slows it down but lets it digest slower
        PROP(hungerSearchLimit); // lowest hunger before searching time
        PROP(birthBrainMutations); // brain mutations upon birth

        glm::vec3 color;
    };

    class Food : public Scene::MeshNode
    {
    public:
        float energy;
        FoodType type;
        Food(FoodType type, Scene::SceneNode* parent);
        virtual void DbgWidgets();
    };

    class Organism : public Scene::MeshNode
    {
    private:
    public:
        TE::Organism* last_child;
        TE::Organism* parent_genetic;
        static std::list<TE::Organism*>* livingOrganisms;
        SpeciesInfo s;
        OrganismStatus status;
        int generation;

        float energy;
        float health;
        float growthAmount;
        float pregnancyAmount;
        float stamina;
        bool pregnant;
        bool pregnantOk;
        float age;

        bool pause;

        Organism(std::list<TE::Organism*>* livingOrganisms, Scene::SceneNode* parent);
        virtual void Update();
        virtual void DbgWidgets();
        Organism* Birth();
    };
}

#endif