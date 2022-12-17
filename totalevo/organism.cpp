#include "organism.hpp"
#include "engine.hpp"
#include "imgui/implot.h"
#include <random>

#define RATE_METABOLIZE 0.1f
#define NEURON_ENERGY 0.1f
#define SPEED_RATE 0.5f
#define MOVESPEED_RATE 0.05f
#define RATE_AGING 0.1f

std::list<TE::Organism*>* TE::Organism::livingOrganisms = nullptr;
static std::random_device rd;
static std::mt19937 gen(rd());
static std::uniform_real_distribution<> startposition_distr(-100, 100);

TE::Food::Food(FoodType type, Scene::SceneNode* parent) : MeshNode(parent)
{
    mesh = engine_app->mesh_manager->GetMesh("teapot.obj");
    shader = engine_app->shader_manager->GetShader("simple");
    SetName("FoodBit");
    tag = TE_FOOD_TAG;
    this->type = type;
    switch(type)
    {
        case VEGETABLE:
            transform.color = glm::vec4(0.f,1.f,0.f,1.f);
            break;
        case MEAT:
            transform.color = glm::vec4(1.f,0.f,0.f,1.f);
            break;
    }
}

void TE::Food::DbgWidgets()
{
    ImGui::SliderFloat("Energy contained", &energy, 0.f, 100.f);
}

void TE::SpeciesInfo::Mutate()
{
    std::uniform_real_distribution<> distrb(-2.5, 2.5);
    std::uniform_real_distribution<> distr(-0.25, 0.25);
    std::uniform_real_distribution<> distrs(-0.025, 0.025);
    std::uniform_int_distribution<> distri(-1, 1);

    growthRate += distr(gen); 
    pregnancyLength += distr(gen); 
    pregnancyEnergy += distrb(gen); 
    birthEnergy += distrb(gen);
    foodTypePreference += distr(gen);
    sightDistance += distrb(gen);
    pregnancyChance += distrs(gen);
    hungerSearchLimit += distrb(gen);
    speed += distr(gen);
    scale += distr(gen);
    birthBrainMutations += distrb(gen);

    color.x += distrs(gen);
    color.y += distrs(gen);
    color.z += distrs(gen);

    Clip();
}

void TE::SpeciesInfo::Clip()
{
    CLAMP(growthRate, 5.f, 0.1f);
    CLAMP(pregnancyEnergy, 100.f, 0.1f);
    CLAMP(pregnancyLength, 25.f, 0.1f);
    CLAMP(pregnancyChance, 1.f, 0.f);
    CLAMP(birthEnergy, 100.f, 0.1f);
    CLAMP(sightDistance, 100.f, 0.0f);
    CLAMP(speed, 5.f, 0.0f);
    CLAMP(scale, 5.f, 0.1f);
    CLAMP(hungerSearchLimit, 100.f, 0.f);
    CLAMP(birthBrainMutations, 10.f, 1.f);
    CLIP(foodTypePreference);

    CLAMP(color.x,1.f,0.f);
    CLAMP(color.y,1.f,0.f);
    CLAMP(color.z,1.f,0.f);
}

void TE::SpeciesInfo::Deinit()
{
    delete speciesBrain;
    speciesBrain = nullptr;
}

void TE::SpeciesInfo::Init()
{
    growthRate = 0.1f;
    pregnancyLength = 1.f;
    pregnancyEnergy = 45.f;
    birthEnergy = 50.f;
    foodTypePreference = 0.f;
    sightDistance = 50.f;
    speed = 0.5f;
    pregnancyChance = 0.1f; 
    hungerSearchLimit = 50.f;
    scale = 1.0f;
    color.x = 0.5f;
    color.y = 0.5f;
    color.z = 0.5f;
    birthBrainMutations = 2.f;

    speciesBrain = new Brain();

    Clip();
}

TE::Organism::Organism(std::list<TE::Organism*>* livingOrganisms, SceneNode* parent) : 
    Scene::MeshNode(parent)
{
    mesh = engine_app->mesh_manager->GetMesh("cell.gltf");
    shader = engine_app->shader_manager->GetShader("textured");

    transform.pos.x = startposition_distr(gen);
    transform.pos.z = startposition_distr(gen);

    tag = TE_ORGANISM_TAG;
    status = IDLE;
    health = 100.f;
    age = 0.0f;
    pause = false;
    SetName("Organism");
    energy = 0.0f;
    generation = 0;
    last_child = nullptr;
    parent_genetic = nullptr;
    this->livingOrganisms = livingOrganisms;
}

void TE::Organism::Update()
{
    if(pause)
        return;

    age += 0.01f * engine_app->delta_time;
    
    if(status == DEAD)
        return;

    if(energy >= 35.f && growthAmount != 1.0f)
    {
        growthAmount += (s.growthRate) * SPEED_RATE;
        if(s.speciesBrain && engine_app->frame_counter % 15 == 0) 
            s.speciesBrain->Mutate();
        energy -= (s.growthRate) * SPEED_RATE;    
    }
    energy -= (RATE_AGING * (age / 120.f)) * SPEED_RATE;
    energy -= (s.sightDistance * 0.001f) * SPEED_RATE;
    if(s.speciesBrain)
    {
        energy -= (s.speciesBrain->neurons->size() * 0.001f) * SPEED_RATE;
        energy -= (std::abs(s.speciesBrain->outputs[NON_TOXINRANGE]) * 0.01f) * SPEED_RATE;
    }

    CLAMP(growthAmount, 1.f, 0.f);

    CLAMP(energy, 100.f, 0.f);
    if(energy <= 0.f)
    {
        // start dying
        health -= ((RATE_AGING * (age / 30.f) * (s.scale * 10.f)) + RATE_METABOLIZE) * SPEED_RATE;
    }

    CLAMP(health, 100.f, 0.f);
    if(health == 0.f)
    {
        status = DEAD;
    }
    Scene::SceneNode::FindResult budder = parent->GetChildClosestTo(TE_ORGANISM_TAG, transform.pos, this);
    if(budder.found && budder.distance < s.scale * s.speciesBrain->outputs[NON_TOXINRANGE])
    {
        Organism* organism = (Organism*)budder.found;
        if(organism != this)
        {
            organism->health -= 2.f * (s.speciesBrain->outputs[NON_TOXINRANGE]-budder.distance) * engine_app->delta_time;
            organism->energy -= 4.f * (s.speciesBrain->outputs[NON_TOXINRANGE]-budder.distance) * engine_app->delta_time;
        }
    }

    if(transform.pos.x > 200.f)
        transform.pos.x = 200.f;
    if(transform.pos.z > 200.f)
        transform.pos.z = 200.f;
    if(transform.pos.x < -200.f)
        transform.pos.x = -200.f;
    if(transform.pos.z < -200.f)
        transform.pos.z = -200.f;

    if(pregnant)
    {
        pregnancyAmount += 0.01f * engine_app->delta_time;
        CLAMP(pregnancyAmount,s.pregnancyLength*s.scale,0.f);
        if(pregnancyAmount == s.pregnancyLength*s.scale)
        {
            pregnantOk = true;
            pregnant = false;
        }
    }

    if(s.speciesBrain)
    {
        s.speciesBrain->inputs[NIN_STAMINA] = stamina;
        s.speciesBrain->inputs[NIN_ENERGY] = energy/100.f;
        s.speciesBrain->inputs[NIN_HEALTH] = health/100.f;
        s.speciesBrain->inputs[NIN_PREGNANCY] = pregnancyAmount/(s.pregnancyLength*s.scale);
        s.speciesBrain->inputs[NIN_RS] = std::sin(transform.eulerRot.z);
        s.speciesBrain->inputs[NIN_RC] = std::cos(transform.eulerRot.z);
        s.speciesBrain->inputs[NIN_PX] = transform.pos.x/200.f;
        s.speciesBrain->inputs[NIN_PY] = transform.pos.z/200.f;
        s.speciesBrain->inputs[NIN_CR1] = std::sin(((float)engine_app->frame_counter)/10.f);
        s.speciesBrain->inputs[NIN_CR2] = std::cos(((float)engine_app->frame_counter)/100.f);
        Scene::SceneNode::FindResult nearestfood = parent->GetChildClosestTo(TE_FOOD_TAG, transform.pos);
        if(nearestfood.found)
            if(nearestfood.distance < s.sightDistance)
            {
                Food* food = (Food*)nearestfood.found;
                s.speciesBrain->inputs[NIN_FPX] = glm::distance(nearestfood.found->transform.pos.x, transform.pos.x)/s.sightDistance;
                s.speciesBrain->inputs[NIN_FPY] = glm::distance(nearestfood.found->transform.pos.z, transform.pos.z)/s.sightDistance;
                s.speciesBrain->inputs[NIN_FOODLOOK] = ((int)food->type)-1;
            }
        if(budder.found)
            if(budder.distance < s.sightDistance)
            {
                s.speciesBrain->inputs[NIN_ODISTANCE] = budder.distance/s.sightDistance;
            }
        if(last_child && last_child->tag == TE_ORGANISM_TAG)
            s.speciesBrain->inputs[NIN_LCDISTANCE] = glm::distance(last_child->transform.pos, transform.pos)/200.f;
        s.speciesBrain->inputs[NIN_GROWTH] = growthAmount;
        s.speciesBrain->Evaluate(true);
        transform.eulerRot.y += s.speciesBrain->outputs[NON_ROTATE];
        float totalSpeed = std::abs(std::max(0.15f,stamina) * std::sin(transform.eulerRot.z) * s.speciesBrain->outputs[NON_MOVEFORWARD] * engine_app->delta_time + std::max(0.15f,stamina) * std::cos(transform.eulerRot.y) * s.speciesBrain->outputs[NON_MOVEFORWARD] * engine_app->delta_time);
        totalSpeed += std::abs(std::max(0.15f,stamina) * std::cos(transform.eulerRot.z) * s.speciesBrain->outputs[NON_MOVESIDEWAYS] * engine_app->delta_time + std::max(0.15f,stamina) * std::sin(transform.eulerRot.y) * s.speciesBrain->outputs[NON_MOVESIDEWAYS] * engine_app->delta_time);
        energy -= totalSpeed * engine_app->delta_time * 0.0001f;
        transform.pos.x += std::max(0.15f,stamina) * std::sin(transform.eulerRot.z) * s.speciesBrain->outputs[NON_MOVEFORWARD] * engine_app->delta_time * 0.1f;
        transform.pos.z += std::max(0.15f,stamina) * std::cos(transform.eulerRot.z) * s.speciesBrain->outputs[NON_MOVEFORWARD] * engine_app->delta_time * 0.1f;
        transform.pos.x += std::max(0.15f,stamina) * std::cos(transform.eulerRot.z) * s.speciesBrain->outputs[NON_MOVESIDEWAYS] * engine_app->delta_time * 0.1f;
        transform.pos.z += std::max(0.15f,stamina) * std::sin(transform.eulerRot.z) * s.speciesBrain->outputs[NON_MOVESIDEWAYS] * engine_app->delta_time * 0.1f;
        if(totalSpeed > 0.5f)
        {
            stamina -= 0.01f * totalSpeed * engine_app->delta_time;
        }
        if(totalSpeed < 0.5f)
        {
            stamina += 0.01f * totalSpeed * engine_app->delta_time;
        }
        CLAMP(stamina, 1.0f, 0.f);
        float eatDistance = ((std::abs(s.speciesBrain->outputs[NON_MOUTHSIZE]) + 0.5f) * 15.f) * (s.scale * growthAmount);
        CLAMP(eatDistance, 15.f, 0.f);
        energy -= eatDistance * engine_app->delta_time * 0.001f;
        if(nearestfood.found)
            if(nearestfood.distance < s.scale || nearestfood.distance < eatDistance)
            {
                Food* food = (Food*)nearestfood.found;
                food->parent->children.remove(food);
                switch(food->type)
                {
                    case VEGETABLE:
                        energy += (-0.8f * s.foodTypePreference + 0.2f) * food->energy;
                        break;
                    case MEAT:
                        energy += (0.25f * s.foodTypePreference + 0.25f) * food->energy;
                        break;
                    default:
                        energy += food->energy;
                        break;
                }
                delete nearestfood.found;
            }
    }

    transform.scale = glm::vec3(1) * growthAmount * s.scale * std::max(0.5f, energy/100.f) * std::max(0.5f, health/100.f);
    transform.color = glm::vec4(s.color, 1.f);
    transform.color.g -= (100.0 - health)/100.0;
    transform.color.b -= (100.0 - health)/100.0;
}

void TE::Organism::DbgWidgets()
{
    ImGui::InputInt("Generation", &generation);
    ImGui::InputInt("General Status", (int*)&status);
    ImGui::Checkbox("Pause", &pause);
    ImGui::SliderFloat("Health", &health, 0.f, 100.f);
    ImGui::SliderFloat("Stamina", &stamina, 0.f, 1.f);
    ImGui::SliderFloat("Energy", &energy, 0.f, 100.f);
    ImGui::SliderFloat("Growth", &growthAmount, 0.f, 1.0f);
    ImGui::InputFloat("Age", &age);
    ImGui::Separator();
    if(ImGui::Button("Rejuvenate"))
    {
        status = IDLE;
        health = 100.f;
        energy = 100.f;
    }
    if(ImGui::Button("Kill"))
    {
        health = 0.f;
        energy = 0.f;
        status = DEAD;
    }
    ImGui::Text("Genetics");
    if(ImGui::Button("Mutate"))
        s.Mutate();
    if(ImGui::Button("Brain Mutate"))
        s.speciesBrain->Mutate();
    if(ImGui::Button("Clip"))
        s.Clip();
    if(ImGui::Button("Init"))
        s.Init();
    ImGui::SliderFloat3("Genetic Color", (float*)&s.color, 0.0f, 1.f);
    ImGui::SliderFloat("Birth Energy Min", &s.birthEnergy, 0.1f, 100.f);
    ImGui::SliderFloat("Birth Energy Cost", &s.pregnancyEnergy, 0.1f, 100.f);
    ImGui::SliderFloat("Birth Chance", &s.pregnancyChance, 0.f, 1.f);
    ImGui::SliderFloat("Birth Length", &s.pregnancyLength, 0.1f, 25.f);
    ImGui::SliderFloat("Food Preference", &s.foodTypePreference, -1.f, 1.f);
    ImGui::SliderFloat("Growth Rate", &s.growthRate, 0.1f, 5.f);
    ImGui::SliderFloat("Sight Range", &s.sightDistance, 0.f, 100.f);
    ImGui::SliderFloat("Scale", &s.scale, 0.1f, 5.f);
}

TE::Organism* TE::Organism::Birth()
{
    Organism* newOrganism = new TE::Organism(livingOrganisms,parent);
    energy -= s.pregnancyEnergy;
    if(energy < 0)
    {
        health = -100.f; // we die for the baby
        newOrganism->energy = 0.f;
    }
    else
    {
        newOrganism->energy = s.pregnancyEnergy;
    }
    newOrganism->transform.pos.x = transform.pos.x;
    newOrganism->transform.pos.y = transform.pos.y;
    newOrganism->transform.pos.z = transform.pos.z;
    newOrganism->s = s;
    newOrganism->s.Mutate();
    newOrganism->s.speciesBrain = new Brain();
    s.speciesBrain->Copy(newOrganism->s.speciesBrain);
    for(int i = 0; i < std::round(s.birthBrainMutations); i++)
        newOrganism->s.speciesBrain->Mutate();
    newOrganism->generation = generation + 1; 
    last_child = newOrganism;       
    newOrganism->parent_genetic = this;
    return newOrganism;
}