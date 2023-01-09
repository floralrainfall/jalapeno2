#include "jpbsp.hpp"
#include "engine.hpp"
#include <iostream>
#include <fstream>
#include <imgui/imgui.h>

bgfx::VertexLayout JPBSP::BSPFile::bsp_layout;
bgfx::UniformHandle JPBSP::BSPFile::texture0;
bgfx::UniformHandle JPBSP::BSPFile::texture1;
void JPBSP::BSPFile::Init()
{
    engine_app->PreLoad(Engine::LT_SHADER, "bspmap");
    bsp_layout
        .begin()
        .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
        .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
        .add(bgfx::Attrib::TexCoord1, 2, bgfx::AttribType::Float)
        .add(bgfx::Attrib::Normal, 3, bgfx::AttribType::Float)
        .add(bgfx::Attrib::Color0,   4, bgfx::AttribType::Uint8, true)
        .end();
    texture0 = bgfx::createUniform("texture0",bgfx::UniformType::Sampler);
    texture1 = bgfx::createUniform("texture1",bgfx::UniformType::Sampler);
}

enum FaceType
{
    Normal = 1,
    Mesh = 2,
    TriSurf = 3,
    Flare = 4
};


void JPBSP::BSPFile::FixVertex(BSPVertex& vertex)
{
    //Swizzle(vertex.position, true);
    //Swizzle(vertex.normal);
}

void JPBSP::BSPFile::Swizzle(int* v)
{
    int temp = v[1];
    v[1] = v[2];
    v[2] = -temp;
}

void JPBSP::BSPFile::ParseFace(BSPFace face)
{
    switch((FaceType)face.type)
    {
        case Normal:
        case TriSurf:
        {
            BSPMesh mesh;
            for(int i = 0; i < face.n_vertices; i++)
            {
                FixVertex(bsp_vertices[face.vertex + i]);
            }
            mesh.vbh = bgfx::createVertexBuffer(bgfx::makeRef(&bsp_vertices[face.vertex],sizeof(BSPVertex)*face.n_vertices), bsp_layout);
            mesh.ibh = bgfx::createIndexBuffer(bgfx::makeRef(&bsp_meshverts[face.meshvert],sizeof(BSPMeshVert)*face.n_meshverts), BGFX_BUFFER_INDEX32);
            if(face.texture < r_textures.size())
            {
                mesh.texture = r_textures[face.texture];    
                mesh.shader = engine_app->shader_manager->GetShader("bspmap");
                if(face.effect != -1)
                {
                    BSPEffect effect = ((BSPEffect*)bsp_data[BET_EFFECTS])[face.effect];
                    mesh.shader = engine_app->shader_manager->GetShader(effect.name);
                }
            }
            if(face.lm_index < r_lightmaps.size())
                mesh.lightmap = r_lightmaps[face.lm_index];
            meshes.push_back(mesh);
        }
        case Mesh:
            break;
        case Flare:
            break;
    }
}

void JPBSP::BSPFile::ParseModel(BSPModel model)
{
    for(int i = 0; i < model.n_faces; i++)
    {
        ParseFace(bsp_faces[model.face + i]);
    }
}

JPBSP::BSPFile::BSPFile(const char* file)
{
    engine_app->Logf("BSPFile: loading %s", file);
    std::ifstream file_read(IO::FileSystem::GetDataPath(file), std::ios::binary);
    loaded = false;

    for(int i = 0; i < BSPEntryType::Count; i++)
    {
        bsp_data[i] = 0;
    }

    if(file_read.good())
    {
        file_read.read((char*)&header, sizeof(header));

        engine_app->Logf("BSPFile: file version %02x", header.version);
        for(int i = 0; i < BSPEntryType::Count; i++)
        {
            void* new_space = malloc(header.entries[i].length);
            file_read.seekg(header.entries[i].offset, std::ios::beg);
            file_read.read((char*)new_space, header.entries[i].length);
            if(file_read.eof())
            {
                engine_app->Logf("BSPFile: unexpected eof (lump %i)", i);
                return;
            }
            engine_app->Logf("BSPFile: read lump %02i from %p to %p, %i bytes large", i, header.entries[i].offset, new_space, header.entries[i].length);
            bsp_data[i] = new_space;

            if(i == BET_FACES)
                bsp_faces = (BSPFace*)new_space;
            else if(i == BET_MESHVERTS)
                bsp_meshverts = (BSPMeshVert*)new_space;
            else if(i == BET_VERTICES)
                bsp_vertices = (BSPVertex*)new_space;
        }

        BSPTexture* textures = (BSPTexture*)bsp_data[BET_TEXTURES];
        int num_textures = header.entries[BET_TEXTURES].length / sizeof(BSPTexture);
        engine_app->Logf("BSPFile: %i textures to load", num_textures);
        for(int i = 0; i < num_textures; i++)
        {   
            char fname[64];
            snprintf(fname,64,"%s.jpg",&textures[i].name[0]);
            engine_app->PreLoad(Engine::LT_TEXTURE, &fname[0]);
            Data::Texture* tex =  engine_app->texture_manager->GetTexture(&fname[0]);
            if(tex)
                r_textures.push_back({.texture = tex->texture, .name = fname});
        }        
        BSPLightmap* lightmaps = (BSPLightmap*)bsp_data[BET_LIGHTMAPS];
        int num_lightmaps = header.entries[BET_LIGHTMAPS].length / sizeof(BSPLightmap);
        engine_app->Logf("BSPFile: %i lightmaps to load", num_lightmaps);
        for(int i = 0; i < num_lightmaps; i++)
        {   
            BSPLightmap lightmap = lightmaps[i];
            bgfx::TextureHandle tex = bgfx::createTexture2D(128,128,false,1,bgfx::TextureFormat::RGB8,BGFX_SAMPLER_UVW_CLAMP,bgfx::copy(&lightmaps[i].data,sizeof(lightmaps[i].data)));
            r_lightmaps.push_back({.texture = tex});
        }

        BSPModel* model = (BSPModel*)bsp_data[BET_MODELS];
        ParseModel(*model);

        /*
        int num_nodes = header.entries[BET_NODES].length / sizeof(BSPNode);
        for(int i = 0; i < num_nodes; i++)
        {
            BSPNode& node = ((BSPNode*)bsp_data[BET_NODES])[i];
            int temp = node.maxs[1];
            node.maxs[1] = node.maxs[2];
            node.maxs[2] = -temp;
            temp = node.mins[1];
            node.mins[1] = node.mins[2];
            node.mins[2] = -temp;
        }
        int num_planes = header.entries[BET_PLANES].length / sizeof(BSPPlane);
        for(int i = 0; i < num_planes; i++)
        {
            BSPPlane& plane = ((BSPPlane*)bsp_data[BET_PLANES])[i];
            plane.distance *= 0.3f;
            float temp = plane.normal[1];
            plane.normal[1] = plane.normal[2];
            plane.normal[2] = -temp;
        }
        int num_leafs = header.entries[BET_LEAFS].length / sizeof(BSPLeaf);
        for(int i = 0; i < num_planes; i++)
        {
            BSPLeaf& leaf = ((BSPLeaf*)bsp_data[BET_LEAFS])[i];
            int temp = leaf.maxs[1];
            leaf.maxs[1] = leaf.maxs[2];
            leaf.maxs[2] = -temp;
            temp = leaf.mins[1];
            leaf.mins[1] = leaf.mins[2];
            leaf.mins[2] = -temp;
        }*/
        int num_effects = header.entries[BET_EFFECTS].length / sizeof(BSPEffect);
        engine_app->Logf("BSPFile: %i shaders to load", num_effects);
        for(int i = 0; i < num_effects; i++)
        {
            BSPEffect effect = ((BSPEffect*)bsp_data[BET_EFFECTS])[i];
            Data::Shader* old_shader = engine_app->shader_manager->GetShader(effect.name);
            if(!old_shader)
            {
                engine_app->Logf("BSPFile: Loading effect %s", effect.name);
                engine_app->shader_manager->PrecacheLoadShader(effect.name);
            }
        }

        loaded = true;
    }
    else
    {
        engine_app->Logf("BSPFile: couldn't open %s", file);
    }
};

JPBSP::BSPFile::~BSPFile()
{
    for(int i = 0; i < BSPEntryType::Count; i++)
    {
        if(bsp_data[i])
            free(bsp_data[i]);
    }
    for(int i = 0; i < r_lightmaps.size(); i++)
    {
        bgfx::destroy(r_lightmaps[i].texture);
    }
    for(int i = 0; i < meshes.size(); i++)
    {
        bgfx::destroy(meshes[i].vbh);
        bgfx::destroy(meshes[i].ibh);
    }
}

JPBSP::BSPSceneNode::BSPSceneNode(const char* file, Scene::SceneNode* parent) :
    bsp_file(file), Scene::SceneNode(parent)
{
    SetName(file);

    uint16_t tindices[] = {
        2,6,7,
        2,3,7,
        0,4,5,
        0,2,6,
        0,4,6,
        1,3,7,
        1,5,7,
        0,2,3,
        0,1,3,
        4,6,7,
        4,5,7,
    };
    for(int i = 0; i < sizeof(tindices)/sizeof(uint16_t); i++)
    {
        bbox_indices.push_back(tindices[i]);
    }

    BSPVertex tvertices[] = {
        {.position = {-1.f,-1.f,0.5f}},
        {.position = { 1.f,-1.f,0.5f}},
        {.position = {-1.f, 1.f,0.5f}},
        {.position = { 1.f, 1.f,0.5f}},
        {.position = {-1.f,-1.f,-0.5f}},
        {.position = { 1.f,-1.f,-0.5f}},
        {.position = {-1.f, 1.f,-0.5f}},
        {.position = { 1.f, 1.f,-0.5f}},
    };
    for(int i = 0; i < sizeof(tindices)/sizeof(uint16_t); i++)
    {
        bbox_vertices.push_back(tvertices[i]);
    }

    bbox_vbh = bgfx::createVertexBuffer(bgfx::makeRef(bbox_vertices.data(),sizeof(BSPVertex)*bbox_vertices.size()), BSPFile::bsp_layout);
    bbox_ibh = bgfx::createIndexBuffer(bgfx::makeRef(bbox_indices.data(),sizeof(uint16_t)*bbox_indices.size()));
    bbox_shader = engine_app->shader_manager->GetShader("simple");
}

void JPBSP::BSPSceneNode::RenderBrush(BSPBrush* brush)
{
    
}

void JPBSP::BSPSceneNode::RenderNode(int node_id)
{
    if (node_id < 0)
    {
        BSPLeaf* leaf = &((BSPLeaf*)bsp_file.bsp_data[BET_LEAFS])[-(node_id + 1)];
        for(int i = 0; i < leaf->n_leafbrushes; i++)
        {
            BSPBrush* brush = &((BSPBrush*)bsp_file.bsp_data[BET_BRUSHES])[leaf->leafbrush+i];
            RenderBrush(brush);
            bgfx::setVertexBuffer(0, bbox_vbh);
            bgfx::setIndexBuffer(bbox_ibh);
            bgfx::submit(0, bbox_shader->program);
        }
    }
    BSPNode node = ((BSPNode*)bsp_file.bsp_data[BET_NODES])[node_id];
    RenderNode(node.children[0]);
    RenderNode(node.children[1]);
}

void JPBSP::BSPSceneNode::Render()
{
    glm::mat4 model = glm::mat4(1.f);
    model = glm::translate(glm::vec3(0.f));
    if(draw_collision)
    {
        //RenderNode(0);
    }
    else
    {
        for(int i = 0; i < bsp_file.meshes.size(); i++)
        {
            if(bsp_file.meshes[i].shader)
            {
                bgfx::setVertexBuffer(0, bsp_file.meshes[i].vbh);
                bgfx::setIndexBuffer(bsp_file.meshes[i].ibh);
                bgfx::setTransform((float*)&model);
                bgfx::setState(BGFX_STATE_DEFAULT ^ BGFX_STATE_CULL_CW | BGFX_STATE_CULL_CCW);
                bgfx::setTexture(0, BSPFile::texture0, bsp_file.meshes[i].texture.texture);
                bgfx::setTexture(1, BSPFile::texture1, bsp_file.meshes[i].lightmap.texture);
                bgfx::submit(0, bsp_file.meshes[i].shader->program);
            }
            else
                bsp_file.meshes[i].shader = engine_app->shader_manager->GetShader("simple");
        }
    }
    SceneNode::Render();
}

void JPBSP::BSPSceneNode::DbgWidgets()
{
    SceneNode::DbgWidgets();

    for(int i = 0; i < bsp_file.r_lightmaps.size(); i++)
    {
        ImGui::Image((ImTextureID)bsp_file.r_lightmaps[i].texture.idx, ImVec2(384,384));
    }
}