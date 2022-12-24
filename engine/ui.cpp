#include "ui.hpp"
#include "engine.hpp"
#include <math.h>
#include "console.hpp"

bool UI::im_aboutmenu_draw = true;
bool UI::im_debugmenu_draw = false;
ImFont* UI::roboto_regular = 0;
ImFont* UI::roboto_black = 0;
Data::Texture* brand_texture;

const char* bgfx_license = R"(Copyright 2010-2022 Branimir Karadzic

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

   1. Redistributions of source code must retain the above copyright notice, this
      list of conditions and the following disclaimer.

   2. Redistributions in binary form must reproduce the above copyright notice,
      this list of conditions and the following disclaimer in the documentation
      and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
OF THE POSSIBILITY OF SUCH DAMAGE.)";

const char* imgui_license = R"(The MIT License (MIT)

Copyright (c) 2014-2022 Omar Cornut

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

const char* gns_license = R"(Copyright (c) 2018, Valve Corporation
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation and/or
other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors
may be used to endorse or promote products derived from this software without
specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.)";

void DM_DrawNode(Scene::SceneNode* child)
{
    if(ImGui::TreeNode(child,"%s (%p)",child->name,child))
    {
        ImGui::Text("Flags: %08x", child->flags);
        ImGui::InputText("UUID", (char*)child->uuid, 16);
        ImGui::InputText("Name", (char*)child->name, 64);
        ImGui::InputFloat3("Position", (float*)&child->transform.pos);
        if(ImGui::Button("Target Camera on"))
        {
            engine_app->camera.target = child->transform.pos;
            engine_app->camera.position = child->transform.pos;
            engine_app->camera.position.x -= 10.f;
            engine_app->camera.position.y += 10.f;
        }
        ImGui::InputFloat3("Euler Angles", (float*)&child->transform.eulerRot);
        ImGui::InputFloat3("Scale", (float*)&child->transform.scale);
        ImGui::SliderFloat3("Color", (float*)&child->transform.color, 0.0f, 1.0f);
        ImGui::InputScalar("Tag", ImGuiDataType_U64, (void*)&child->tag);
        child->DbgWidgets();
        for(auto&& node : child->children)
        {
            DM_DrawNode(node);
        }
        ImGui::TreePop();
    }
}

void UI::IMDrawDebugMenu()
{
    ImGui::ShowStackToolWindow();
    ImGui::ShowMetricsWindow();
    if(ImGui::Begin("Jalapeno advanced debug"));
    {
        ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_Reorderable;
        if (ImGui::BeginTabBar("Debug", tab_bar_flags))
        {
            if (ImGui::BeginTabItem("Engine")) { 
                ImGui::Checkbox("Console Enable", &Debug::Console::enabled);
                ImGui::InputFloat3("Camera Position", (float*)&engine_app->camera.position);
                ImGui::InputFloat3("Camera Target", (float*)&engine_app->camera.target);
                ImGui::InputFloat("Camera Pitch", &engine_app->camera.pitch);
                ImGui::InputFloat("Camera Yaw", &engine_app->camera.yaw);
                ImGui::InputFloat3("Camera Up", (float*)&engine_app->camera.up);
                ImGui::InputFloat3("Camera Front", (float*)&engine_app->camera.front);
                ImGui::InputFloat3("Camera Right", (float*)&engine_app->camera.right);
                ImGui::DragFloat("Camera Fog Max Dist", &engine_app->camera.fog_maxdist,1.f,0.1f,1000.f);
                ImGui::DragFloat("Camera Fog Min Dist", &engine_app->camera.fog_mindist,1.f,0.1f,1000.f);
                ImGui::InputFloat3("Camera Fog Color", (float*)&engine_app->camera.fog_color);
                ImGui::InputInt("Camera Mode", (int*)&engine_app->camera.mode);
                ImGui::Text("GetAppName: %s", engine_app->GetAppName());
                ImGui::Text("GetAppDescription: %s", engine_app->GetAppDescription());
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Perf")) { 
                ImGui::Text("SDL_Tick: %i", SDL_GetTicks());
                ImGui::Text("App::frame_counter: %i", engine_app->frame_counter);
                ImGui::PlotVar("FPS", engine_app->fps, 0, 240.f);
                ImGui::PlotVar("Dt", engine_app->delta_time, 0.f);
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Scene")) { 
                Scene::SceneNode* root_node = engine_app->root_node;
                ImGui::BeginChild("scenegraph");
                DM_DrawNode(root_node);
                ImGui::EndChild();
                ImGui::EndTabItem();
            }
            if (engine_app->networking_enabled)
            {
                if(ImGui::BeginTabItem("Network"))
                {
                    ImGui::Text("Using Valve's GameNetworkingSockets");
                    ImGui::Separator();
                    if(engine_app->netcontext)
                    {
                        ImGui::Text("GNSClient");
                        char caddr[256];
                        Network::GNSClient* client = (Network::GNSClient*)engine_app->netcontext;
                        SteamNetConnectionInfo_t cinfo;
                        client->netface->GetConnectionInfo(client->connection,&cinfo);
                        if(client->active)
                        {
                            ImGui::Text("Description: %s", cinfo.m_szConnectionDescription);
                            cinfo.m_addrRemote.ToString(caddr, 256, true);
                            ImGui::Text("Remote: %s", caddr);
                        }
                        else
                        {
                            ImGui::Text("Client inactive");
                        }
                        ImGui::Text("State: %i", cinfo.m_eState);
                        client->lastAddr.ToString(caddr, 256, true);
                        ImGui::Text("Last Remote: %s", caddr);                    
                        ImGui::Separator();
                    }
                    ImGui::EndTabItem();
                }
            }
            Input::InputManager* input_manager = engine_app->input_manager;
            if(input_manager)
            {
                if (ImGui::BeginTabItem("Input")) {
                    if(ImGui::Button("Reset Scalars"))
                    {
                        input_manager->ResetScalars();
                    }
                    ImGui::Separator();
                    for(auto&& input : input_manager->scalarInputs)
                    {
                        ImGui::Text("Scalar Input: %s", input.name);
                        ImGui::Text("Positive S Data: %p", input.positiveSource.data);
                        ImGui::Text("Positive S Mode: %i", input.positiveSource.mode);
                        ImGui::Text("Negative S Data: %p", input.negativeSource.data);
                        ImGui::Text("Negative S Mode: %i", input.positiveSource.mode);
                        ImGui::Text("Scalar Value: %f", input.value);
                        ImGui::Separator();
                    }
                    for(auto&& input : input_manager->keyInputs)
                    {
                        ImGui::Text("Keyboard Input: %s", input.name);
                        ImGui::Text("S Data: %p", input.source.data);
                        ImGui::Text("Keyboard Value: %f", input.value);
                        ImGui::Separator();
                    }
                    ImGui::EndTabItem();
                }
            }
            if (ImGui::BeginTabItem("Data")) { 
                ImGui::Text("Filesystem directories:");
                for(int i = 0; i < engine_app->directories.size(); i++)
                {
                    ImGui::SameLine();
                    if(i == engine_app->directories.size() - 1)
                        ImGui::Text("%s", engine_app->directories[i].path);
                    else
                        ImGui::Text("%s,", engine_app->directories[i].path);
                }
                if (ImGui::BeginTabBar("AssetManager", tab_bar_flags))
                {
                    if(ImGui::BeginTabItem("MeshManager"))
                    {
                        ImGui::BeginChild("assetmanager");
                        if(ImGui::Button("Refresh ALL Renders"))
                        {
                            engine_app->mesh_manager->RerenderAll();
                        }
                        for(int i = 0; i < engine_app->mesh_manager->meshes.size(); i++)
                        {
                            Data::Mesh* mesh = engine_app->mesh_manager->meshes.at(i);
                            ImGui::Text("Mesh: %s", mesh->file);
                            ImGui::Text("BBox: %f, %f, %f - %f, %f, %f",    mesh->bbox.width, mesh->bbox.height, mesh->bbox.depth,
                                                                            mesh->bbox.width2, mesh->bbox.height2, mesh->bbox.depth2);
                            ImGui::Image((ImTextureID)mesh->snapshotHandle.idx, ImVec2(128, 128));
                        }
                        ImGui::EndChild();
                        ImGui::EndTabItem();
                    }
                    if(ImGui::BeginTabItem("TextureManager"))
                    {
                        ImGui::BeginChild("assetmanager");          
                        for(int i = 0; i < engine_app->texture_manager->textures.size(); i++)
                        {
                            Data::Texture* texture = engine_app->texture_manager->textures.at(i);
                            ImGui::Text("%ix%i %s", texture->width, texture->height, texture->name);
                            ImGui::Image((ImTextureID)texture->texture.idx, ImVec2(std::min(texture->width,384), std::min(texture->height,384)));
                        }
                        ImGui::EndChild();
                        ImGui::EndTabItem();
                    }
                    if(ImGui::BeginTabItem("ShaderManager"))
                    {
                        ImGui::BeginChild("assetmanager");
                        for(int i = 0; i < engine_app->shader_manager->shaders.size(); i++)
                        {
                            Data::Shader* shader = engine_app->shader_manager->shaders.at(i);
                            ImGui::Text("Vtx: %i", shader->v.idx);
                            ImGui::Text("Frg: %i", shader->f.idx);
                            ImGui::Text("Prg: %i", shader->program.idx);
                        }
                        ImGui::EndChild();
                        ImGui::EndTabItem();
                    }
                }
                ImGui::EndTabBar();
                ImGui::EndTabItem();
            }
        }
        ImGui::EndTabBar();
    }
    ImGui::End();
}

void UI::IMDrawAboutMenu()
{
    float t = sinf(((float)SDL_GetTicks())/100);
    float d = cosf(((float)SDL_GetTicks())/100);
    ImGui::Begin("About");
    ImGui::PushFont(roboto_black);
    ImGui::TextColored(ImVec4(t,d,d/t,1.0),engine_app->GetAppName());
    ImGui::PopFont();
    ImGui::Text(engine_app->GetAppDescription());
    if(ImGui::Button("Okay"))
        im_aboutmenu_draw = false;                    
    ImGui::SameLine();
    if(ImGui::Button("Quit"))
        engine_app->running = false;
    ImGui::Separator();
    if(ImGui::BeginChild("about_xtra"))
    {
        if(ImGui::BeginChild("acknowledgements"))
        {
            ImGui::TextColored(ImVec4(abs(t),abs(t),0.0f,abs(t)),"Acknowledgements");
            ImGui::Separator();
            ImGui::Text("BGFX License");
            ImGui::TextWrapped("%s",bgfx_license);
            ImGui::Separator();
            ImGui::Text("ImGui License");
            ImGui::TextWrapped("%s",imgui_license);
            ImGui::Separator();
            ImGui::Text("GameNetworkingSockets License");
            ImGui::TextWrapped("%s",gns_license);
            if(ImGui::BeginChild("additional_acknowledgements"))
            {
                // GUIRender can put stuff here
            }
            ImGui::EndChild();
        }
        ImGui::EndChild();
    }
    ImGui::EndChild();
    ImGui::Separator();
    ImGui::Image((ImTextureID)brand_texture->texture.idx,ImVec2(615 / 2,153 / 2));
    ImGui::Text("Jalapeno2 is (c) 2022-2023 Plastinium Interactive.\nA copy of the Mozilla Public License version 2.0 in which Jalapeno2 is licensed should be found in the root directory\nof the Jalapeno 2 source code/executable/libraries.");
    ImGui::End();
}

static char ui_ipaddr_input[64];
static bool ui_connectionmenu_open;
void UI::IMDrawConnectionStuff()
{
    ImDrawList* drawList = ImGui::GetBackgroundDrawList();
    if(!engine_app->netcontext->active)
    {
        drawList->AddText(ImVec2(0,20),0xff0000ff,"DISCONNECTED");
        if(ImGui::Begin("Connection Status"))
        {
            ImGui::InputText("IP:Port", ui_ipaddr_input, 64);
            ImGui::Separator();
            if(ImGui::Button("Connect"))
            {
                Network::GNSClient* gns = (Network::GNSClient*)engine_app->netcontext;
                SteamNetworkingIPAddr ipaddr;
                ipaddr.ParseString(ui_ipaddr_input);
                gns->Init(ipaddr);
            }
            ImGui::SameLine();
            if(ImGui::Button("Reconnect"))
            {
                Network::GNSClient* gns = (Network::GNSClient*)engine_app->netcontext;
                gns->Init(gns->lastAddr);
            }
            ImGui::SameLine();
            if(ImGui::Button("Quit"))
            {
                engine_app->running = false;
            }
        }
        ImGui::End();
    }
    else
    {
        switch(engine_app->netcontext->cstatus)
        {
            case Network::CS_CONNECTING:
                drawList->AddText(ImVec2(0,20),0xff00ffff,"Connecting...");
                ui_connectionmenu_open = true;
                if(ImGui::Begin("Connection Status"))
                {
                    ImGui::ProgressBar(0.f);
                    ImGui::Separator();
                    ImGui::Text("Connecting...");
                    if(ImGui::Button("Cancel"))
                    {
                        engine_app->netcontext->Stop("Connection cancelled");
                    }
                    ImGui::BeginDisabled(true);
                    ImGui::SameLine();
                    ImGui::Button("Disconnect");
                    ImGui::EndDisabled();
                }
                ImGui::End();
                break;
            case Network::CS_CONNECTED:
                if(ImGui::Begin("Connection Status"))
                {
                    ImGui::ProgressBar(1.f);
                    ImGui::Separator();
                    ImGui::Text("Connected!");
                    ImGui::BeginDisabled(true);
                    ImGui::Button("Cancel");
                    ImGui::EndDisabled();                    
                    ImGui::SameLine();
                    if(ImGui::Button("Disconnect"))
                    {
                        engine_app->netcontext->Stop("Disconnect by user");
                    }
                }
                ImGui::End();
                break;
        }
    }
}

void UI::IMDrawAllThings()
{
    if(SDL_GetTicks() % 1000 == 0)
        ImGui::PlotVarFlushOldEntries();
    if(im_aboutmenu_draw)
        IMDrawAboutMenu();
    if(im_debugmenu_draw)
        IMDrawDebugMenu();
    if(engine_app->netcontext)
        IMDrawConnectionStuff();
}

void UI::PrecacheUIAssets()
{
    ImGuiIO& io = ImGui::GetIO();
    roboto_regular = io.Fonts->AddFontFromFileTTF("data/ui/Roboto-Regular.ttf", 16.0f);
    roboto_black = io.Fonts->AddFontFromFileTTF("data/ui/Roboto-Black.ttf", 16.0f);
    engine_app->PreLoad(Engine::LT_TEXTURE, "ui/disconnected.png");
    brand_texture = engine_app->texture_manager->GetTexture("brand.png");
}