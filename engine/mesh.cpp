#include "mesh.hpp"
#include "engine.hpp"
#include "stb_image.h"
#include "filesystem.hpp"
#include <easy/profiler.h>
#include <stack>

std::stack<Data::Mesh*> renderQueue;
bgfx::VertexLayout Data::ms_layout;
bgfx::FrameBufferHandle screenshotFramebuffer;
bgfx::TextureHandle screenshotFbTexture;
Scene::DirectionalLightNode* screenshotLighting;

void Data::Part::ApplyTextures(int stage)
{
    for(int i = 0; i < textures.size(); i++)
    {
        if(i > engine_app->texture_manager->maxTextures)
            break;
        engine_app->texture_manager->SetTextureUniform(stage, i, textures.at(i));
    }
}

Data::MeshManager::MeshManager()
{            
    ms_layout
        .begin()
        .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
        .add(bgfx::Attrib::Normal, 3, bgfx::AttribType::Float)
        .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
        .add(bgfx::Attrib::TexCoord1, 2, bgfx::AttribType::Float)
        .add(bgfx::Attrib::Color0,   4, bgfx::AttribType::Uint8, true)
        .add(bgfx::Attrib::Indices,   4, bgfx::AttribType::Int16)
        .add(bgfx::Attrib::Weight,   4, bgfx::AttribType::Float)
        .end();

    screenshotFbTexture = bgfx::createTexture2D(bgfx::BackbufferRatio::Half, false, 1, bgfx::TextureFormat::RGBA8, BGFX_TEXTURE_RT);
    screenshotFramebuffer = bgfx::createFrameBuffer(1, &screenshotFbTexture);
    bgfx::setViewFrameBuffer(1, screenshotFramebuffer);
    bgfx::setViewClear(1, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH,
        0x00000000, 1.0f, 0);
    bgfx::setViewRect(1, 0, 0, GAME_FIXED_WIDTH/2, GAME_FIXED_HEIGHT/2);
    engine_app->texture_manager->PrecacheLoadTexture("#screenshot_fb",screenshotFbTexture,512,512);
    screenshotLighting = new Scene::DirectionalLightNode(nullptr);
    screenshotLighting->direction = glm::vec4(1.0,1.0,0.0,1.0);
    screenshotLighting->ambient = glm::vec4(0.2,0.2,0.2,1.0);
    screenshotLighting->diffuse = glm::vec4(0.2,0.4,0.7,1.0);
    screenshotLighting->specular = glm::vec4(0.8,0.8,0.8,1.0);
    PrecacheLoadMesh("jalapeno.gltf");
}

void Data::Mesh::RenderSnapshot()
{
    EASY_FUNCTION();
    if(!engine_app->shader_manager)
    {    
        engine_app->Logf("Render attempted before shader_manager was initialized!!!!!!!");
        return;
    }
    Shader* render_shader = engine_app->shader_manager->GetShader("snapshot");
    if(!render_shader)
    {
        engine_app->Logf("Mesh: Using simple shader as fallback");
        render_shader = engine_app->shader_manager->GetShader("simple");
        if(!render_shader)
        {
            engine_app->Logf("Mesh: No shader available!!!!");
            return;
        }
    }

    glm::vec3 cameraPos = glm::vec3(bbox.width, 0.f, bbox.depth);  
    glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
    bgfx::setUniform(engine_app->camera.u_camera_position, &cameraPos);

    glm::mat4 view;
    view = glm::lookAt(cameraPos, cameraTarget, glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 proj;
    proj = glm::perspective(45.f, 1.f, 100.f, 0.1f);
    glm::mat4 model;
    model = glm::mat4(1.f);
    model = glm::translate(model, glm::vec3(0.0f,0.0f,0.0f));
    bgfx::setViewFrameBuffer(1, screenshotFramebuffer);
    bgfx::setViewClear(1, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH,
        0x000000ff, 1.0f, 0);
    bgfx::setViewRect(1, 0, 0, GAME_FIXED_WIDTH/2, GAME_FIXED_HEIGHT/2);
    screenshotLighting->Update(); // use values in screenshot lighting
    bgfx::touch(1);

    for(int i = 0; i < parts->size(); i++)
    {
        Part* meshPart = parts->at(i);

        meshPart->normalMatrix = glm::mat3(glm::transpose(glm::inverse(model)));
        meshPart->ApplyTextures(1);
        bgfx::setUniform(meshPart->u_normalMatrix,&meshPart->normalMatrix);

        bgfx::setTransform(&model);
        bgfx::setViewTransform(1, &view,&proj);
        bgfx::setVertexBuffer(1, meshPart->vbh);
        bgfx::setIndexBuffer(meshPart->ibh);

        bgfx::submit(1, render_shader->program);
    }

    bgfx::blit(1, snapshotHandle, 0, 0, screenshotFbTexture, 0, 0, GAME_FIXED_WIDTH/2, GAME_FIXED_HEIGHT/2); 
    engine_app->Logf("Mesh: Taken snapshot of mesh %s", file);
}

void Data::Mesh::ProcessNode(aiNode* node, const aiScene* scene)
{
    EASY_FUNCTION();
    for(int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        Part* part = ProcessMesh(mesh, scene);
        parts->push_back(part);
        bbox.width = std::max(bbox.width,bbox.width);
        bbox.height = std::max(bbox.height,bbox.height);
        bbox.depth = std::max(bbox.depth,bbox.depth);
    }
    for(int i = 0; i < node->mNumChildren; i++)
    {
        ProcessNode(node->mChildren[i], scene);
    }
}

Data::Part* Data::Mesh::ProcessMesh(aiMesh* mesh, const aiScene *scene)
{
    EASY_FUNCTION();
    Part* p = new Part();
    int WEIGHTS_PER_VERTEX = 4;
    int boneArraysSize = mesh->mNumVertices*WEIGHTS_PER_VERTEX;
    std::vector<int> bone_ids;    
    bone_ids.resize(boneArraysSize);
    std::vector<float> bone_weights;
    bone_weights.resize(boneArraysSize);

    p->u_normalMatrix = bgfx::createUniform("u_normalmatrix", bgfx::UniformType::Mat3);

    float bbox_w1, bbox_w2, bbox_h1, bbox_h2, bbox_d1, bbox_d2;

    for(int i = 0; i < mesh->mNumVertices; i++)
    {
        PosColorVertex vertex;
        vertex.vertex.x = mesh->mVertices[i].x;
        vertex.vertex.y = mesh->mVertices[i].y;
        vertex.vertex.z = mesh->mVertices[i].z; 
        vertex.normal.x = mesh->mNormals[i].x;
        vertex.normal.y = mesh->mNormals[i].y;
        vertex.normal.z = mesh->mNormals[i].z;
        vertex.r = 255;
        vertex.g = 255;
        vertex.b = 255;
        vertex.a = 0;
        if(mesh->mTextureCoords[0])
        {
            vertex.texcoord.x = mesh->mTextureCoords[0][i].x; 
            vertex.texcoord.y = mesh->mTextureCoords[0][i].y;
        }
        if(mesh->mColors[0])
        {
            vertex.r = mesh->mColors[0][i].r * 255.f;
            vertex.g = mesh->mColors[0][i].g * 255.f;
            vertex.b = mesh->mColors[0][i].b * 255.f;
            vertex.a = mesh->mColors[0][i].a * 255.f;
        }

        bbox_w1 = std::max(vertex.vertex.x, bbox_w1);
        bbox_w2 = std::min(vertex.vertex.x, bbox_w2);
        bbox_h1 = std::max(vertex.vertex.y, bbox_h1);
        bbox_h2 = std::min(vertex.vertex.y, bbox_h2);
        bbox_d1 = std::max(vertex.vertex.z, bbox_d1);
        bbox_d2 = std::min(vertex.vertex.z, bbox_d2);
        
        p->vertices.push_back(vertex);
    }

    // i used https://realitymultiplied.wordpress.com/2016/05/02/assimp-skeletal-animation-tutorial-1-vertex-weights-and-indices/
    // because i didnt quite get this part and would really like to continue programming without dealing with bones for 100 years
    for(int i = 0; i < mesh->mNumBones; i++)
    {
        aiBone* aiBone = mesh->mBones[i]; 
        for(int j=0;j<aiBone->mNumWeights;j++)
        {
            aiVertexWeight weight = aiBone->mWeights[j];
            unsigned int vtx = weight.mVertexId * WEIGHTS_PER_VERTEX;
            for(int k = 0; k < WEIGHTS_PER_VERTEX; k++)
            {
                if(bone_weights.at(vtx+k)==0)
                {
                    bone_weights.at(vtx+k) = weight.mWeight;
                    bone_ids.at(vtx+k) = i;
                    p->vertices.at(weight.mVertexId).bone_id[k] = i;
                    p->vertices.at(weight.mVertexId).bone_weight[k] = weight.mWeight;
                    break;
                }
            }
        }
    }
    engine_app->Logf("MeshManager: %i bones added", mesh->mNumBones);

    p->bbox.width = bbox_w1 - bbox_w2;
    p->bbox.height = bbox_h1 - bbox_h2;
    p->bbox.depth = bbox_d1 - bbox_d2;

    bbox.width = std::max(p->bbox.width,bbox.width);
    bbox.height = std::max(p->bbox.height,bbox.height);
    bbox.depth = std::max(p->bbox.depth,bbox.depth);

    for(int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for(unsigned int j = 0; j < face.mNumIndices; j++)
            p->indices.push_back(face.mIndices[j]);
    }  

    if(mesh->mMaterialIndex >= 0)
    {
        aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
        std::vector<Texture*> diffuseMaps = LoadMaterialTextures(material, 
                                            aiTextureType_DIFFUSE, "texture_diffuse", scene);
        p->textures.insert(p->textures.end(), diffuseMaps.begin(), diffuseMaps.end());
        std::vector<Texture*> specularMaps = LoadMaterialTextures(material, 
                                            aiTextureType_SPECULAR, "texture_specular", scene);
        p->textures.insert(p->textures.end(), specularMaps.begin(), specularMaps.end());
    }

    engine_app->Logf("MeshManager: Created new part with %i vertices and %i indices", p->vertices.size(), p->indices.size());
    Data::PosColorVertex* verticesp = (Data::PosColorVertex*)malloc(sizeof(Data::PosColorVertex)*p->vertices.size());
    for(int i = 0; i < p->vertices.size(); i++)
    {
        verticesp[i] = p->vertices[i];
    }
    p->vbh = bgfx::createVertexBuffer(bgfx::makeRef(verticesp,sizeof(Data::PosColorVertex)*p->vertices.size()), ms_layout);
    p->ibh = bgfx::createIndexBuffer(bgfx::makeRef(p->indices.data(),p->indices.size()*sizeof(uint16_t)));

    return p;
}

std::vector<Data::Texture*> Data::Mesh::LoadMaterialTextures(aiMaterial *material, aiTextureType type, const char* name, const aiScene* scene)
{
    EASY_FUNCTION();
    std::vector<Texture*> textures;
    for(unsigned int i = 0; i < material->GetTextureCount(type); i++)
    {
        aiString str;
        material->GetTexture(type, i, &str);
        char texName2[512];
        snprintf(texName2, 512, "%s_%s_%s", name, str.C_Str(), file);      
        if(!engine_app->texture_manager->GetTexture(texName2))
        {
            if(str.data[0] == '*')
            {
                const aiTexture* tex = scene->GetEmbeddedTexture(str.C_Str());
                engine_app->texture_manager->PrecacheLoadTexture(texName2, tex->pcData, tex->mWidth);      
                textures.push_back(engine_app->texture_manager->GetTexture(texName2));
            }
            else
            {
                Texture* tex = engine_app->texture_manager->GetTexture(str.C_Str());
                if(!tex)
                {
                    engine_app->PreLoad(Engine::LT_TEXTURE, str.C_Str());
                }
                tex = engine_app->texture_manager->GetTexture(str.C_Str());
                if(!tex)
                {
                    engine_app->Logf("Mesh: cant load material textures for %s",str.C_Str());
                    continue;
                }
                textures.push_back(tex);
            }
        }
    }
    for(int i = 0; i < textures.size(); i++)
    {
        if(i > engine_app->texture_manager->maxTextures)
            break;
        engine_app->Logf("Mesh: texture%i = %s", i, textures[i]->name);
    }
    return textures;
}

void Data::MeshManager::PrecacheLoadMesh(const char* file)
{
    EASY_FUNCTION();
    Mesh* mesh = (Mesh*)malloc(sizeof(Mesh));
    mesh->parts = new std::vector<Part*>();
    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(IO::FileSystem::GetDataPath(file), aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals);
    if(scene)
    {
        engine_app->Logf("MeshManager: Precaching %s", file);
        mesh->file = (const char*)malloc(strlen(file)+1);
        strcpy((char*)mesh->file,file);
        mesh->ProcessNode(scene->mRootNode, scene);
        mesh->snapshotHandle = bgfx::createTexture2D(bgfx::BackbufferRatio::Half, false, 1, bgfx::TextureFormat::RGBA8, BGFX_TEXTURE_BLIT_DST);
        engine_app->texture_manager->PrecacheLoadTexture(file,mesh->snapshotHandle,512,512);
        renderQueue.push(mesh);
        meshes.push_back(mesh);
    }
    else
    {
        engine_app->Logf("MeshManager: FAILED %s", file);
    }
}

Data::Mesh* Data::MeshManager::GetMesh(const char* file)
{
    EASY_FUNCTION();
    for(int i = 0; i < meshes.size(); i++)
    {
        if(strcmp(file,meshes[i]->file)==0)
        {
            return meshes[i];
        }
    }
    #ifdef DEBUG
    
    #else
    engine_app->Logf("MeshManager: %s couldnt be loaded", file);
    #endif
    return nullptr;
}

bool Data::MeshManager::RenderQueue()
{
    EASY_FUNCTION();
    bgfx::setMarker("Render queue");
    if(!renderQueue.empty())
    {
        Data::Mesh* mesh = renderQueue.top();
        mesh->RenderSnapshot();
        renderQueue.pop();
        engine_app->Logf("%i more renders to do...", renderQueue.size());
        return true;
    }
    return false;
}

void Data::MeshManager::RerenderAll()
{
    EASY_FUNCTION();
    for(int i = 0; i < meshes.size(); i++)
    {
        renderQueue.push(meshes[i]);
    }
}