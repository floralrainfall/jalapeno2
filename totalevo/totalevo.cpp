#include <engine.hpp>
#include <console.hpp>
#include <ui.hpp>
#include <bgfx/bgfx.h>
#include <imgui/imgui.h>
#include <imgui/custom.hpp>
#include "imgui/implot.h"
#include "organism.hpp"
#include "filesystem.hpp"
#include <random>

static std::random_device rd;
static std::mt19937 gen(rd());
static std::uniform_real_distribution<> startposition_distr(-200, 200);

#define MAX_ORGANISMS 1000
#define NO_MORE_FOOD_DROPS 200

char* implot_license = R"(Copyright (c) 2022 Evan Pezent

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.)";

struct ScrollingBuffer {
    int MaxSize;
    int Offset;
    ImVector<ImVec2> Data;
    ScrollingBuffer(int max_size = 2000) {
        MaxSize = max_size;
        Offset  = 0;
        Data.reserve(MaxSize);
    }
    void AddPoint(float x, float y) {
        if (Data.size() < MaxSize)
            Data.push_back(ImVec2(x,y));
        else {
            Data[Offset] = ImVec2(x,y);
            Offset =  (Offset + 1) % MaxSize;
        }
    }
    void Erase() {
        if (Data.size() > 0) {
            Data.shrink(0);
            Offset  = 0;
        }
    }
};

class TEApp : public Engine::App
{
public:
    ENGINE_VERSION_CHECK;
    
    virtual char* GetAppName() { return "Total Evo"; }
    virtual char* GetAppDescription() { return "Simple evolution game"; }

    Scene::DirectionalLightNode* d1;
    std::list<TE::Organism*> livingOrganisms;
    TE::Organism* selectedOrganism = 0;
    TE::Organism* trackedOrganism = 0;

    virtual void ClientInit()
    {
        IO::FileSystem::AddDataPath({.path = "totalevo"});
        PreLoad(Engine::LT_MESH, "cell.gltf");
        PreLoad(Engine::LT_MESH, "teapot.obj");
        bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0xA7A69DFF);
        Debug::Console::enabled = false;
        Debug::Console::accessible = true;
    }

    virtual void Init()
    {
        ImPlot::CreateContext();

        camera.proj = glm::perspective(45.f,1.f,0.1f,1000.0f);
        camera.up = camera.worldup;
        camera.target = glm::vec3(0,0,0);
        camera.position = glm::vec3(400,100,100);

        d1 = new Scene::DirectionalLightNode(root_node);
        d1->ambient = glm::vec4(1.0f,0.9f,0.7f,1);
        d1->diffuse = glm::vec4(1.0f,0.9f,0.9f,1);
        d1->specular = glm::vec4(1.0f,1.0f,0.9f,1);
        d1->direction = glm::vec4(1.0f,-1.0f,1.0f,1);
    }

    float averageGrowth;
    float averageHealth;
    float averageEnergy;
    float averageAge;

    int bornOnThisTick = 0;
    int killedOnThisTick = 0;

    enum {CT_FOLLOW, CT_FIRSTPERSON} ctmode;

    virtual void PreTick()
    {
        TE::Brain::average_neurons = 0.f;
        TE::Brain::total_neuron_count = 0;
    }

    virtual void Tick()
    {
        if(engine_app->pauseGame)
            return;

        TE::Brain::average_neurons /= livingOrganisms.size();
        std::vector<TE::Organism*> newOrganisms;
        std::vector<TE::Organism*> deadOrganisms;
        averageGrowth = 0;
        averageHealth = 0;
        averageEnergy = 0;
        averageAge = 0;
        for(auto&& organism : livingOrganisms)
        {
            averageGrowth += organism->growthAmount;
            averageHealth += organism->health;
            averageEnergy += organism->energy;
            averageAge += organism->age;

            if(organism->pause)
                continue;

            switch(organism->status)
            {
                case TE::IDLE:
                    if(organism->energy >= organism->s.birthEnergy)
                    {
                        if(organism->growthAmount > 0.5f && !organism->pregnant)
                        {
                            float chance = abs(startposition_distr(gen));
                            chance /= 100.f;
                            if(chance > organism->s.pregnancyChance)
                            {
                                organism->pregnant = true; // new baby woohoo
                                organism->pregnancyAmount = 0.f;
                            }
                        }
                    }
                    if(organism->pregnantOk)
                    {
                        if(organism->energy >= organism->s.birthEnergy)
                        {
                            float chance = abs(startposition_distr(gen));
                            chance /= 100.f;
                            if(chance > organism->s.pregnancyLength / 25.f)
                                if(livingOrganisms.size() <= MAX_ORGANISMS)
                                    newOrganisms.push_back(organism->Birth());
                            organism->pregnantOk = false;
                            organism->pregnancyAmount = 0.f;
                            organism->pregnant = false;
                        }
                    }
                    break;
                case TE::DEAD:
                    deadOrganisms.push_back(organism);
                    break;
            }
        }
        averageGrowth /= livingOrganisms.size();
        averageHealth /= livingOrganisms.size();
        averageEnergy /= livingOrganisms.size();
        averageAge /= livingOrganisms.size();
        for(int i = 0; i < newOrganisms.size(); i++)
        {
            selectedOrganism = newOrganisms[i];
            livingOrganisms.push_back(newOrganisms[i]);
        }
        for(int i = 0; i < deadOrganisms.size(); i++)
        {
            if(livingOrganisms.size() >= NO_MORE_FOOD_DROPS)
            {
                TE::Food* food = new TE::Food(TE::MEAT,root_node);
                food->transform.pos.x = deadOrganisms[i]->transform.pos.x;
                food->transform.pos.z = deadOrganisms[i]->transform.pos.z;
                food->energy = deadOrganisms[i]->growthAmount * deadOrganisms[i]->s.speciesBrain->neurons->size();
            }
            // clean up references
            if(deadOrganisms[i]->last_child)
                deadOrganisms[i]->last_child->parent_genetic = nullptr;
            if(deadOrganisms[i]->parent_genetic && deadOrganisms[i]->parent_genetic->last_child == deadOrganisms[i])
                deadOrganisms[i]->parent_genetic->last_child = nullptr;
            if(deadOrganisms[i]->parent)
                deadOrganisms[i]->parent->children.remove(deadOrganisms[i]);
            livingOrganisms.remove(deadOrganisms[i]);
            if(deadOrganisms[i] == selectedOrganism)
                selectedOrganism = nullptr;
            if(deadOrganisms[i] == trackedOrganism)
                trackedOrganism = nullptr;
            deadOrganisms[i]->s.Deinit();
            delete deadOrganisms[i];
        }
        bornOnThisTick = 0;
        killedOnThisTick = 0;
        if(newOrganisms.size() != 0)
        {
            bornOnThisTick = newOrganisms.size();
            Logf("%i new organisms born on this tick", newOrganisms.size());
        }
        if(deadOrganisms.size() != 0)
        {
            killedOnThisTick = deadOrganisms.size();
            Logf("%i new organisms died on this tick", deadOrganisms.size());
        }
        if(frame_counter % 100 == 0)
        {
            for(int i = 0; i < 45; i++)
            {
                TE::Food* food = new TE::Food(TE::VEGETABLE,root_node);
                food->transform.pos.x = startposition_distr(gen);
                food->transform.pos.z = startposition_distr(gen);
                food->energy = abs(startposition_distr(gen));
            }
        }

        camera.fog_maxdist = 1000.f;
        if(trackedOrganism)
        {
            TE::Brain* trackedBrain = trackedOrganism->s.speciesBrain;
            switch(ctmode)
            {
                case CT_FOLLOW:
                    {            
                        camera.target = trackedOrganism->transform.pos;
                        camera.position = trackedOrganism->transform.pos;
                        camera.position.y += 10.f;
                        camera.projection = CP_ORTHOGRAPHIC;
                        camera.mode = CM_TARGET;
                        camera.orthographic_settings.top = 2;
                        camera.orthographic_settings.bottom = -2;
                        camera.orthographic_settings.right = 2;
                        camera.orthographic_settings.left = -2;
                    }
                    break;
                case CT_FIRSTPERSON:
                    {
                        camera.mode = CM_FREE;
                        camera.projection = CP_PERSPECTIVE;
                        camera.position = trackedOrganism->transform.pos;
                        camera.yaw = trackedOrganism->transform.eulerRot.z + 180.f;
                        camera.fog_maxdist = trackedOrganism->s.sightDistance;
                    }
                    break;
            }
        }
        else
        {
            camera.target = glm::vec3(0,0,0);
            camera.position = glm::vec3(400,400,100);
            camera.mode = CM_TARGET;
            camera.projection = CP_ORTHOGRAPHIC;
            camera.orthographic_settings.top = 100;
            camera.orthographic_settings.bottom = -100;
            camera.orthographic_settings.right = 100;
            camera.orthographic_settings.left = -100;
        }
    }

    virtual void GUIRender()
    {
        ImDrawList* drawList = ImGui::GetBackgroundDrawList();
        if(livingOrganisms.size() == 0)
        {
            ImGui::Begin("There are no organisms present");
            ImGui::Text("The population in this simulation is 0.");
            if(ImGui::Button("Create new creatures"))
            {
                for(int i = 0; i < 45; i++)
                {
                    TE::Organism* adam = new TE::Organism(&livingOrganisms,root_node);
                    adam->s.Init();
                    adam->s.Mutate();
                    adam->energy = 75.f;
                    livingOrganisms.push_back(adam);
                }
            }
            ImGui::End();
        }

        if(livingOrganisms.size() >= MAX_ORGANISMS)
        {
            drawList->AddText(ImVec2(0,GAME_FIXED_HEIGHT - 16),0xff0000ff,"Too many organisms! Suppression field activated.");
        }
        if(killedOnThisTick > bornOnThisTick)
        {
            drawList->AddText(ImVec2(0,GAME_FIXED_HEIGHT - 16),0xff0000ff,"The world is currently in decay! Extinction event may be imminent.");
        }

        ImGui::Begin("Total Evo");
        ImGui::Checkbox("Pause", &pauseGame);
        ImGui::SliderInt("Camera Mode",(int*)&ctmode,0,1);
        if(ImGui::Button("Cull All"))
        {
            for(auto&& organism : livingOrganisms)
            {
                organism->health = 0.f;
            }
        }
        ImGui::SameLine();
        if(ImGui::Button("Cull Mature"))
        {
            for(auto&& organism : livingOrganisms)
            {
                if(organism->growthAmount > 0.5f)
                    organism->health = 0.f;
            }
        }
        ImGui::SameLine();
        if(ImGui::Button("Cull Old"))
        {
            for(auto&& organism : livingOrganisms)
            {
                if(organism->age > 40.f)
                    organism->health = 0.f;
            }
        }
        if(ImGui::Button("Sprinkle Food"))
        {
            for(int i = 0; i < 15; i++)
            {
                TE::Food* food = new TE::Food(TE::MEAT,root_node);
                food->transform.pos.x = startposition_distr(gen);
                food->transform.pos.z = startposition_distr(gen);
                food->energy = abs(startposition_distr(gen));
            }
        }
        if(selectedOrganism)
        {
            ImGui::Begin("Organism");
            if(ImGui::Button("Track This"))
            {
                trackedOrganism = selectedOrganism;
            }
            if(selectedOrganism->s.speciesBrain)
            {
                ImGui::Begin("Organism Brain");
                ImDrawList* list = ImGui::GetWindowDrawList();
                ImGui::Text("Brain has %i neurons", selectedOrganism->s.speciesBrain->neurons->size());
                selectedOrganism->s.speciesBrain->DrawWidgets(list);
                ImGui::End();
            }
            selectedOrganism->DbgWidgets();
            ImGui::End();
        }
        if(trackedOrganism)
        {
            ImGui::Begin("Tracked Organism");
            if(trackedOrganism->s.speciesBrain)
            {
                ImGui::Begin("Tracked Organism Brain");
                ImDrawList* list = ImGui::GetWindowDrawList();
                ImGui::Text("Brain has %i neurons", trackedOrganism->s.speciesBrain->neurons->size());
                trackedOrganism->s.speciesBrain->DrawWidgets(list);
                ImGui::End();
            }
            if(ImGui::Button("UnTrack This"))
            {
                trackedOrganism = 0;
            }
            else
            {
                trackedOrganism->DbgWidgets();
            }
            ImGui::End();
        }
        ImGui::Text("Delta time: %f", delta_time);
        ImGui::Text("There are %i organisms living", livingOrganisms.size());      
        ImGui::PlotVar("Population", livingOrganisms.size());
        ImGui::PlotVar("Average Growth", averageGrowth);
        ImGui::PlotVar("Average Health", averageHealth);
        ImGui::PlotVar("Average Energy", averageEnergy);
        ImGui::PlotVar("Average Age", averageAge);
        ImGui::PlotVar("Birth Rate", bornOnThisTick);
        ImGui::PlotVar("Death Rate", killedOnThisTick);
        if(livingOrganisms.size() != 0)
            TE::SpeciesInfo::CalcAverages();
        ImGui::PlotVar("avg_pregnancyLength", TE::SpeciesInfo::avg_pregnancyLength);
        ImGui::PlotVar("avg_pregnancyEnergy", TE::SpeciesInfo::avg_pregnancyEnergy);
        ImGui::PlotVar("avg_pregnancyChance", TE::SpeciesInfo::avg_pregnancyChance);
        ImGui::PlotVar("avg_growthRate", TE::SpeciesInfo::avg_growthRate);
        ImGui::PlotVar("avg_foodTypePreference", TE::SpeciesInfo::avg_foodTypePreference);
        ImGui::PlotVar("avg_birthEnergy", TE::SpeciesInfo::avg_birthEnergy);
        ImGui::PlotVar("avg_speed", TE::SpeciesInfo::avg_speed);
        ImGui::PlotVar("avg_scale", TE::SpeciesInfo::avg_scale);
        ImGui::PlotVar("avg_sightDistance", TE::SpeciesInfo::avg_sightDistance);
        ImGui::PlotVar("avg_hungerSearchLimit", TE::SpeciesInfo::avg_hungerSearchLimit);
        ImGui::PlotVar("avg_birthBrainMutations", TE::SpeciesInfo::avg_birthBrainMutations, 0.f, 10.f);
        ImGui::PlotVar("average_neurons", TE::Brain::average_neurons, 0.f, 10.f);
        ImGui::PlotVar("total_neuron_count", TE::Brain::total_neuron_count, 0.f, 10.f);
        // ImGui::PlotVar("total_brain_count", TE::Brain::total_brain_count, 0.f, 10.f);
        ImGui::End();
        if(UI::im_aboutmenu_draw)
        {
            if(ImGui::Begin("About"));
            {
                if(ImGui::BeginChild("about_xtra"))
                {
                    if(ImGui::BeginChild("acknowledgements"))
                    {
                        ImGui::Separator();
                        ImGui::Text("ImPlot License");
                        ImGui::Text(implot_license);
                    }
                    ImGui::EndChild();
                }
                ImGui::EndChild();
            }
            ImGui::End();
        }
    }
};

Engine::App* LoadApp()
{
    TEApp* current_app = new TEApp();
    return current_app;
}